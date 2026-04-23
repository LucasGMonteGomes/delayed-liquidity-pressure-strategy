#pragma once

#pragma once

struct Config {
    double imbalanceLongThreshold{68.0};
    double imbalanceShortThreshold{32.0};

    double minConfidence{0.70};

    double maxSpreadBps{6.0};
    double maxRecentMoveBps{80.0};
    double minExpectedMoveBps{45.0};

    double minFlowStrength{0.90};
    double minFlowBiasAbs{0.68};

    double targetPct{0.70};
    double stopPct{0.25};
    long timeoutMs{3600000};

    double feePctPerSide{0.01};
    double slippagePctPerSide{0.0025};
};
