#pragma once

#include <string>

struct TestRunPaths {
    std::string runDirectory;
    std::string tradeResultsCsvPath;
    std::string testSummaryCsvPath;
};

class TestRunPathsBuilder {
public:
    static TestRunPaths build(long testDurationMs);
};