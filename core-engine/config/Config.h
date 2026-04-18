#pragma once

struct Config {
    // ---------- SINAL ----------
    double imbalanceLongThreshold{65.0};
    double imbalanceShortThreshold{35.0};

    double minConfidence{0.60};

    // spread máximo permitido em basis points
    double maxSpreadBps{8.0};

    // movimento recente máximo permitido em basis points
    // para evitar entrar quando o preço já andou demais
    double maxRecentMoveBps{5.0};

    // movimento esperado mínimo para validar o sinal
    double minExpectedMoveBps{10.0};

    // ---------- FLOW FILTER ----------
    // volume agressor mínimo acumulado na janela
    double minFlowStrength{0.05};

    // viés absoluto mínimo do fluxo (-1 a +1)
    double minFlowBiasAbs{0.35};

    // ---------- EXECUÇÃO PAPER ----------
    // alvo e stop em percentual
    double targetPct{0.02};   // 0.02%
    double stopPct{0.07};     // 0.07%

    // tempo máximo da posição aberta
    long timeoutMs{30000};    // 15 segundos

    // ---------- CUSTOS ----------
    // taxas e slippage simulados, em percentual
    double feePctPerSide{0.04};       // 0.04% por lado
    double slippagePctPerSide{0.01};  // 0.01% por lado
};