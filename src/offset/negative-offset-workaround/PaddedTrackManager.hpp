#pragma once

#include "../../utils/Utils.hpp"

struct PaddedResult
{
    bool isPadded = false;
    gd::string resultingPath;
};

class PaddedTrackManager{
    public:
        static PaddedTrackManager get();
        PaddedResult getPaddedResult(int totalOffset, gd::string filePath);
        PaddedResult getPaddedResult(int songKey, int totalOffset, gd::string filePath);
};