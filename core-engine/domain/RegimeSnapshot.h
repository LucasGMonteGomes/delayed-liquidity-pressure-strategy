#pragma once

#include <string>

struct RegimeSnapshot {
    double avgSpreadBps{0.0};

    double shortRangeBps{0.0};
    double activityBps{0.0};

    double imbalanceRange{0.0};
    bool hasRecentFlow{false};

    bool tradable{false};
    std::string reason{"uninitialized"};
    int sampleCount{0};
};