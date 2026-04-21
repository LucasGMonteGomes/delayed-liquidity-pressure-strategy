#include "TradeCsvWriter.h"

TradeCsvWriter::TradeCsvWriter(const std::string& filePath)
    : file_(filePath, std::ios::out | std::ios::trunc) {
    if (file_.is_open()) {
        writeHeader();
    }
}

TradeCsvWriter::~TradeCsvWriter() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

bool TradeCsvWriter::isOpen() const {
    return file_.is_open();
}

void TradeCsvWriter::writeTrade(const TradeResult& trade) {
    if (!file_.is_open()) {
        return;
    }

    file_
        << trade.entryTimestampMs << ","
        << trade.exitTimestampMs << ","
        << trade.exchange << ","
        << trade.symbol << ","
        << signalSideToString(trade.side) << ","
        << trade.entryPrice << ","
        << trade.exitPrice << ","
        << exitReasonToString(trade.exitReason) << ","
        << trade.grossPnlPct << ","
        << trade.feePct << ","
        << trade.slippagePct << ","
        << trade.netPnlPct << ","
        << trade.entryImbalance << ","
        << trade.entryConfidence
        << "\n";

    file_.flush();
}

void TradeCsvWriter::writeHeader() {
    file_
        << "entry_timestamp_ms,"
        << "exit_timestamp_ms,"
        << "exchange,"
        << "symbol,"
        << "side,"
        << "entry_price,"
        << "exit_price,"
        << "exit_reason,"
        << "gross_pnl_pct,"
        << "fee_pct,"
        << "slippage_pct,"
        << "net_pnl_pct,"
        << "entry_imbalance,"
        << "entry_confidence"
        << "\n";
}

std::string TradeCsvWriter::exitReasonToString(ExitReason reason) const {
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

std::string TradeCsvWriter::signalSideToString(SignalSide side) const {
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