#include "TradeStatsCollector.h"

void TradeStatsCollector::onTradeClosed(const TradeResult& trade) {
    totalTrades_++;

    if (trade.exitReason == ExitReason::TAKE_PROFIT) {
        takeProfitCount_++;
    } else if (trade.exitReason == ExitReason::STOP_LOSS) {
        stopLossCount_++;
    } else if (trade.exitReason == ExitReason::TIMEOUT) {
        timeoutCount_++;
    }

    grossPnlSumPct_ += trade.grossPnlPct;
    netPnlSumPct_ += trade.netPnlPct;

    if (trade.netPnlPct >= 0.0) {
        winCount_++;
    } else {
        lossCount_++;
    }

    const std::string key = makeKey(trade.exchange, trade.symbol);
    SymbolStats& stats = statsBySymbolKey_[key];

    stats.totalTrades++;

    if (trade.exitReason == ExitReason::TAKE_PROFIT) {
        stats.takeProfitCount++;
    } else if (trade.exitReason == ExitReason::STOP_LOSS) {
        stats.stopLossCount++;
    } else if (trade.exitReason == ExitReason::TIMEOUT) {
        stats.timeoutCount++;
    }

    stats.grossPnlSumPct += trade.grossPnlPct;
    stats.netPnlSumPct += trade.netPnlPct;
}

int TradeStatsCollector::getTotalTrades() const {
    return totalTrades_;
}

int TradeStatsCollector::getTakeProfitCount() const {
    return takeProfitCount_;
}

int TradeStatsCollector::getStopLossCount() const {
    return stopLossCount_;
}

int TradeStatsCollector::getTimeoutCount() const {
    return timeoutCount_;
}

int TradeStatsCollector::getWinCount() const {
    return winCount_;
}

int TradeStatsCollector::getLossCount() const {
    return lossCount_;
}

double TradeStatsCollector::getGrossPnlSumPct() const {
    return grossPnlSumPct_;
}

double TradeStatsCollector::getNetPnlSumPct() const {
    return netPnlSumPct_;
}

double TradeStatsCollector::getWinRatePct() const {
    if (totalTrades_ <= 0) {
        return 0.0;
    }

    return (static_cast<double>(winCount_) / static_cast<double>(totalTrades_)) * 100.0;
}

double TradeStatsCollector::getAverageNetPnlPct() const {
    if (totalTrades_ <= 0) {
        return 0.0;
    }

    return netPnlSumPct_ / static_cast<double>(totalTrades_);
}

const std::unordered_map<std::string, SymbolStats>& TradeStatsCollector::getStatsBySymbolKey() const {
    return statsBySymbolKey_;
}

std::string TradeStatsCollector::makeKey(const std::string& exchange, const std::string& symbol) const {
    return exchange + "|" + symbol;
}