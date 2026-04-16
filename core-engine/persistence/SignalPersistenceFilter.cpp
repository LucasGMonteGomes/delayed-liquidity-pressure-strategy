#include "SignalPersistenceFilter.h"

SignalPersistenceFilter::SignalPersistenceFilter(std::int64_t minPersistenceMs, int minConfirmations)
    : minPersistenceMs_(minPersistenceMs),
      minConfirmations_(minConfirmations) {}

bool SignalPersistenceFilter::shouldAllowEntry(const MarketSnapshot& snapshot,
                                               const SignalResult& signal) {
    const std::string key = makeKey(snapshot.exchange, snapshot.symbol);

    // Se o sinal não for válido ou HOLD, reseta o candidato
    if (!signal.isValid || signal.side == SignalSide::HOLD) {
        resetCandidate(key);
        return false;
    }

    auto it = candidatesByKey_.find(key);

    // Primeiro sinal válido desse ativo/corretora
    if (it == candidatesByKey_.end() || !it->second.active) {
        SignalCandidate candidate;
        candidate.exchange = snapshot.exchange;
        candidate.symbol = snapshot.symbol;
        candidate.side = signal.side;
        candidate.firstSeenTimestampMs = snapshot.timestampMs;
        candidate.lastSeenTimestampMs = snapshot.timestampMs;
        candidate.confirmations = 1;
        candidate.active = true;

        candidatesByKey_[key] = candidate;
        return false;
    }

    SignalCandidate& candidate = it->second;

    // Se mudou o lado, reinicia do zero
    if (candidate.side != signal.side) {
        candidate.exchange = snapshot.exchange;
        candidate.symbol = snapshot.symbol;
        candidate.side = signal.side;
        candidate.firstSeenTimestampMs = snapshot.timestampMs;
        candidate.lastSeenTimestampMs = snapshot.timestampMs;
        candidate.confirmations = 1;
        candidate.active = true;
        return false;
    }

    // Mesmo lado continua: acumula persistência
    candidate.lastSeenTimestampMs = snapshot.timestampMs;
    candidate.confirmations++;

    const std::int64_t persistenceMs =
        candidate.lastSeenTimestampMs - candidate.firstSeenTimestampMs;

    if (persistenceMs >= minPersistenceMs_ &&
        candidate.confirmations >= minConfirmations_) {
        resetCandidate(key);
        return true;
    }

    return false;
}

std::string SignalPersistenceFilter::makeKey(const std::string& exchange,
                                             const std::string& symbol) const {
    return exchange + "|" + symbol;
}

void SignalPersistenceFilter::resetCandidate(const std::string& key) {
    candidatesByKey_.erase(key);
}