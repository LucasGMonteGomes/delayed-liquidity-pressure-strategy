#pragma once

#include <string>
#include <cstdint>
#include "TradeStatsCollector.h"

class TestSummaryWriter {
public:
    static bool writeSummary(const std::string& filePath,
                         const TradeStatsCollector& stats,
                         long testDurationMs,
                         std::int64_t testStartMs,
                         std::int64_t testEndMs);
};