#include "SignalEngineLite.h"

#include <cmath>
#include <sstream>

SignalEngineLite::SignalEngineLite(const Config& config)
    : config_(config) {}

SignalResult SignalEngineLite::evaluate(const MarketSnapshot& snapshot,
                                        const FlowSnapshot& flowSnapshot,
                                        double recentMoveBps) const {
    SignalResult result;

    result.flowBias = flowSnapshot.aggressionBias;
    result.flowStrength = flowSnapshot.totalAggressionQty;

    // 1. Filtro de spread
    if (snapshot.spreadBps > config_.maxSpreadBps) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "spread too high";
        result.isValid = false;
        return result;
    }

    // 2. Filtro de movimento recente
    if (std::abs(recentMoveBps) > config_.maxRecentMoveBps) {
        result.side = SignalSide::HOLD;
        result.confidence = 0.0;
        result.expectedMoveBps = 0.0;
        result.reason = "recent move too large";
        result.isValid = false;
        return result;
    }

    // 3. Cálculo base
    const double confidence = calculateConfidence(snapshot, flowSnapshot, recentMoveBps);
    const double expectedMoveBps = calculateExpectedMoveBps(snapshot, flowSnapshot, recentMoveBps);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    // 4. Condições estruturais
    const bool bookLong = snapshot.imbalance >= config_.imbalanceLongThreshold;
    const bool bookShort = snapshot.imbalance <= config_.imbalanceShortThreshold;

    const bool flowStrongEnough = flowSnapshot.totalAggressionQty >= config_.minFlowStrength;
    const bool flowLong = flowSnapshot.aggressionBias >= config_.minFlowBiasAbs;
    const bool flowShort = flowSnapshot.aggressionBias <= -config_.minFlowBiasAbs;

    const bool confidenceOk = confidence >= config_.minConfidence;
    const bool expectedMoveOk = expectedMoveBps >= config_.minExpectedMoveBps;

    // 5. LONG válido
    if (bookLong &&
        flowStrongEnough &&
        flowLong &&
        confidenceOk &&
        expectedMoveOk) {

        result.side = SignalSide::LONG;
        result.reason = "long signal: imbalance + strong aggressive buy flow + controlled spread + low recent move";
        result.isValid = true;
        return result;
    }

    // 6. SHORT válido
    if (bookShort &&
        flowStrongEnough &&
        flowShort &&
        confidenceOk &&
        expectedMoveOk) {

        result.side = SignalSide::SHORT;
        result.reason = "short signal: negative imbalance + strong aggressive sell flow + controlled spread + low recent move";
        result.isValid = true;
        return result;
    }

    // 7. Diagnóstico detalhado
    std::ostringstream reason;

    if (!flowStrongEnough) {
        reason << "flow too weak;";
    }

    if (!confidenceOk) {
        reason << " confidence below min;";
    }

    if (!expectedMoveOk) {
        reason << " expected move below min;";
    }

    if (bookLong && !flowLong && flowStrongEnough) {
        reason << " book long but flow not confirming long;";
    }

    if (bookShort && !flowShort && flowStrongEnough) {
        reason << " book short but flow not confirming short;";
    }

    if (bookLong && flowShort) {
        reason << " book-flow conflict: long book vs short flow;";
    }

    if (bookShort && flowLong) {
        reason << " book-flow conflict: short book vs long flow;";
    }

    if (!bookLong && !bookShort) {
        reason << " imbalance not extreme enough;";
    }

    std::string finalReason = reason.str();
    if (finalReason.empty()) {
        finalReason = "no valid setup";
    }

    result.side = SignalSide::HOLD;
    result.reason = finalReason;
    result.isValid = false;
    return result;
}

double SignalEngineLite::calculateConfidence(const MarketSnapshot& snapshot,
                                             const FlowSnapshot& flowSnapshot,
                                             double recentMoveBps) const {
    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;

    double flowStrength = std::abs(flowSnapshot.aggressionBias);
    if (flowStrength > 1.0) flowStrength = 1.0;

    double spreadPenalty = snapshot.spreadBps / config_.maxSpreadBps;
    if (spreadPenalty > 1.0) spreadPenalty = 1.0;

    double movePenalty = std::abs(recentMoveBps) / config_.maxRecentMoveBps;
    if (movePenalty > 1.0) movePenalty = 1.0;

    double combinedStrength = (imbalanceStrength * 0.55) + (flowStrength * 0.45);

    double confidence = combinedStrength
                        * (1.0 - 0.5 * spreadPenalty)
                        * (1.0 - 0.5 * movePenalty);

    if (confidence < 0.0) confidence = 0.0;
    if (confidence > 1.0) confidence = 1.0;

    return confidence;
}

double SignalEngineLite::calculateExpectedMoveBps(const MarketSnapshot& snapshot,
                                                  const FlowSnapshot& flowSnapshot,
                                                  double recentMoveBps) const {
    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0);
    const double flowStrength = std::abs(flowSnapshot.aggressionBias) * 20.0;

    double expectedMove = (imbalanceStrength * 0.5) + (flowStrength * 0.8);

    expectedMove -= std::abs(recentMoveBps) * 0.5;

    if (expectedMove < 0.0) expectedMove = 0.0;

    return expectedMove;
}