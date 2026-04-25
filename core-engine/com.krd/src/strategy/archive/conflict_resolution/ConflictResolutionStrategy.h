#pragma once

#include <unordered_map>
#include <string>

#include "../../config/Config.h"
#include "../common/ISignalStrategy.h"
#include "ConflictState.h"

class ConflictResolutionStrategy : public ISignalStrategy {
public:
    explicit ConflictResolutionStrategy(const Config& config);

    SignalResult evaluate(const StrategyContext& context) const override;

private:
    const Config& config_;

    mutable std::unordered_map<std::string, ConflictState> conflictStates_;

    std::string makeKey(const StrategyContext& context) const;

    double calculateConfidence(const StrategyContext& context) const;
    double calculateExpectedMoveBps(const StrategyContext& context) const;

    bool isBookLong(const StrategyContext& context) const;
    bool isBookShort(const StrategyContext& context) const;
    bool isFlowLong(const StrategyContext& context) const;
    bool isFlowShort(const StrategyContext& context) const;
    bool isFlowStrongEnough(const StrategyContext& context) const;
};