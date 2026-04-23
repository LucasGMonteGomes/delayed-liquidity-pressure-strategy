#include "IntradayTrendContinuationStrategy.h"

#include <algorithm>
#include <cmath>

IntradayTrendContinuationStrategy::IntradayTrendContinuationStrategy(const Config& config)
    : config_(config) {}

SignalResult IntradayTrendContinuationStrategy::evaluate(const StrategyContext& context) const {
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
        result.reason = "regime too weak for intraday continuation";
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
        result.reason = "flow not aligned for intraday continuation";
        result.isValid = false;
        return result;
    }

    if (longFlow && (!longBook || !longMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "long intraday continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (shortFlow && (!shortBook || !shortMove)) {
        result.side = SignalSide::HOLD;
        result.reason = "short intraday continuation not aligned";
        result.isValid = false;
        return result;
    }

    if (result.confidence < 0.68) {
        result.side = SignalSide::HOLD;
        result.reason = "intraday continuation confidence below min";
        result.isValid = false;
        return result;
    }

    if (result.expectedMoveBps < 26.0) {
        result.side = SignalSide::HOLD;
        result.reason = "intraday continuation expected move below min";
        result.isValid = false;
        return result;
    }

    if (longFlow && longBook && longMove) {
        result.side = SignalSide::LONG;
        result.reason = "long intraday continuation detected";
        result.isValid = true;
        return result;
    }

    if (shortFlow && shortBook && shortMove) {
        result.side = SignalSide::SHORT;
        result.reason = "short intraday continuation detected";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "intraday continuation unresolved";
    result.isValid = false;
    return result;
}

bool IntradayTrendContinuationStrategy::isSpreadAcceptable(const StrategyContext& context) const {
    return context.marketSnapshot.spreadBps <= config_.maxSpreadBps;
}

bool IntradayTrendContinuationStrategy::isRegimeStrongEnough(const StrategyContext& context) const {
    return context.regimeSnapshot.shortRangeBps >= 0.60 &&
           context.regimeSnapshot.activityBps >= 0.60 &&
           context.regimeSnapshot.hasRecentFlow;
}

bool IntradayTrendContinuationStrategy::isLongFlowAligned(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= 0.70 &&
           context.flowSnapshot.totalAggressionQty >= 1.50;
}

bool IntradayTrendContinuationStrategy::isShortFlowAligned(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -0.70 &&
           context.flowSnapshot.totalAggressionQty >= 1.50;
}

bool IntradayTrendContinuationStrategy::isLongBookAligned(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= 58.0;
}

bool IntradayTrendContinuationStrategy::isShortBookAligned(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= 42.0;
}

bool IntradayTrendContinuationStrategy::isLongMoveConfirmed(const StrategyContext& context) const {
    return context.recentMoveBps >= 0.20;
}

bool IntradayTrendContinuationStrategy::isShortMoveConfirmed(const StrategyContext& context) const {
    return context.recentMoveBps <= -0.20;
}

double IntradayTrendContinuationStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    flowBiasStrength = std::clamp(flowBiasStrength, 0.0, 1.0);

    double flowStrengthNorm = flow.totalAggressionQty / 6.0;
    flowStrengthNorm = std::clamp(flowStrengthNorm, 0.0, 1.0);

    double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;
    imbalanceStrength = std::clamp(imbalanceStrength, 0.0, 1.0);

    double regimeStrength = (regime.shortRangeBps + regime.activityBps) / 2.0;
    double regimeStrengthNorm = std::clamp(regimeStrength / 3.0, 0.0, 1.0);

    double recentMoveStrength = std::abs(context.recentMoveBps) / 2.0;
    recentMoveStrength = std::clamp(recentMoveStrength, 0.0, 1.0);

    const double confidence =
        (flowBiasStrength * 0.20) +
        (flowStrengthNorm * 0.20) +
        (imbalanceStrength * 0.15) +
        (regimeStrengthNorm * 0.25) +
        (recentMoveStrength * 0.20);

    return std::clamp(confidence, 0.0, 1.0);
}

double IntradayTrendContinuationStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double flowBiasComponent = std::abs(flow.aggressionBias) * 12.0;
    const double flowStrengthComponent = std::min(flow.totalAggressionQty, 8.0) * 1.2;
    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.15;
    const double regimeComponent = (regime.shortRangeBps + regime.activityBps) * 5.0;
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