#include "TestSummaryWriter.h"

#include <fstream>
#include <iomanip>

bool TestSummaryWriter::writeSummary(const std::string& filePath,
                                     const TradeStatsCollector& stats,
                                     long testDurationMs,
                                     std::int64_t testStartMs,
                                     std::int64_t testEndMs) {
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    file << "metric,value\n";
    file << "test_start_timestamp_ms," << testStartMs << "\n";
    file << "test_end_timestamp_ms," << testEndMs << "\n";
    file << "test_duration_ms," << testDurationMs << "\n";
    file << "test_duration_minutes," << (testDurationMs / (60L * 1000L)) << "\n";
    file << "total_trades," << stats.getTotalTrades() << "\n";
    file << "take_profit_count," << stats.getTakeProfitCount() << "\n";
    file << "stop_loss_count," << stats.getStopLossCount() << "\n";
    file << "timeout_count," << stats.getTimeoutCount() << "\n";
    file << "win_count," << stats.getWinCount() << "\n";
    file << "loss_count," << stats.getLossCount() << "\n";

    file << std::fixed << std::setprecision(6);
    file << "gross_pnl_sum_pct," << stats.getGrossPnlSumPct() << "\n";
    file << "net_pnl_sum_pct," << stats.getNetPnlSumPct() << "\n";
    file << "win_rate_pct," << stats.getWinRatePct() << "\n";
    file << "average_net_pnl_pct," << stats.getAverageNetPnlPct() << "\n";

    file << "\n";
    file << "symbol_key,total_trades,take_profit_count,stop_loss_count,timeout_count,gross_pnl_sum_pct,net_pnl_sum_pct\n";

    for (const auto& [key, symbolStats] : stats.getStatsBySymbolKey()) {
        file
            << key << ","
            << symbolStats.totalTrades << ","
            << symbolStats.takeProfitCount << ","
            << symbolStats.stopLossCount << ","
            << symbolStats.timeoutCount << ","
            << symbolStats.grossPnlSumPct << ","
            << symbolStats.netPnlSumPct
            << "\n";
    }

    file.flush();
    file.close();
    return true;
}