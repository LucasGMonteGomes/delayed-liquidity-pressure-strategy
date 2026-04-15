#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

std::mutex zmq_mutex;

// Nova função analítica que processa o volume e calcula o Imbalance
void broadcast_data(zmq::socket_t& publisher, const std::string& exchange, const std::string& symbol,
                    const std::string& bid, const std::string& ask,
                    double bid_qty, double ask_qty) {

    // 1. O Cérebro Quant: Cálculo de Pressão de Liquidez
    double total_qty = bid_qty + ask_qty;
    double imbalance = (total_qty > 0) ? (bid_qty / total_qty) * 100.0 : 50.0;

    // 2. Alerta Visual para o Terminal
    std::string alert = "[NEUTRO]";
    if (imbalance >= 70.0) alert = "[🚀 COMPRA FORTE]";
    else if (imbalance <= 30.0) alert = "[🩸 VENDA FORTE]";

    // 3. Formatação cravada em 2 casas decimais para o Imbalance
    std::ostringstream imb_stream;
    imb_stream << std::fixed << std::setprecision(2) << imbalance;

    // 4. Novo Protocolo: CORRETORA|MOEDA|COMPRA|VENDA|IMBALANCE
    std::string payload = exchange + "|" + symbol + "|" + bid + "|" + ask + "|" + imb_stream.str();

    std::lock_guard<std::mutex> lock(zmq_mutex);
    zmq::message_t message(payload.size());
    memcpy(message.data(), payload.c_str(), payload.size());
    publisher.send(message, zmq::send_flags::none);

    std::cout << "[MOTOR] -> " << payload << " \t" << alert << std::endl;
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t publisher(context, zmq::socket_type::pub);
    publisher.bind("tcp://*:5555");

    ix::initNetSystem();

    // ==========================================
    // 1. TURBINA BINANCE
    // ==========================================
    ix::WebSocket binance_ws;
    binance_ws.setUrl("wss://stream.binance.com:9443/stream?streams=btcusdt@bookTicker/ethusdt@bookTicker");

    binance_ws.setOnMessageCallback([&publisher](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "[REDE] Binance Conectada. (Monitorando Pressao BTC e ETH)" << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Message) {
            try {
                auto j = json::parse(msg->str);
                if (j.contains("data")) {
                    std::string symbol = j["data"]["s"];
                    std::string bidPrice = j["data"]["b"];
                    std::string askPrice = j["data"]["a"];

                    // Extrai o volume e converte para decimal
                    double bidQty = std::stod(std::string(j["data"]["B"]));
                    double askQty = std::stod(std::string(j["data"]["A"]));

                    broadcast_data(publisher, "BINANCE", symbol, bidPrice, askPrice, bidQty, askQty);
                }
            } catch (...) {}
        }
    });

    // ==========================================
    // 2. TURBINA BYBIT
    // ==========================================
    ix::WebSocket bybit_ws;
    bybit_ws.setUrl("wss://stream.bybit.com/v5/public/spot");

    bybit_ws.setOnMessageCallback([&publisher, &bybit_ws](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "[REDE] Bybit Conectada. Solicitando dados de volume..." << std::endl;
            std::string sub_msg = "{\"op\": \"subscribe\", \"args\": [\"orderbook.1.BTCUSDT\", \"orderbook.1.ETHUSDT\"]}";
            bybit_ws.sendText(sub_msg);
        }
        else if (msg->type == ix::WebSocketMessageType::Message) {
            try {
                auto j = json::parse(msg->str);
                if (j.contains("data") && j["data"].contains("b") && j["data"].contains("a")) {
                    if (!j["data"]["b"].empty() && !j["data"]["a"].empty()) {
                        std::string symbol = j["data"]["s"];
                        std::string bidPrice = j["data"]["b"][0][0];
                        std::string askPrice = j["data"]["a"][0][0];

                        // Extrai o volume da segunda posição da matriz [preço, volume]
                        double bidQty = std::stod(std::string(j["data"]["b"][0][1]));
                        double askQty = std::stod(std::string(j["data"]["a"][0][1]));

                        broadcast_data(publisher, "BYBIT", symbol, bidPrice, askPrice, bidQty, askQty);
                    }
                }
            } catch (...) {}
        }
    });

    // ==========================================
    // IGNIÇÃO
    // ==========================================
    std::cout << "[SYSTEM] Analisador Quantitativo Iniciado..." << std::endl;
    binance_ws.start();
    bybit_ws.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ix::uninitNetSystem();
    return 0;
}