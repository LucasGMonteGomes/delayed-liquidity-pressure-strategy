#pragma once

#include <cstdint>
#include <string>

struct MarketSnapshot {
    std::string exchange;
    std::string symbol;

    double bidPrice{0.0};
    double askPrice{0.0};

    double bidQty{0.0};
    double askQty{0.0};

    double midPrice{0.0};
    double spread{0.0};
    double spreadBps{0.0};

    double imbalance{50.0};

    std::int64_t timestampMs{0};
};