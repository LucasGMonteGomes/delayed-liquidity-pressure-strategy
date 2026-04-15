cmake_minimum_required(VERSION 3.17)
project(core_engine)

set(CMAKE_CXX_STANDARD 17)

# 1. Encontra os pacotes instalados pelo vcpkg (Agora com ZeroMQ)
find_package(ixwebsocket CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)

# 2. Cria o executável
add_executable(core_engine main.cpp)

# 3. Linka as bibliotecas principais
target_link_libraries(core_engine PRIVATE ixwebsocket::ixwebsocket nlohmann_json::nlohmann_json cppzmq)

# 4. Força o Windows a usar o gerador nativo de criptografia (BCrypt)
if (WIN32)
    target_link_libraries(core_engine PRIVATE bcrypt)
endif()