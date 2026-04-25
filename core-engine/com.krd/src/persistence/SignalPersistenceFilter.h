#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../domain/SignalCandidate.h"
#include "../domain/SignalResult.h"
#include "../domain/MarketSnapshot.h"

class SignalPersistenceFilter {
public:
    SignalPersistenceFilter(std::int64_t minPersistenceMs, int minConfirmations);

    bool shouldAllowEntry(const MarketSnapshot& snapshot,
                          const SignalResult& signal);

private:
    std::int64_t minPersistenceMs_;
    int minConfirmations_;

    std::unordered_map<std::string, SignalCandidate> candidatesByKey_;

    std::string makeKey(const std::string& exchange, const std::string& symbol) const;

    void resetCandidate(const std::string& key);
};