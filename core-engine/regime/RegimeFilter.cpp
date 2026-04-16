#include "RegimeFilter.h"

#include <algorithm>
#include <cmath>

RegimeFilter::RegimeFilter(std::int64_t windowMs)
    : windowMs_(windowMs) {}

void RegimeFilter::onSnapshot(const MarketSnapshot& snapshot) {
    const std::string key = makeKey(snapshot.exchange, snapshot.symbol);
    auto& snapshots = snapshotsByKey_[key];
    snapshots.push_back(snapshot);
}

RegimeSnapshot RegimeFilter::getSnapshot(const std::string& exchange,
                                         const std::string& symbol,
                                         std::int64_t nowMs) {
    RegimeSnapshot result;

    const std::string key = makeKey(exchange, symbol);
    auto it = snapshotsByKey_.find(key);
    if (it == snapshotsByKey_.end()) {
        return result;
    }

    auto& snapshots = it->second;
    pruneOldSnapshots(snapshots, nowMs);

    if (snapshots.empty()) {
        return result;
    }

    double spreadSum = 0.0;
    double minMid = snapshots.front().midPrice;
    double maxMid = snapshots.front().midPrice;

    double activitySum = 0.0;
    double previousMid = snapshots.front().midPrice;

    for (std::size_t i = 0; i < snapshots.size(); ++i) {
        const auto& s = snapshots[i];

        spreadSum += s.spreadBps;
        minMid = std::min(minMid, s.midPrice);
        maxMid = std::max(maxMid, s.midPrice);

        if (i > 0 && previousMid > 0.0) {
            activitySum += std::abs((s.midPrice - previousMid) / previousMid) * 10000.0;
        }

        previousMid = s.midPrice;
    }

    result.avgSpreadBps = spreadSum / static_cast<double>(snapshots.size());

    if (minMid > 0.0) {
        result.shortRangeBps = ((maxMid - minMid) / minMid) * 10000.0;
    }

    result.activityBps = activitySum;

    // Regras mínimas de regime para a v3-lite
    const bool spreadOk = result.avgSpreadBps <= 1.5;
    const bool rangeOk = result.shortRangeBps >= 0.5;
    const bool activityOk = result.activityBps >= 0.5;

    result.tradable = spreadOk && rangeOk && activityOk;

    return result;
}

std::string RegimeFilter::makeKey(const std::string& exchange, const std::string& symbol) const {
    return exchange + "|" + symbol;
}

void RegimeFilter::pruneOldSnapshots(std::deque<MarketSnapshot>& snapshots, std::int64_t nowMs) const {
    while (!snapshots.empty()) {
        const auto& front = snapshots.front();
        if ((nowMs - front.timestampMs) > windowMs_) {
            snapshots.pop_front();
        } else {
            break;
        }
    }
}