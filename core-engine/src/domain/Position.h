#pragma once

#include <cstdint>
#include <string>

#include "SignalResult.h"

enum class ExitReason {
    NONE,
    TAKE_PROFIT,
    STOP_LOSS,
    TIMEOUT,
    INVALIDATION
};

struct Position {
    bool isOpen{false};

    std::string exchange;
    std::string symbol;

    SignalSide side{SignalSide::HOLD};

    double entryPrice{0.0};
    double targetPrice{0.0};
    double stopPrice{0.0};

    std::int64_t entryTimestampMs{0};
    std::int64_t timeoutTimestampMs{0};

    double entryImbalance{50.0};
    double entryConfidence{0.0};
    double expectedMoveBps{0.0};

    ExitReason exitReason{ExitReason::NONE};
};