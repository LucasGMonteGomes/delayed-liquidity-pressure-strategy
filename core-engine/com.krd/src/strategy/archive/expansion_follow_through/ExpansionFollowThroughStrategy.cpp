#include "ExpansionFollowThroughStrategy.h"

#include <algorithm>
#include <cmath>

ExpansionFollowThroughStrategy::ExpansionFollowThroughStrategy(const Config& config)
    : config_(config) {}

SignalResult ExpansionFollowThroughStrategy::evaluate(const StrategyContext& context) const {
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

    if (!isRegimeExpandedEnough(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "regime expansion too weak for follow through";
        result.isValid = false;
        return result;
    }

    const bool longFlow = isLongFlowConfirmed(context);
    const bool shortFlow = isShortFlowConfirmed(context);

    const bool longBook = isLongBookSupportive(context);
    const bool shortBook = isShortBookSupportive(context);

    const bool longMove = isRecentMoveAlignedForLong(context);
    const bool shortMove = isRecentMoveAlignedForShort(context);

    result.confidence = calculateConfidence(context);
    result.expectedMoveBps = calculateExpectedMoveBps(context);

    if (!longFlow && !shortFlow) {
        result.side = SignalSide::HOLD;
        result.reason = "flow not strong enough for follow through";
        result.isValid = false;
        return result;
    }

    if (longFlow && (!longBook || !longMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "long continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (shortFlow && (!shortBook || !shortMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "short continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (result.confidence < 0.60) {
        result.side = SignalSide::HOLD;
        result.reason = "follow through confidence below min";
        result.isValid = false;
        return result;
    }

    if (result.expectedMoveBps < 18.0) {
        result.side = SignalSide::HOLD;
        result.reason = "follow through expected move below min";
        result.isValid = false;
        return result;
    }

    if (longFlow && longBook && longMove) {
        result.side = SignalSide::LONG;
        result.reason = "long expansion follow through detected";
        result.isValid = true;
        return result;
    }

    if (shortFlow && shortBook && shortMove) {
        result.side = SignalSide::SHORT;
        result.reason = "short expansion follow through detected";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "follow through unresolved";
    result.isValid = false;
    return result;
}

bool ExpansionFollowThroughStrategy::isSpreadAcceptable(const StrategyContext& context) const {
    return context.marketSnapshot.spreadBps <= config_.maxSpreadBps;
}

bool ExpansionFollowThroughStrategy::isRegimeExpandedEnough(const StrategyContext& context) const {
    return context.regimeSnapshot.shortRangeBps >= 0.30 &&
           context.regimeSnapshot.activityBps >= 0.30 &&
           context.regimeSnapshot.hasRecentFlow;
}

bool ExpansionFollowThroughStrategy::isLongFlowConfirmed(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= 0.65 &&
           context.flowSnapshot.totalAggressionQty >= 1.0;
}

bool ExpansionFollowThroughStrategy::isShortFlowConfirmed(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -0.65 &&
           context.flowSnapshot.totalAggressionQty >= 1.0;
}

bool ExpansionFollowThroughStrategy::isLongBookSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= 55.0;
}

bool ExpansionFollowThroughStrategy::isShortBookSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= 45.0;
}

bool ExpansionFollowThroughStrategy::isRecentMoveAlignedForLong(const StrategyContext& context) const {
    return context.recentMoveBps >= 0.10;
}

bool ExpansionFollowThroughStrategy::isRecentMoveAlignedForShort(const StrategyContext& context) const {
    return context.recentMoveBps <= -0.10;
}

double ExpansionFollowThroughStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    flowBiasStrength = std::clamp(flowBiasStrength, 0.0, 1.0);

    double flowStrengthNorm = flow.totalAggressionQty / 5.0;
    flowStrengthNorm = std::clamp(flowStrengthNorm, 0.0, 1.0);

    double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;
    imbalanceStrength = std::clamp(imbalanceStrength, 0.0, 1.0);

    double regimeExpansion = (regime.shortRangeBps + regime.activityBps) / 2.0;
    double regimeExpansionNorm = std::clamp(regimeExpansion / 2.0, 0.0, 1.0);

    double recentMoveStrength = std::abs(context.recentMoveBps) / 1.0;
    recentMoveStrength = std::clamp(recentMoveStrength, 0.0, 1.0);

    const double confidence =
        (flowBiasStrength * 0.25) +
        (flowStrengthNorm * 0.20) +
        (imbalanceStrength * 0.15) +
        (regimeExpansionNorm * 0.25) +
        (recentMoveStrength * 0.15);

    return std::clamp(confidence, 0.0, 1.0);
}

double ExpansionFollowThroughStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double flowBiasComponent = std::abs(flow.aggressionBias) * 10.0;
    const double flowStrengthComponent = std::min(flow.totalAggressionQty, 5.0) * 1.0;
    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.12;
    const double regimeComponent = (regime.shortRangeBps + regime.activityBps) * 4.0;
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