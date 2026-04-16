#pragma once

#include <string>

#include <zmq.hpp>

#include "../domain/TradeResult.h"

class TradePublisher {
public:
    explicit TradePublisher(zmq::socket_t& publisher);

    void publish(const TradeResult& trade) const;

private:
    zmq::socket_t& publisher_;

    std::string buildPayload(const TradeResult& trade) const;
    std::string sideToString(SignalSide side) const;
    std::string exitReasonToString(ExitReason reason) const;
};