#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>

#include "../domain/MarketSnapshot.h"
#include "../domain/RegimeSnapshot.h"
#include "../domain/FlowSnapshot.h"

class RegimeFilter {
public:
    explicit RegimeFilter(std::int64_t windowMs);

    void onSnapshot(const MarketSnapshot& snapshot);

    RegimeSnapshot getSnapshot(const std::string& exchange,
                              const std::string& symbol,
                              std::int64_t nowMs,
                              const FlowSnapshot& flowSnapshot);

private:
    std::int64_t windowMs_;

    std::unordered_map<std::string, std::deque<MarketSnapshot>> snapshotsByKey_;

    std::string makeKey(const std::string& exchange, const std::string& symbol) const;

    void pruneOldSnapshots(std::deque<MarketSnapshot>& snapshots, std::int64_t nowMs) const;
};