#pragma once

#include <cstdint>
#include <string>

#include "SignalResult.h"

struct SignalCandidate {
    std::string exchange;
    std::string symbol;

    SignalSide side{SignalSide::HOLD};

    std::int64_t firstSeenTimestampMs{0};
    std::int64_t lastSeenTimestampMs{0};

    int confirmations{0};

    bool active{false};
};