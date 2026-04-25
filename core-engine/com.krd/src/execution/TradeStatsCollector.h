#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../domain/TradeResult.h"

struct SymbolStats {
    int totalTrades{0};
    int takeProfitCount{0};
    int stopLossCount{0};
    int timeoutCount{0};

    double grossPnlSumPct{0.0};
    double netPnlSumPct{0.0};
};

class TradeStatsCollector {
public:
    void onTradeClosed(const TradeResult& trade);

    int getTotalTrades() const;
    int getTakeProfitCount() const;
    int getStopLossCount() const;
    int getTimeoutCount() const;

    int getWinCount() const;
    int getLossCount() const;

    double getGrossPnlSumPct() const;
    double getNetPnlSumPct() const;

    double getWinRatePct() const;
    double getAverageNetPnlPct() const;

    const std::unordered_map<std::string, SymbolStats>& getStatsBySymbolKey() const;

private:
    int totalTrades_{0};
    int takeProfitCount_{0};
    int stopLossCount_{0};
    int timeoutCount_{0};

    int winCount_{0};
    int lossCount_{0};

    double grossPnlSumPct_{0.0};
    double netPnlSumPct_{0.0};

    std::unordered_map<std::string, SymbolStats> statsBySymbolKey_;

    std::string makeKey(const std::string& exchange, const std::string& symbol) const;
};