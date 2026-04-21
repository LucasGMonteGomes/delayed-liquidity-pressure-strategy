#pragma once

#include "../../domain/SignalResult.h"
#include "StrategyContext.h"

class ISignalStrategy {
public:
    virtual ~ISignalStrategy() = default;

    virtual SignalResult evaluate(const StrategyContext& context) const = 0;
};