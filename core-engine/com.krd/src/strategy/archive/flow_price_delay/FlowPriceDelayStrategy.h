#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class FlowPriceDelayStrategy : public ISignalStrategy {
public:
    explicit FlowPriceDelayStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;

    bool isFlowLongStrong(const StrategyContext& context) const;
    bool isFlowShortStrong(const StrategyContext& context) const;

    bool isBookSupportiveForLong(const StrategyContext& context) const;
    bool isBookSupportiveForShort(const StrategyContext& context) const;

    bool isRecentMoveSmallEnough(const StrategyContext& context) const;
};