#include "FlowPriceDelayStrategy.h"

#include <algorithm>
#include <cmath>

FlowPriceDelayStrategy::FlowPriceDelayStrategy(const Config& config)
    : config_(config) {}

SignalResult FlowPriceDelayStrategy::evaluate(const StrategyContext& context) const {
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

    if (snapshot.spreadBps > config_.maxSpreadBps) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "spread too high";
        result.isValid = false;
        return result;
    }

    if (!isRecentMoveSmallEnough(context)) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "price already moved too much for delay setup";
        result.isValid = false;
        return result;
    }

    const bool flowLong = isFlowLongStrong(context);
    const bool flowShort = isFlowShortStrong(context);

    const bool bookLong = isBookSupportiveForLong(context);
    const bool bookShort = isBookSupportiveForShort(context);

    const double confidence = calculateConfidence(context);
    const double expectedMoveBps = calculateExpectedMoveBps(context);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    if (!flowLong && !flowShort) {
        result.side = SignalSide::HOLD;
        result.reason = "flow too weak for delay setup";
        result.isValid = false;
        return result;
    }

    if (flowLong && !bookLong) {
        result.side = SignalSide::HOLD;
        result.reason = "book too contradictory for delay setup";
        result.isValid = false;
        return result;
    }

    if (flowShort && !bookShort) {
        result.side = SignalSide::HOLD;
        result.reason = "book too contradictory for delay setup";
        result.isValid = false;
        return result;
    }

    if (confidence < 0.45                                                                                                         ) {
        result.side = SignalSide::HOLD;
        result.reason = "delay setup confidence below min";
        result.isValid = false;
        return result;
    }

    if (expectedMoveBps < 10.0) {
        result.side = SignalSide::HOLD;
        result.reason = "delay setup expected move below min";
        result.isValid = false;
        return result;
    }

    if (flowLong && bookLong) {
        result.side = SignalSide::LONG;
        result.reason = "long flow-price delay detected";
        result.isValid = true;
        return result;
    }

    if (flowShort && bookShort) {
        result.side = SignalSide::SHORT;
        result.reason = "short flow-price delay detected";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "delay setup unresolved";
    result.isValid = false;
    return result;
}

double FlowPriceDelayStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;

    double flowBiasStrength = std::abs(flow.aggressionBias);
    if (flowBiasStrength > 1.0) {
        flowBiasStrength = 1.0;
    }

    double flowStrengthNorm = flow.totalAggressionQty / 5.0;
    flowStrengthNorm = std::clamp(flowStrengthNorm, 0.0, 1.0);

    double confidence =
        (flowBiasStrength * 0.45) +
        (flowStrengthNorm * 0.35) +
        (imbalanceStrength * 0.20);

    return std::clamp(confidence, 0.0, 1.0);
}

double FlowPriceDelayStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;

    const double flowBiasComponent = std::abs(flow.aggressionBias) * 12.0;
    const double flowStrengthComponent = std::min(flow.totalAggressionQty, 5.0) * 1.2;
    const double imbalanceComponent = std::abs(snapshot.imbalance - 50.0) * 0.15;

    double expectedMove = flowBiasComponent + flowStrengthComponent + imbalanceComponent;

    if (expectedMove < 0.0) {
        expectedMove = 0.0;
    }

    return expectedMove;
}

bool FlowPriceDelayStrategy::isFlowLongStrong(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= 0.60 &&
           context.flowSnapshot.totalAggressionQty >= 0.50;
}

bool FlowPriceDelayStrategy::isFlowShortStrong(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -0.60 &&
           context.flowSnapshot.totalAggressionQty >= 0.50;
}

bool FlowPriceDelayStrategy::isBookSupportiveForLong(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= 52.0;
}

bool FlowPriceDelayStrategy::isBookSupportiveForShort(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= 48.0;
}

bool FlowPriceDelayStrategy::isRecentMoveSmallEnough(const StrategyContext& context) const {
    (void)context;
    return true;
}