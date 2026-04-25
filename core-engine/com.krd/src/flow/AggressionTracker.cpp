#include "AggressionTracker.h"

#include <cmath>

AggressionTracker::AggressionTracker(std::int64_t windowMs)
    : windowMs_(windowMs) {}

void AggressionTracker::onTrade(const TradeTick& trade) {
    const std::string key = makeKey(trade.exchange, trade.symbol);
    auto& trades = tradesByKey_[key];
    trades.push_back(trade);
}

FlowSnapshot AggressionTracker::getSnapshot(const std::string& exchange,
                                            const std::string& symbol,
                                            std::int64_t nowMs) {
    FlowSnapshot snapshot;

    const std::string key = makeKey(exchange, symbol);
    auto it = tradesByKey_.find(key);
    if (it == tradesByKey_.end()) {
        return snapshot;
    }

    auto& trades = it->second;
    pruneOldTrades(trades, nowMs);

    double buyQty = 0.0;
    double sellQty = 0.0;

    for (const auto& trade : trades) {
        if (trade.aggressorSide == AggressorSide::BUY) {
            buyQty += trade.qty;
        } else if (trade.aggressorSide == AggressorSide::SELL) {
            sellQty += trade.qty;
        }
    }

    snapshot.buyAggressionQty = buyQty;
    snapshot.sellAggressionQty = sellQty;
    snapshot.netAggressionQty = buyQty - sellQty;
    snapshot.totalAggressionQty = buyQty + sellQty;

    if (snapshot.totalAggressionQty > 0.0) {
        snapshot.aggressionBias = snapshot.netAggressionQty / snapshot.totalAggressionQty;
    } else {
        snapshot.aggressionBias = 0.0;
    }

    return snapshot;
}

std::string AggressionTracker::makeKey(const std::string& exchange, const std::string& symbol) const {
    return exchange + "|" + symbol;
}

void AggressionTracker::pruneOldTrades(std::deque<TradeTick>& trades, std::int64_t nowMs) const {
    while (!trades.empty()) {
        const auto& front = trades.front();
        if ((nowMs - front.timestampMs) > windowMs_) {
            trades.pop_front();
        } else {
            break;
        }
    }
}