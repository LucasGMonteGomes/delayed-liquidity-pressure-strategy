#pragma once

#include "../config/Config.h"
#include "../domain/MarketSnapshot.h"
#include "../domain/SignalResult.h"

class ExecutionQualityFilter {
public:
    explicit ExecutionQualityFilter(const Config& config);

    bool shouldAllowEntry(const MarketSnapshot& snapshot,
                          const SignalResult& signal) const;

    double estimateTotalCostBps(const MarketSnapshot& snapshot) const;
    double estimateRequiredMoveBps(const MarketSnapshot& snapshot) const;

private:
    const Config& config_;
};