#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class CompressionBreakoutStrategy : public ISignalStrategy {
public:
    explicit CompressionBreakoutStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;
};