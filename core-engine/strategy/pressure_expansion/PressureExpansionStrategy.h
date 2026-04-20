#pragma once

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"

class PressureExpansionStrategy : public ISignalStrategy {
public:
    explicit PressureExpansionStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;
    double calculateRegimeExpansionStrength(const StrategyContext& context) const;

    bool isBookLongSupportive(const StrategyContext& context) const;
    bool isBookShortSupportive(const StrategyContext& context) const;

    bool isFlowLongSupportive(const StrategyContext& context) const;
    bool isFlowShortSupportive(const StrategyContext& context) const;

    bool isFlowStrongEnough(const StrategyContext& context) const;
    bool isRegimeExpansionStrongEnough(const StrategyContext& context) const;
    bool isRecentMoveAcceptable(const StrategyContext& context) const;

    bool isFlowOverrideLong(const StrategyContext& context) const;
    bool isFlowOverrideShort(const StrategyContext& context) const;
};