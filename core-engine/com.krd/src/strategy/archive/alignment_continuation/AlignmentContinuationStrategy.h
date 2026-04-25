#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class AlignmentContinuationStrategy : public ISignalStrategy {
public:
    explicit AlignmentContinuationStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;

    bool isBookLong(const StrategyContext& context) const;
    bool isBookShort(const StrategyContext& context) const;

    bool isFlowLong(const StrategyContext& context) const;
    bool isFlowShort(const StrategyContext& context) const;

    bool isFlowNeutralToLong(const StrategyContext& context) const;
    bool isFlowNeutralToShort(const StrategyContext& context) const;

    bool isFlowStrongEnough(const StrategyContext& context) const;
};