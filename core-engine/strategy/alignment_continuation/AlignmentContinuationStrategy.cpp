#include "AlignmentContinuationStrategy.h"

#include <cmath>

AlignmentContinuationStrategy::AlignmentContinuationStrategy(const Config& config)
    : config_(config) {}

SignalResult AlignmentContinuationStrategy::evaluate(const StrategyContext& context) const {
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

    const bool flowStrongEnough = isFlowStrongEnough(context);

    if (!flowStrongEnough) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "flow too weak";
        result.isValid = false;
        return result;
    }

    const bool bookLong = isBookLong(context);
    const bool bookShort = isBookShort(context);

    const bool flowLong = isFlowLong(context);
    const bool flowShort = isFlowShort(context);

    const bool flowNeutralToLong = isFlowNeutralToLong(context);
    const bool flowNeutralToShort = isFlowNeutralToShort(context);

    const double confidence = calculateConfidence(context);
    const double expectedMoveBps = calculateExpectedMoveBps(context);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    const bool confidenceOk = confidence >= config_.minConfidence;
    const bool expectedMoveOk = expectedMoveBps >= config_.minExpectedMoveBps;

    if (bookLong && (flowLong || flowNeutralToLong)) {
        if (!confidenceOk) {
            result.side = SignalSide::HOLD;
            result.reason = "long alignment but confidence below min";
            result.isValid = false;
            return result;
        }

        if (!expectedMoveOk) {
            result.side = SignalSide::HOLD;
            result.reason = "long alignment but expected move below min";
            result.isValid = false;
            return result;
        }

        result.side = SignalSide::LONG;
        result.reason = "long alignment confirmed: strong book with supportive flow";
        result.isValid = true;
        return result;
    }

    if (bookShort && (flowShort || flowNeutralToShort)) {
        if (!confidenceOk) {
            result.side = SignalSide::HOLD;
            result.reason = "short alignment but confidence below min";
            result.isValid = false;
            return result;
        }

        if (!expectedMoveOk) {
            result.side = SignalSide::HOLD;
            result.reason = "short alignment but expected move below min";
            result.isValid = false;
            return result;
        }

        result.side = SignalSide::SHORT;
        result.reason = "short alignment confirmed: strong book with supportive flow";
        result.isValid = true;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "book-flow alignment not strong enough";
    result.isValid = false;
    return result;
}

double AlignmentContinuationStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;

    double flowStrength = std::abs(flow.aggressionBias);
    if (flowStrength > 1.0) {
        flowStrength = 1.0;
    }

    double confidence = (imbalanceStrength * 0.60) + (flowStrength * 0.40);

    if (confidence < 0.0) {
        confidence = 0.0;
    }
    if (confidence > 1.0) {
        confidence = 1.0;
    }

    return confidence;
}

double AlignmentContinuationStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0);
    const double flowStrength = std::abs(flow.aggressionBias) * 20.0;

    double expectedMove = (imbalanceStrength * 0.50) + (flowStrength * 0.70);

    if (expectedMove < 0.0) {
        expectedMove = 0.0;
    }

    return expectedMove;
}

bool AlignmentContinuationStrategy::isBookLong(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance >= config_.imbalanceLongThreshold;
}

bool AlignmentContinuationStrategy::isBookShort(const StrategyContext& context) const {
    return context.marketSnapshot.imbalance <= config_.imbalanceShortThreshold;
}

bool AlignmentContinuationStrategy::isFlowLong(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias >= config_.minFlowBiasAbs;
}

bool AlignmentContinuationStrategy::isFlowShort(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias <= -config_.minFlowBiasAbs;
}

bool AlignmentContinuationStrategy::isFlowNeutralToLong(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias > -0.10;
}

bool AlignmentContinuationStrategy::isFlowNeutralToShort(const StrategyContext& context) const {
    return context.flowSnapshot.aggressionBias < 0.10;
}

bool AlignmentContinuationStrategy::isFlowStrongEnough(const StrategyContext& context) const {
    return context.flowSnapshot.totalAggressionQty >= config_.minFlowStrength;
}