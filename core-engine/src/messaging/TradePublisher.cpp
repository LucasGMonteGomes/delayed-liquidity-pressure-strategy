#include "TradePublisher.h"

#include <cstring>
#include <sstream>
#include <iomanip>

TradePublisher::TradePublisher(zmq::socket_t& publisher)
    : publisher_(publisher) {}

void TradePublisher::publish(const TradeResult& trade) const {
    std::string payload = buildPayload(trade);

    zmq::message_t message(payload.size());
    std::memcpy(message.data(), payload.c_str(), payload.size());

    publisher_.send(message, zmq::send_flags::none);
}

std::string TradePublisher::buildPayload(const TradeResult& trade) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);

    oss << "TRADE"
        << "|" << trade.exchange
        << "|" << trade.symbol
        << "|" << sideToString(trade.side)
        << "|" << trade.entryPrice
        << "|" << trade.exitPrice
        << "|" << exitReasonToString(trade.exitReason)
        << "|" << trade.grossPnlPct
        << "|" << trade.feePct
        << "|" << trade.slippagePct
        << "|" << trade.netPnlPct
        << "|" << trade.entryImbalance
        << "|" << trade.entryConfidence
        << "|" << trade.entryTimestampMs
        << "|" << trade.exitTimestampMs;

    return oss.str();
}

std::string TradePublisher::sideToString(SignalSide side) const {
    switch (side) {
        case SignalSide::LONG:
            return "LONG";
        case SignalSide::SHORT:
            return "SHORT";
        case SignalSide::HOLD:
        default:
            return "HOLD";
    }
}

std::string TradePublisher::exitReasonToString(ExitReason reason) const {
    switch (reason) {
        case ExitReason::TAKE_PROFIT:
            return "TAKE_PROFIT";
        case ExitReason::STOP_LOSS:
            return "STOP_LOSS";
        case ExitReason::TIMEOUT:
            return "TIMEOUT";
        case ExitReason::INVALIDATION:
            return "INVALIDATION";
        case ExitReason::NONE:
        default:
            return "NONE";
    }
}