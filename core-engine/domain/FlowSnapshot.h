#pragma once

struct FlowSnapshot {
    double buyAggressionQty{0.0};
    double sellAggressionQty{0.0};

    double netAggressionQty{0.0};     // buy - sell
    double totalAggressionQty{0.0};   // buy + sell

    double aggressionBias{0.0};       // -1.0 a +1.0
};