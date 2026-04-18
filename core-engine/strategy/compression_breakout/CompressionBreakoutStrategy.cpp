#include "CompressionBreakoutStrategy.h"

#include <cmath>
#include <sstream>

CompressionBreakoutStrategy::CompressionBreakoutStrategy(const Config& config)
    : config_(config) {}

SignalResult CompressionBreakoutStrategy::evaluate(const StrategyContext& context) const {
    SignalResult result;

    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;
    const double recentMoveBps = context.recentMoveBps;

    result.flowBias = flow.aggressionBias;
    result.flowStrength = flow.totalAggressionQty;

    // 1. Regime obrigatório
    if (!regime.tradable) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "regime not tradable";
        result.isValid = false;
        return result;
    }

    // 2. Spread atual muito aberto
    if (snapshot.spreadBps > config_.maxSpreadBps) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "spread too high";
        result.isValid = false;
        return result;
    }

    // 3. Queremos breakout, mas não explosão tardia
    if (std::abs(recentMoveBps) > config_.maxRecentMoveBps) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "recent move too large for breakout entry";
        result.isValid = false;
        return result;
    }

    const bool compressedEnough = regime.shortRangeBps <= 6.0;
    const bool activeEnough = regime.activityBps >= 0.5;
    const bool flowStrongEnough = flow.totalAggressionQty >= config_.minFlowStrength;

    const bool bookLong = snapshot.imbalance >= config_.imbalanceLongThreshold;
    const bool bookShort = snapshot.imbalance <= config_.imbalanceShortThreshold;

    const bool flowLong = flow.aggressionBias >= config_.minFlowBiasAbs;
    const bool flowShort = flow.aggressionBias <= -config_.minFlowBiasAbs;

    const double confidence = calculateConfidence(context);
    const double expectedMoveBps = calculateExpectedMoveBps(context);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    const bool confidenceOk = confidence >= config_.minConfidence;
    const bool expectedMoveOk = expectedMoveBps >= config_.minExpectedMoveBps;

    // LONG breakout
    if (compressedEnough &&
        activeEnough &&
        flowStrongEnough &&
        bookLong &&
        flowLong &&
        confidenceOk &&
        expectedMoveOk) {

        result.side = SignalSide::LONG;
        result.reason = "compression breakout long: compressed regime + buy flow + buy imbalance";
        result.isValid = true;
        return result;
    }

    // SHORT breakout
    if (compressedEnough &&
        activeEnough &&
        flowStrongEnough &&
        bookShort &&
        flowShort &&
        confidenceOk &&
        expectedMoveOk) {

        result.side = SignalSide::SHORT;
        result.reason = "compression breakout short: compressed regime + sell flow + sell imbalance";
        result.isValid = true;
        return result;
    }

    std::ostringstream reason;

    if (!compressedEnough) {
        reason << "range not compressed enough for breakout;";
    }

    if (!activeEnough) {
        reason << " regime not active enough;";
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

    if (!bookLong && !bookShort) {
        reason << " imbalance not extreme enough;";
    }

    if (bookLong && !flowLong && flowStrongEnough) {
        reason << " long book without long flow confirmation;";
    }

    if (bookShort && !flowShort && flowStrongEnough) {
        reason << " short book without short flow confirmation;";
    }

    if (bookLong && flowShort) {
        reason << " book-flow conflict: long book vs short flow;";
    }

    if (bookShort && flowLong) {
        reason << " book-flow conflict: short book vs long flow;";
    }

    std::string finalReason = reason.str();
    if (finalReason.empty()) {
        finalReason = "no valid compression breakout setup";
    }

    result.side = SignalSide::HOLD;
    result.reason = finalReason;
    result.isValid = false;
    return result;
}

double CompressionBreakoutStrategy::calculateConfidence(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;
    double flowStrength = std::abs(flow.aggressionBias);
    if (flowStrength > 1.0) flowStrength = 1.0;

    double compressionBonus = 0.0;
    if (regime.shortRangeBps <= 6.0) {
        compressionBonus = 0.15;
    }

    double confidence = (imbalanceStrength * 0.5) + (flowStrength * 0.35) + compressionBonus;

    if (confidence < 0.0) confidence = 0.0;
    if (confidence > 1.0) confidence = 1.0;

    return confidence;
}

double CompressionBreakoutStrategy::calculateExpectedMoveBps(const StrategyContext& context) const {
    const auto& snapshot = context.marketSnapshot;
    const auto& flow = context.flowSnapshot;
    const auto& regime = context.regimeSnapshot;

    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0);
    const double flowStrength = std::abs(flow.aggressionBias) * 20.0;
    const double compressionFactor = (regime.shortRangeBps <= 6.0) ? 5.0 : 0.0;

    double expectedMove = (imbalanceStrength * 0.4) + (flowStrength * 0.7) + compressionFactor;

    if (expectedMove < 0.0) expectedMove = 0.0;

    return expectedMove;
}