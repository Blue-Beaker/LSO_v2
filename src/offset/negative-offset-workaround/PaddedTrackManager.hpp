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
        static PaddedResult getPaddedResult(gd::string filePath);
};