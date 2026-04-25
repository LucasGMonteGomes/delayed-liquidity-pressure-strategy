#include "LowFrequencyRegimeContinuationStrategy.h"

#include <algorithm>
#include <cmath>

LowFrequencyRegimeContinuationStrategy::LowFrequencyRegimeContinuationStrategy(const Config& config)
    : config_(config) {}

SignalResult LowFrequencyRegimeContinuationStrategy::evaluate(const StrategyContext& context) const {
    SignalResult result;

    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

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

    if (!isSpreadAcceptable(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "spread too high";
        result.isValid = false;
        return result;
    }

    if (!isRegimeStrongEnough(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "regime too weak for low frequency continuation";
        result.isValid = false;
        return result;
    }

    const bool longFlow = isLongFlowAligned(context);
    const bool shortFlow = isShortFlowAligned(context);

    const bool longBook = isLongBookAligned(context);
    const bool shortBook = isShortBookAligned(context);

    const bool longMove = isLongMoveConfirmed(context);
    const bool shortMove = isShortMoveConfirmed(context);

    result.confidence = calculateConfidence(context);
    result.expectedMoveBps = calculateExpectedMoveBps(context);

    if (!longFlow && !shortFlow) {
        result.side = SignalSide::HOLD;
        result.reason = "flow not aligned for low frequency continuation";
        result.isValid = false;
        return result;
    }

    if (longFlow && (!longBook || !longMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "long low frequency continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (shortFlow && (!shortBook || !shortMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "short low frequency continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (result.confidence < config_.minConfidence) {
        result.side = SignalSide::HOLD;
        result.reason = "low frequency continuation confidence below min";
        result.isValid = false;
        return result;
    }

    if (result.expectedMoveBps < config_.minExpectedMoveBps) {
        result.side = SignalSide::HOLD;
        result.reason = "low frequency continuation expected move below min";
        result.isValid = false;
        return result;
    }

    if (longFlow && longBook && longMove) {
        result.side = SignalSide::LONG;
        result.reason = "long low frequency continuation detected";
        result.isValid = true;
        return result;
    }

    if (shortFlow && shortBook && shortMove) {
        result.side = SignalSide::SHORT;
        result.reason = "short low frequency continuation detected";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "low frequency continuation unresolved";
    result.isValid = false;
    return result;
}

bool LowFrequencyRegimeContinuationStrategy::isSpreadAcceptable(const StrategyContext& context) const {
    return context.marketSnapshot.spreadBps <= config_.maxSpreadBps;
}

bool LowFrequencyRegimeContinuationStrategy::isRegimeStrongEnough(const StrategyContext& context) const {
    return context.regimeSnapshot.shortRangeBps >= 1.80 &&
           context.regimeSnapshot.activityBps >= 1.80 &&
           context.regimeSnapshot.hasRecentFlow;
}

bool LowFrequencyRegimeContinuationStrategy::isLongFlowAligned(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= config_.minFlowBiasAbs &&
           context.flowSnapshot.totalAggressionQty >= config_.minFlowStrength;
}

bool LowFrequencyRegimeContinuationStrategy::isShortFlowAligned(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -config_.minFlowBiasAbs &&
           context.flowSnapshot.totalAggressionQty >= config_.minFlowStrength;
}

bool LowFrequencyRegimeContinuationStrategy::isLongBookAligned(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= config_.imbalanceLongThreshold;
}

bool LowFrequencyRegimeContinuationStrategy::isShortBookAligned(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= config_.imbalanceShortThreshold;
}

bool LowFrequencyRegimeContinuationStrategy::isLongMoveConfirmed(const StrategyContext& context) const {
    return context.recentMoveBps >= 1.20 &&
           context.recentMoveBps <= config_.maxRecentMoveBps;
}

bool LowFrequencyRegimeContinuationStrategy::isShortMoveConfirmed(const StrategyContext& context) const {
    return context.recentMoveBps <= -1.20 &&
           std::abs(context.recentMoveBps) <= config_.maxRecentMoveBps;
}

double LowFrequencyRegimeContinuationStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    flowBiasStrength = std::clamp(flowBiasStrength, 0.0, 1.0);

    double flowStrengthNorm = flow.totalAggressionQty / 8.0;
    flowStrengthNorm = std::clamp(flowStrengthNorm, 0.0, 1.0);

    double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;
    imbalanceStrength = std::clamp(imbalanceStrength, 0.0, 1.0);

    double regimeStrength = (regime.shortRangeBps + regime.activityBps) / 2.0;
    double regimeStrengthNorm = std::clamp(regimeStrength / 8.0, 0.0, 1.0);

    double recentMoveStrength = std::abs(context.recentMoveBps) / 10.0;
    recentMoveStrength = std::clamp(recentMoveStrength, 0.0, 1.0);

    const double confidence =
        (flowBiasStrength * 0.20) +
        (flowStrengthNorm * 0.15) +
        (imbalanceStrength * 0.20) +
        (regimeStrengthNorm * 0.25) +
        (recentMoveStrength * 0.20);

    return std::clamp(confidence, 0.0, 1.0);
}

double LowFrequencyRegimeContinuationStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double flowBiasComponent = std::abs(flow.aggressionBias) * 15.0;
    const double flowStrengthComponent = std::min(flow.totalAggressionQty, 10.0) * 1.5;
    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.20;
    const double regimeComponent = (regime.shortRangeBps + regime.activityBps) * 7.0;
    const double moveComponent = std::abs(context.recentMoveBps) * 4.0;

    double expectedMove =
        flowBiasComponent +
        flowStrengthComponent +
        imbalanceComponent +
        regimeComponent +
        moveComponent;

    if (expectedMove < 0.0) {
        expectedMove = 0.0;
    }

    return expectedMove;
}