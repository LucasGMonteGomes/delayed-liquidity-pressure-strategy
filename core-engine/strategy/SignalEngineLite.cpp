#include "SignalEngineLite.h"

#include <cmath>

SignalEngineLite::SignalEngineLite(const Config &config)
    : config_(config) {
}

SignalResult SignalEngineLite::evaluate(const MarketSnapshot &snapshot,
                                        const FlowSnapshot &flowSnapshot,
                                        double recentMoveBps) const {
    SignalResult result;

    // Preenche contexto de fluxo no resultado
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

    // 3. Cálculo de confiança e movimento esperado
    const double confidence = calculateConfidence(snapshot, flowSnapshot, recentMoveBps);
    const double expectedMoveBps = calculateExpectedMoveBps(snapshot, flowSnapshot, recentMoveBps);

    result.confidence = confidence;
    result.expectedMoveBps = expectedMoveBps;

    // 4. Regras LONG
    const bool bookLong = snapshot.imbalance >= config_.imbalanceLongThreshold;
    const bool flowStrongEnough = flowSnapshot.totalAggressionQty >= config_.minFlowStrength;
    const bool flowLong = flowSnapshot.aggressionBias >= config_.minFlowBiasAbs;

    if (bookLong &&
        flowStrongEnough &&
        flowLong &&
        confidence >= config_.minConfidence &&
        expectedMoveBps >= config_.minExpectedMoveBps) {
        result.side = SignalSide::LONG;
        result.reason = "long signal: imbalance + strong aggressive buy flow + controlled spread + low recent move";
        result.isValid = true;
        return result;
    }

    // 5. Regras SHORT
    const bool bookShort = snapshot.imbalance <= config_.imbalanceShortThreshold;
    const bool flowShort = flowSnapshot.aggressionBias <= -config_.minFlowBiasAbs;

    if (bookShort &&
        flowStrongEnough &&
        flowShort &&
        confidence >= config_.minConfidence &&
        expectedMoveBps >= config_.minExpectedMoveBps) {
        result.side = SignalSide::SHORT;
        result.reason =
                "short signal: negative imbalance + strong aggressive sell flow + controlled spread + low recent move";
        result.isValid = true;
        return result;
    }

    // 6. Sem trade
    result.side = SignalSide::HOLD;
    result.reason = "no valid setup";
    result.isValid = false;
    return result;
}

double SignalEngineLite::calculateConfidence(const MarketSnapshot &snapshot,
                                             const FlowSnapshot &flowSnapshot,
                                             double recentMoveBps) const {
    // Força do imbalance (0 a 1)
    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0) / 50.0;

    // Força do fluxo (0 a 1)
    double flowStrength = std::abs(flowSnapshot.aggressionBias);
    if (flowStrength > 1.0) flowStrength = 1.0;

    // Penaliza spread alto
    double spreadPenalty = snapshot.spreadBps / config_.maxSpreadBps;
    if (spreadPenalty > 1.0) spreadPenalty = 1.0;

    // Penaliza mercado que já andou
    double movePenalty = std::abs(recentMoveBps) / config_.maxRecentMoveBps;
    if (movePenalty > 1.0) movePenalty = 1.0;

    // Mistura book + flow
    double combinedStrength = (imbalanceStrength * 0.55) + (flowStrength * 0.45);

    double confidence = combinedStrength
                        * (1.0 - 0.5 * spreadPenalty)
                        * (1.0 - 0.5 * movePenalty);

    if (confidence < 0.0) confidence = 0.0;
    if (confidence > 1.0) confidence = 1.0;

    return confidence;
}

double SignalEngineLite::calculateExpectedMoveBps(const MarketSnapshot &snapshot,
                                                  const FlowSnapshot &flowSnapshot,
                                                  double recentMoveBps) const {
    const double imbalanceStrength = std::abs(snapshot.imbalance - 50.0);
    const double flowStrength = std::abs(flowSnapshot.aggressionBias) * 20.0;

    double expectedMove = (imbalanceStrength * 0.5) + (flowStrength * 0.8);

    // Penaliza se o mercado já andou
    expectedMove -= std::abs(recentMoveBps) * 0.5;

    if (expectedMove < 0.0) expectedMove = 0.0;

    return expectedMove;
}
