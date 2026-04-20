#include "PressureExpansionStrategy.h"

#include <algorithm>
#include <cmath>

PressureExpansionStrategy::PressureExpansionStrategy(const Config &config)
    : config_(config) {
}

SignalResult PressureExpansionStrategy::evaluate(const StrategyContext &context) const {
    SignalResult result;

    const auto &snapshot = context.marketSnapshot;
    const auto &flow = context.flowSnapshot;
    const auto &regime = context.regimeSnapshot;

    result.flowBias = flow.aggressionBias;
    result.flowStrength = flow.totalAggressionQty;

    if (!regime.tradable) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "regime not tradable";
        result.isValid = false;
        return result;
    }

    if (snapshot.spreadBps > config_.maxSpreadBps) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "spread too high";
        result.isValid = false;
        return result;
    }

    if (!isFlowStrongEnough(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "flow too weak for pressure expansion";
        result.isValid = false;
        return result;
    }

    if (!isRegimeExpansionStrongEnough(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "regime expansion too weak";
        result.isValid = false;
        return result;
    }

    const bool bookLong = isBookLongSupportive(context);
    const bool bookShort = isBookShortSupportive(context);
    const bool flowLong = isFlowLongSupportive(context);
    const bool flowShort = isFlowShortSupportive(context);

    const bool flowOverrideLong = isFlowOverrideLong(context);
    const bool flowOverrideShort = isFlowOverrideShort(context);

    const double confidence = calculateConfidence(context);
    const double expectedMoveBps = calculateExpectedMoveBps(context);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    const bool confidenceOk = confidence >= 0.55;
    const bool expectedMoveOk = expectedMoveBps >= 12.0;

    const bool normalLong = bookLong && flowLong;
    const bool normalShort = bookShort && flowShort;

    if (normalLong || flowOverrideLong) {
        if (!confidenceOk) {
            result.side = SignalSide::HOLD;
            result.reason = "confidence below min";
            result.isValid = false;
            return result;
        }

        if (!expectedMoveOk) {
            result.side = SignalSide::HOLD;
            result.reason = "expected move below min";
            result.isValid = false;
            return result;
        }

        result.side = SignalSide::LONG;
        result.reason = flowOverrideLong
                            ? "long pressure expansion confirmed by flow override"
                            : "long pressure expansion confirmed";
        result.isValid = true;
        return result;
    }

    if (normalShort || flowOverrideShort) {
        if (!confidenceOk) {
            result.side = SignalSide::HOLD;
            result.reason = "confidence below min";
            result.isValid = false;
            return result;
        }

        if (!expectedMoveOk) {
            result.side = SignalSide::HOLD;
            result.reason = "expected move below min";
            result.isValid = false;
            return result;
        }

        result.side = SignalSide::SHORT;
        result.reason = flowOverrideShort
                            ? "short pressure expansion confirmed by flow override"
                            : "short pressure expansion confirmed";
        result.isValid = true;
        return result;
    }

    if ((bookLong && flowShort) || (bookShort && flowLong)) {
        result.side = SignalSide::HOLD;
        result.reason = "flow-book direction mismatch";
        result.isValid = false;
        return result;
    }

    if (!bookLong && !bookShort) {
        result.side = SignalSide::HOLD;
        result.reason = "book not supportive enough for pressure expansion";
        result.isValid = false;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "book-flow alignment not strong enough";
    result.isValid = false;
    return result;
}

double PressureExpansionStrategy::calculateConfidence(const StrategyContext &context) const {
    const auto &snapshot = context.marketSnapshot;
    const auto &flow = context.flowSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    if (flowBiasStrength > 1.0) {
        flowBiasStrength = 1.0;
    }

    const double regimeExpansionStrength = calculateRegimeExpansionStrength(context);

    double confidence =
            (imbalanceStrength * 0.45) +
            (flowBiasStrength * 0.35) +
            (regimeExpansionStrength * 0.20);

    confidence = std::clamp(confidence, 0.0, 1.0);
    return confidence;
}

double PressureExpansionStrategy::calculateExpectedMoveBps(const StrategyContext &context) const {
    const auto &snapshot = context.marketSnapshot;
    const auto &flow = context.flowSnapshot;

    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.40;
    const double flowComponent = (std::abs(flow.aggressionBias) * 20.0) * 0.70;
    const double regimeComponent = calculateRegimeExpansionStrength(context) * 4.0;

    double expectedMove = imbalanceComponent + flowComponent + regimeComponent;
    if (expectedMove < 0.0) {
        expectedMove = 0.0;
    }

    return expectedMove;
}

double PressureExpansionStrategy::calculateRegimeExpansionStrength(const StrategyContext &context) const {
    const auto &regime = context.regimeSnapshot;
    const double expansionBps = std::max(regime.activityBps, regime.shortRangeBps);
    return std::clamp(expansionBps / 1.0, 0.0, 1.0);
}

bool PressureExpansionStrategy::isBookLongSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= 58.0;
}

bool PressureExpansionStrategy::isBookShortSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= 42.0;
}

bool PressureExpansionStrategy::isFlowLongSupportive(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= 0.45;
}

bool PressureExpansionStrategy::isFlowShortSupportive(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -0.45;
}

bool PressureExpansionStrategy::isFlowStrongEnough(const StrategyContext &context) const {
    return context.flowSnapshot.totalAggressionQty >= 0.08;
}

bool PressureExpansionStrategy::isRegimeExpansionStrongEnough(const StrategyContext &context) const {
    const auto &regime = context.regimeSnapshot;
    return std::max(regime.activityBps, regime.shortRangeBps) >= 0.08;
}

bool PressureExpansionStrategy::isRecentMoveAcceptable(const StrategyContext &context) const {
    (void) context;
    return true;
}

bool PressureExpansionStrategy::isFlowOverrideLong(const StrategyContext& context) const {
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double expansionBps = std::max(regime.activityBps, regime.shortRangeBps);

    return flow.aggressionBias >= 0.75 &&
           flow.totalAggressionQty >= 5.0 &&
           expansionBps >= 1.0;
}

bool PressureExpansionStrategy::isFlowOverrideShort(const StrategyContext& context) const {
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double expansionBps = std::max(regime.activityBps, regime.shortRangeBps);

    return flow.aggressionBias <= -0.75 &&
           flow.totalAggressionQty >= 5.0 &&
           expansionBps >= 1.0;
}
