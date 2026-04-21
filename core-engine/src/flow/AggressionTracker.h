#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>

#include "../domain/TradeTick.h"
#include "../domain/FlowSnapshot.h"

class AggressionTracker {
public:
    explicit AggressionTracker(std::int64_t windowMs);

    void onTrade(const TradeTick& trade);

    FlowSnapshot getSnapshot(const std::string& exchange, const std::string& symbol, std::int64_t nowMs);

private:
    std::int64_t windowMs_;

    std::unordered_map<std::string, std::deque<TradeTick>> tradesByKey_;

    std::string makeKey(const std::string& exchange, const std::string& symbol) const;

    void pruneOldTrades(std::deque<TradeTick>& trades, std::int64_t nowMs) const;
};