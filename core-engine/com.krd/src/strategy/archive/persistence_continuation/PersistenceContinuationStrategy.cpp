#include "PersistenceContinuationStrategy.h"

#include <algorithm>
#include <cmath>

PersistenceContinuationStrategy::PersistenceContinuationStrategy(const Config& config)
    : config_(config) {}

SignalResult PersistenceContinuationStrategy::evaluate(const StrategyContext& context) const {
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

    if (!isRegimeSupportive(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "regime not supportive for persistence continuation";
        result.isValid = false;
        return result;
    }

    const bool longFlow = isLongFlowPersistent(context);
    const bool shortFlow = isShortFlowPersistent(context);

    const bool longBook = isLongBookSupportive(context);
    const bool shortBook = isShortBookSupportive(context);

    const bool longMove = isLongMovePersistent(context);
    const bool shortMove = isShortMovePersistent(context);

    result.confidence = calculateConfidence(context);
    result.expectedMoveBps = calculateExpectedMoveBps(context);

    if (!longFlow && !shortFlow) {
        result.side = SignalSide::HOLD;
        result.reason = "flow not persistent enough";
        result.isValid = false;
        return result;
    }

    if (longFlow && (!longBook || !longMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "long persistence continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (shortFlow && (!shortBook || !shortMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "short persistence continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (result.confidence < 0.68) {
        result.side = SignalSide::HOLD;
        result.reason = "persistence continuation confidence below min";
        result.isValid = false;
        return result;
    }

    if (result.expectedMoveBps < 40.0) {
        result.side = SignalSide::HOLD;
        result.reason = "persistence continuation expected move below min";
        result.isValid = false;
        return result;
    }

    if (longFlow && longBook && longMove) {
        result.side = SignalSide::LONG;
        result.reason = "long persistence continuation detected";
        result.isValid = true;
        return result;
    }

    if (shortFlow && shortBook && shortMove) {
        result.side = SignalSide::SHORT;
        result.reason = "short persistence continuation detected";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "persistence continuation unresolved";
    result.isValid = false;
    return result;
}

bool PersistenceContinuationStrategy::isSpreadAcceptable(const StrategyContext& context) const {
    return context.marketSnapshot.spreadBps <= config_.maxSpreadBps;
}

bool PersistenceContinuationStrategy::isRegimeSupportive(const StrategyContext& context) const {
    return context.regimeSnapshot.shortRangeBps >= 1.0 &&
           context.regimeSnapshot.activityBps >= 1.0 &&
           context.regimeSnapshot.hasRecentFlow;
}

bool PersistenceContinuationStrategy::isLongFlowPersistent(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= 0.65 &&
           context.flowSnapshot.totalAggressionQty >= 0.80;
}

bool PersistenceContinuationStrategy::isShortFlowPersistent(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -0.65 &&
           context.flowSnapshot.totalAggressionQty >= 0.80;
}

bool PersistenceContinuationStrategy::isLongBookSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= 60.0;
}

bool PersistenceContinuationStrategy::isShortBookSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= 40.0;
}

bool PersistenceContinuationStrategy::isLongMovePersistent(const StrategyContext& context) const {
    return context.recentMoveBps >= 0.80 &&
           context.recentMoveBps <= config_.maxRecentMoveBps;
}

bool PersistenceContinuationStrategy::isShortMovePersistent(const StrategyContext& context) const {
    return context.recentMoveBps <= -0.80 &&
           std::abs(context.recentMoveBps) <= config_.maxRecentMoveBps;
}

double PersistenceContinuationStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    flowBiasStrength = std::clamp(flowBiasStrength, 0.0, 1.0);

    double flowStrengthNorm = flow.totalAggressionQty / 4.0;
    flowStrengthNorm = std::clamp(flowStrengthNorm, 0.0, 1.0);

    double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;
    imbalanceStrength = std::clamp(imbalanceStrength, 0.0, 1.0);

    double regimeStrength = (regime.shortRangeBps + regime.activityBps) / 2.0;
    double regimeStrengthNorm = std::clamp(regimeStrength / 4.0, 0.0, 1.0);

    double recentMoveStrength = std::abs(context.recentMoveBps) / 4.0;
    recentMoveStrength = std::clamp(recentMoveStrength, 0.0, 1.0);

    const double confidence =
        (flowBiasStrength * 0.22) +
        (flowStrengthNorm * 0.18) +
        (imbalanceStrength * 0.16) +
        (regimeStrengthNorm * 0.22) +
        (recentMoveStrength * 0.22);

    return std::clamp(confidence, 0.0, 1.0);
}

double PersistenceContinuationStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double flowBiasComponent = std::abs(flow.aggressionBias) * 12.0;
    const double flowStrengthComponent = std::min(flow.totalAggressionQty, 6.0) * 1.2;
    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.15;
    const double regimeComponent = (regime.shortRangeBps + regime.activityBps) * 5.0;
    const double moveComponent = std::abs(context.recentMoveBps) * 3.0;

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