#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class PersistenceContinuationStrategy : public ISignalStrategy {
public:
    explicit PersistenceContinuationStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    bool isSpreadAcceptable(const StrategyContext& context) const;
    bool isRegimeSupportive(const StrategyContext& context) const;

    bool isLongFlowPersistent(const StrategyContext& context) const;
    bool isShortFlowPersistent(const StrategyContext& context) const;

    bool isLongBookSupportive(const StrategyContext& context) const;
    bool isShortBookSupportive(const StrategyContext& context) const;

    bool isLongMovePersistent(const StrategyContext& context) const;
    bool isShortMovePersistent(const StrategyContext& context) const;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;
};