#include "ConflictResolutionStrategy.h"

#include <cmath>
#include <sstream>

ConflictResolutionStrategy::ConflictResolutionStrategy(const Config &config)
    : config_(config) {
}

SignalResult ConflictResolutionStrategy::evaluate(const StrategyContext &context) const {
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

    const std::string key = makeKey(context);
    auto &state = conflictStates_[key];

    const bool bookLong = isBookLong(context);
    const bool bookShort = isBookShort(context);
    const bool flowLong = isFlowLong(context);
    const bool flowShort = isFlowShort(context);
    const bool flowStrongEnough = isFlowStrongEnough(context);

    const double confidence = calculateConfidence(context);
    const double expectedMoveBps = calculateExpectedMoveBps(context);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    const bool confidenceOk = confidence >= config_.minConfidence;
    const bool expectedMoveOk = expectedMoveBps >= config_.minExpectedMoveBps;

    const bool conflictLongVsShort = bookLong && flowShort && flowStrongEnough;
    const bool conflictShortVsLong = bookShort && flowLong && flowStrongEnough;

    if (conflictLongVsShort) {
        state.active = true;
        state.type = ConflictType::BOOK_LONG_FLOW_SHORT;
        state.detectedAtMs = snapshot.timestampMs;
        state.conflictStrength = std::abs(flow.aggressionBias);

        result.side = SignalSide::HOLD;
        result.reason = "conflict detected: long book vs short flow";
        result.isValid = false;
        return result;
    }

    if (conflictShortVsLong) {
        state.active = true;
        state.type = ConflictType::BOOK_SHORT_FLOW_LONG;
        state.detectedAtMs = snapshot.timestampMs;
        state.conflictStrength = std::abs(flow.aggressionBias);

        result.side = SignalSide::HOLD;
        result.reason = "conflict detected: short book vs long flow";
        result.isValid = false;
        return result;
    }

    if (!state.active) {
        result.side = SignalSide::HOLD;
        result.reason = "no active conflict to resolve";
        result.isValid = false;
        return result;
    }

    const std::int64_t conflictAgeMs = snapshot.timestampMs - state.detectedAtMs;
    const std::int64_t maxConflictLifetimeMs = 15000;

    if (conflictAgeMs > maxConflictLifetimeMs) {
        state.active = false;
        state.type = ConflictType::NONE;
        state.detectedAtMs = 0;
        state.conflictStrength = 0.0;

        result.side = SignalSide::HOLD;
        result.reason = "conflict expired";
        result.isValid = false;
        return result;
    }

    if (state.type == ConflictType::BOOK_LONG_FLOW_SHORT) {
        const bool resolutionLong = bookLong && flowLong && flowStrongEnough;

        if (resolutionLong && confidenceOk && expectedMoveOk) {
            state.active = false;
            state.type = ConflictType::NONE;
            state.detectedAtMs = 0;
            state.conflictStrength = 0.0;

            result.side = SignalSide::LONG;
            result.reason = "resolution long confirmed: long book regained control with long flow";
            result.isValid = true;
            return result;
        }

        std::ostringstream reason;
        reason << "waiting for long conflict resolution;";

        if (!bookLong) {
            reason << " long book lost;";
        }

        if (!flowLong) {
            reason << " long flow not confirmed;";
        }

        if (!flowStrongEnough) {
            reason << " flow too weak;";
        }

        if (!confidenceOk) {
            reason << " confidence below min;";
        }

        if (!expectedMoveOk) {
            reason << " expected move below min;";
        }

        result.side = SignalSide::HOLD;
        result.reason = reason.str();
        result.isValid = false;
        return result;
    }

    if (state.type == ConflictType::BOOK_SHORT_FLOW_LONG) {
        const bool resolutionShort = bookShort && flowShort && flowStrongEnough;

        if (resolutionShort && confidenceOk && expectedMoveOk) {
            state.active = false;
            state.type = ConflictType::NONE;
            state.detectedAtMs = 0;
            state.conflictStrength = 0.0;

            result.side = SignalSide::SHORT;
            result.reason = "resolution short confirmed: short book regained control with short flow";
            result.isValid = true;
            return result;
        }

        std::ostringstream reason;
        reason << "waiting for short conflict resolution;";

        if (!bookShort) {
            reason << " short book lost;";
        }

        if (!flowShort) {
            reason << " short flow not confirmed;";
        }

        if (!flowStrongEnough) {
            reason << " flow too weak;";
        }

        if (!confidenceOk) {
            reason << " confidence below min;";
        }

        if (!expectedMoveOk) {
            reason << " expected move below min;";
        }

        result.side = SignalSide::HOLD;
        result.reason = reason.str();
        result.isValid = false;
        return result;
    }

    result.side = SignalSide::HOLD;
    result.reason = "unknown conflict state";
    result.isValid = false;
    return result;
}

std::string ConflictResolutionStrategy::makeKey(const StrategyContext &context) const {
    return context.marketSnapshot.exchange + "|" + context.marketSnapshot.symbol;
}

double ConflictResolutionStrategy::calculateConfidence(const StrategyContext &context) const {
    const auto &snapshot = context.marketSnapshot;
    const auto &flow = context.flowSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;

    double flowStrength = std::abs(flow.aggressionBias);
    if (flowStrength > 1.0) {
        flowStrength = 1.0;
    }

    double confidence = (imbalanceStrength * 0.55) + (flowStrength * 0.45);

    if (confidence < 0.0) {
        confidence = 0.0;
    }
    if (confidence > 1.0) {
        confidence = 1.0;
    }

    return confidence;
}

double ConflictResolutionStrategy::calculateExpectedMoveBps(const StrategyContext &context) const {
    const auto &snapshot = context.marketSnapshot;
    const auto &flow = context.flowSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0);
    const double flowStrength = std::abs(flow.aggressionBias) * 20.0;

    double expectedMove = (imbalanceStrength * 0.45) + (flowStrength * 0.75);

    if (expectedMove < 0.0) {
        expectedMove = 0.0;
    }

    return expectedMove;
}

bool ConflictResolutionStrategy::isBookLong(const StrategyContext &context) const {
    return context.marketSnapshot.imbalance >= config_.imbalanceLongThreshold;
}

bool ConflictResolutionStrategy::isBookShort(const StrategyContext &context) const {
    return context.marketSnapshot.imbalance <= config_.imbalanceShortThreshold;
}

bool ConflictResolutionStrategy::isFlowLong(const StrategyContext &context) const {
    return context.flowSnapshot.aggressionBias >= config_.minFlowBiasAbs;
}

bool ConflictResolutionStrategy::isFlowShort(const StrategyContext &context) const {
    return context.flowSnapshot.aggressionBias <= -config_.minFlowBiasAbs;
}

bool ConflictResolutionStrategy::isFlowStrongEnough(const StrategyContext &context) const {
    return context.flowSnapshot.totalAggressionQty >= config_.minFlowStrength;
}
