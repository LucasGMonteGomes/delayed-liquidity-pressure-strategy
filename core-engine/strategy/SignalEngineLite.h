#pragma once

#include "../config/Config.h"
#include "../domain/MarketSnapshot.h"
#include "../domain/SignalResult.h"
#include "../domain/FlowSnapshot.h"

class SignalEngineLite {
public:
    explicit SignalEngineLite(const Config& config);

    SignalResult evaluate(const MarketSnapshot& snapshot,
                          const FlowSnapshot& flowSnapshot,
                          double recentMoveBps) const;

private:
    const Config& config_;

    double calculateConfidence(const MarketSnapshot& snapshot,
                               const FlowSnapshot& flowSnapshot,
                               double recentMoveBps) const;

    double calculateExpectedMoveBps(const MarketSnapshot& snapshot,
                                    const FlowSnapshot& flowSnapshot,
                                    double recentMoveBps) const;
};