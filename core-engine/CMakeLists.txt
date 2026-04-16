cmake_minimum_required(VERSION 3.20)
project(core_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ixwebsocket CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)

add_executable(core_engine
        main.cpp
        strategy/SignalEngineLite.cpp
        execution/PaperTradeEngine.cpp
        messaging/TradePublisher.cpp
        domain/TradeTick.cpp
        domain/TradeTick.h
        domain/FlowSnapshot.cpp
        domain/FlowSnapshot.h
        flow/AggressionTracker.cpp
        flow/AggressionTracker.h
        domain/SignalCandidate.cpp
        domain/SignalCandidate.h
        persistence/SignalPersistenceFilter.cpp
        persistence/SignalPersistenceFilter.h
        domain/RegimeSnapshot.cpp
        domain/RegimeSnapshot.h
        regime/RegimeFilter.cpp
        regime/RegimeFilter.h
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