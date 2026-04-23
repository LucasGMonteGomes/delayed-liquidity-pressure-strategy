#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class RegimeHoldContinuationStrategy : public ISignalStrategy {
public:
    explicit RegimeHoldContinuationStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    bool isSpreadAcceptable(const StrategyContext& context) const;
    bool isRegimeStrongEnough(const StrategyContext& context) const;

    bool isLongFlowAligned(const StrategyContext& context) const;
    bool isShortFlowAligned(const StrategyContext& context) const;

    bool isLongBookAligned(const StrategyContext& context) const;
    bool isShortBookAligned(const StrategyContext& context) const;

    bool isLongMoveConfirmed(const StrategyContext& context) const;
    bool isShortMoveConfirmed(const StrategyContext& context) const;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;
};