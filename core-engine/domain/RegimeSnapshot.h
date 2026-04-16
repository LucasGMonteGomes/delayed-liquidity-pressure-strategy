#pragma once

struct RegimeSnapshot {
    double avgSpreadBps{0.0};
    double shortRangeBps{0.0};
    double activityBps{0.0};

    bool tradable{false};
};