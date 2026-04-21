#pragma once

#include "../../domain/MarketSnapshot.h"
#include "../../domain/FlowSnapshot.h"
#include "../../domain/RegimeSnapshot.h"

struct StrategyContext {
    MarketSnapshot marketSnapshot;
    FlowSnapshot flowSnapshot;
    RegimeSnapshot regimeSnapshot;
    double recentMoveBps{0.0};
};