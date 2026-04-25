#pragma once

#include <fstream>
#include <string>

#include "../domain/TradeResult.h"

class TradeCsvWriter {
public:
    explicit TradeCsvWriter(const std::string& filePath);
    ~TradeCsvWriter();

    void writeTrade(const TradeResult& trade);
    bool isOpen() const;

private:
    std::ofstream file_;

    void writeHeader();
    std::string exitReasonToString(ExitReason reason) const;
    std::string signalSideToString(SignalSide side) const;
};