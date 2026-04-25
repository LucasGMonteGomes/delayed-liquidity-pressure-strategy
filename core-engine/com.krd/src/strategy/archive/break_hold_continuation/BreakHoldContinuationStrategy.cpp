#include "BreakHoldContinuationStrategy.h"

#include <algorithm>
#include <cmath>

BreakHoldContinuationStrategy::BreakHoldContinuationStrategy(const Config& config)
    : config_(config) {}

SignalResult BreakHoldContinuationStrategy::evaluate(const StrategyContext& context) const {
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
        result.reason = "regime not supportive for break-hold continuation";
        result.isValid = false;
        return result;
    }

    const bool longFlow = isLongFlowAligned(context);
    const bool shortFlow = isShortFlowAligned(context);

    const bool longBook = isLongBookSupportive(context);
    const bool shortBook = isShortBookSupportive(context);

    const bool longMove = isLongMoveStructured(context);
    const bool shortMove = isShortMoveStructured(context);

    result.confidence = calculateConfidence(context);
    result.expectedMoveBps = calculateExpectedMoveBps(context);

    if (!longFlow && !shortFlow) {
        result.side = SignalSide::HOLD;
        result.reason = "flow not aligned for break-hold continuation";
        result.isValid = false;
        return result;
    }

    if (longFlow && (!longBook || !longMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "long break-hold continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (shortFlow && (!shortBook || !shortMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "short break-hold continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (result.confidence < 0.70) {
        result.side = SignalSide::HOLD;
        result.reason = "break-hold continuation confidence below min";
        result.isValid = false;
        return result;
    }

    if (result.expectedMoveBps < 42.0) {
        result.side = SignalSide::HOLD;
        result.reason = "break-hold continuation expected move below min";
        result.isValid = false;
        return result;
    }

    if (longFlow && longBook && longMove) {
        result.side = SignalSide::LONG;
        result.reason = "long break-hold continuation detected";
        result.isValid = true;
        return result;
    }

    if (shortFlow && shortBook && shortMove) {
        result.side = SignalSide::SHORT;
        result.reason = "short break-hold continuation detected";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "break-hold continuation unresolved";
    result.isValid = false;
    return result;
}

bool BreakHoldContinuationStrategy::isSpreadAcceptable(const StrategyContext& context) const {
    return context.marketSnapshot.spreadBps <= config_.maxSpreadBps;
}

bool BreakHoldContinuationStrategy::isRegimeSupportive(const StrategyContext& context) const {
    return context.regimeSnapshot.shortRangeBps >= 1.0 &&
           context.regimeSnapshot.activityBps >= 1.0 &&
           context.regimeSnapshot.hasRecentFlow;
}

bool BreakHoldContinuationStrategy::isLongFlowAligned(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= 0.72 &&
           context.flowSnapshot.totalAggressionQty >= 1.10;
}

bool BreakHoldContinuationStrategy::isShortFlowAligned(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -0.65 &&
           context.flowSnapshot.totalAggressionQty >= 0.90;
}

bool BreakHoldContinuationStrategy::isLongBookSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= 62.0;
}

bool BreakHoldContinuationStrategy::isShortBookSupportive(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= 42.0;
}

bool BreakHoldContinuationStrategy::isLongMoveStructured(const StrategyContext& context) const {
    return context.recentMoveBps >= 1.30 &&
           context.recentMoveBps <= 8.00;
}

bool BreakHoldContinuationStrategy::isShortMoveStructured(const StrategyContext& context) const {
    return context.recentMoveBps <= -1.00 &&
           std::abs(context.recentMoveBps) <= 8.00;
}

double BreakHoldContinuationStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    flowBiasStrength = std::clamp(flowBiasStrength, 0.0, 1.0);

    double flowStrengthNorm = flow.totalAggressionQty / 5.0;
    flowStrengthNorm = std::clamp(flowStrengthNorm, 0.0, 1.0);

    double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;
    imbalanceStrength = std::clamp(imbalanceStrength, 0.0, 1.0);

    double regimeStrength = (regime.shortRangeBps + regime.activityBps) / 2.0;
    double regimeStrengthNorm = std::clamp(regimeStrength / 4.0, 0.0, 1.0);

    double recentMoveStrength = std::abs(context.recentMoveBps) / 8.0;
    recentMoveStrength = std::clamp(recentMoveStrength, 0.0, 1.0);

    const double confidence =
        (flowBiasStrength * 0.22) +
        (flowStrengthNorm * 0.18) +
        (imbalanceStrength * 0.18) +
        (regimeStrengthNorm * 0.20) +
        (recentMoveStrength * 0.22);

    return std::clamp(confidence, 0.0, 1.0);
}

double BreakHoldContinuationStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double flowBiasComponent = std::abs(flow.aggressionBias) * 12.0;
    const double flowStrengthComponent = std::min(flow.totalAggressionQty, 6.0) * 1.1;
    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.14;
    const double regimeComponent = (regime.shortRangeBps + regime.activityBps) * 4.5;
    const double moveComponent = std::abs(context.recentMoveBps) * 2.8;

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