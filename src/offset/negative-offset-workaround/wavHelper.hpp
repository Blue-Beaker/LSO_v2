#pragma once

#include <Geode/Geode.hpp>

#include "CacheStorage.hpp"

using namespace geode::prelude;

// Decode source audio to PCM, prepend silence, write as WAV.
bool createPaddedWavFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath,
    int padMs
);
