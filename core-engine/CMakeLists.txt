cmake_minimum_required(VERSION 3.20)
project(core_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ixwebsocket CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)

add_executable(core_engine
        src/main.cpp
        src/strategy/signal_engine/SignalEngineLite.cpp
        src/execution/PaperTradeEngine.cpp
        src/messaging/TradePublisher.cpp

        src/domain/TradeTick.cpp
        src/domain/TradeTick.h
        src/domain/FlowSnapshot.cpp
        src/domain/FlowSnapshot.h
        src/domain/SignalCandidate.cpp
        src/domain/SignalCandidate.h
        src/domain/RegimeSnapshot.cpp
        src/domain/RegimeSnapshot.h

        src/flow/AggressionTracker.cpp
        src/flow/AggressionTracker.h

        src/persistence/SignalPersistenceFilter.cpp
        src/persistence/SignalPersistenceFilter.h

        src/regime/RegimeFilter.cpp
        src/regime/RegimeFilter.h

        src/strategy/common/ISignalStrategy.cpp
        src/strategy/common/ISignalStrategy.h
        src/strategy/common/StrategyContext.cpp
        src/strategy/common/StrategyContext.h

        src/execution/ExecutionQualityFilter.cpp
        src/execution/ExecutionQualityFilter.h
        src/execution/TradeStatsCollector.cpp
        src/execution/TradeStatsCollector.h
        src/execution/TradeCsvWriter.cpp
        src/execution/TradeCsvWriter.h
        src/execution/TestSummaryWriter.cpp
        src/execution/TestSummaryWriter.h
        src/execution/TestRunPaths.cpp
        src/execution/TestRunPaths.h

        src/strategy/intraday_trend_continuation/IntradayTrendContinuationStrategy.cpp
        src/strategy/intraday_trend_continuation/IntradayTrendContinuationStrategy.h
)

target_include_directories(core_engine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_engine PRIVATE
        ixwebsocket::ixwebsocket
        nlohmann_json::nlohmann_json
        cppzmq
        bcrypt
)