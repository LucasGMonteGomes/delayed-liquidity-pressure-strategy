#pragma once

#include <cstdint>
#include <string>

#include "SignalResult.h"
#include "Position.h"

struct TradeResult {
    std::string exchange;
    std::string symbol;

    SignalSide side{SignalSide::HOLD};

    double entryPrice{0.0};
    double exitPrice{0.0};

    std::int64_t entryTimestampMs{0};
    std::int64_t exitTimestampMs{0};

    ExitReason exitReason{ExitReason::NONE};

    double grossPnlPct{0.0};   // PnL bruto (%)
    double feePct{0.0};        // custo total de taxas (%)
    double slippagePct{0.0};   // custo de slippage (%)
    double netPnlPct{0.0};     // PnL líquido (%)

    double entryImbalance{0.0};
    double entryConfidence{0.0};
};