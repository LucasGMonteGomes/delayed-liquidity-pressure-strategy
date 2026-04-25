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
                                         std::int64_t nowMs,
                                         const FlowSnapshot& flowSnapshot) {
    RegimeSnapshot result;

    const std::string key = makeKey(exchange, symbol);
    auto it = snapshotsByKey_.find(key);
    if (it == snapshotsByKey_.end()) {
        result.reason = "window not initialized";
        return result;
    }

    auto& snapshots = it->second;
    pruneOldSnapshots(snapshots, nowMs);

    result.sampleCount = static_cast<int>(snapshots.size());

    if (snapshots.empty()) {
        result.reason = "window empty after prune";
        return result;
    }

    double spreadSum = 0.0;

    double minMid = snapshots.front().midPrice;
    double maxMid = snapshots.front().midPrice;

    double activitySum = 0.0;
    double previousMid = snapshots.front().midPrice;

    double minImbalance = snapshots.front().imbalance;
    double maxImbalance = snapshots.front().imbalance;

    for (std::size_t i = 0; i < snapshots.size(); ++i) {
        const auto& s = snapshots[i];

        spreadSum += s.spreadBps;

        minMid = std::min(minMid, s.midPrice);
        maxMid = std::max(maxMid, s.midPrice);

        minImbalance = std::min(minImbalance, s.imbalance);
        maxImbalance = std::max(maxImbalance, s.imbalance);

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
    result.imbalanceRange = maxImbalance - minImbalance;
    result.hasRecentFlow = flowSnapshot.totalAggressionQty >= 0.05;

    const bool enoughSamples = result.sampleCount >= 3;
    const bool spreadOk = result.avgSpreadBps <= 1.5;
    const bool imbalanceAlive = result.imbalanceRange >= 8.0;
    const bool flowAlive = result.hasRecentFlow;

    result.tradable = enoughSamples && spreadOk && (imbalanceAlive || flowAlive);

    if (result.tradable) {
        result.reason = "tradable";
        return result;
    }

    if (!enoughSamples) {
        result.reason = "not enough samples";
    } else if (!spreadOk) {
        result.reason = "spread too high";
    } else if (!imbalanceAlive && !flowAlive) {
        result.reason = "imbalance and flow too weak";
    } else if (!imbalanceAlive) {
        result.reason = "imbalance too static";
    } else if (!flowAlive) {
        result.reason = "flow too weak";
    } else {
        result.reason = "unknown regime rejection";
    }

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