#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class BreakHoldContinuationStrategy : public ISignalStrategy {
public:
    explicit BreakHoldContinuationStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    bool isSpreadAcceptable(const StrategyContext& context) const;
    bool isRegimeSupportive(const StrategyContext& context) const;

    bool isLongFlowAligned(const StrategyContext& context) const;
    bool isShortFlowAligned(const StrategyContext& context) const;

    bool isLongBookSupportive(const StrategyContext& context) const;
    bool isShortBookSupportive(const StrategyContext& context) const;

    bool isLongMoveStructured(const StrategyContext& context) const;
    bool isShortMoveStructured(const StrategyContext& context) const;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;
};