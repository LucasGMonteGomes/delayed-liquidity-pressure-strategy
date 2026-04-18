#pragma once

#include <cstdint>

enum class ConflictType {
    NONE,
    BOOK_LONG_FLOW_SHORT,
    BOOK_SHORT_FLOW_LONG
};

struct ConflictState {
    ConflictType type{ConflictType::NONE};
    bool active{false};
    std::int64_t detectedAtMs{0};
    double conflictStrength{0.0};
};