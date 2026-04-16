#pragma once

#include <cstdint>
#include <string>

enum class AggressorSide {
    BUY,
    SELL,
    UNKNOWN
};

struct TradeTick {
    std::string exchange;
    std::string symbol;

    double price{0.0};
    double qty{0.0};

    AggressorSide aggressorSide{AggressorSide::UNKNOWN};

    std::int64_t timestampMs{0};
};