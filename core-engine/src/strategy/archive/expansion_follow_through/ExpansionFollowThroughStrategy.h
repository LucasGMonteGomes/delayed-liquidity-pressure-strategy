#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class ExpansionFollowThroughStrategy : public ISignalStrategy {
public:
    explicit ExpansionFollowThroughStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    bool isSpreadAcceptable(const StrategyContext& context) const;
    bool isRegimeExpandedEnough(const StrategyContext& context) const;

    bool isLongFlowConfirmed(const StrategyContext& context) const;
    bool isShortFlowConfirmed(const StrategyContext& context) const;

    bool isLongBookSupportive(const StrategyContext& context) const;
    bool isShortBookSupportive(const StrategyContext& context) const;

    bool isRecentMoveAlignedForLong(const StrategyContext& context) const;
    bool isRecentMoveAlignedForShort(const StrategyContext& context) const;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;
};