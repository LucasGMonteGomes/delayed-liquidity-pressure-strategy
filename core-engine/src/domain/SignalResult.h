#pragma once

#include <string>

enum class SignalSide {
    LONG,
    SHORT,
    HOLD
};

struct SignalResult {
    SignalSide side{SignalSide::HOLD};

    double confidence{0.0};       // força do sinal (0 a 1)
    double expectedMoveBps{0.0};  // movimento esperado em bps

    double flowBias{0.0};         // -1.0 a +1.0
    double flowStrength{0.0};     // volume agressor total da janela

    std::string reason;           // explicação do sinal (debug)

    bool isValid{false};          // se passou nos filtros mínimos
};