#pragma once

#include <Geode/Geode.hpp>

#include <filesystem>
#include <functional>
#include <vector>

using namespace geode::prelude;

// Padded audio file cache manager for negative offset workaround. Handles file naming, cache size limit, and cleanup.

// Compute a hash for a source audio file path.
// Used to distinguish nong songs that share the same GD song ID.
// Returns 0 for original GD songs (numeric filename) so they use
// the old songKey-only naming scheme.
unsigned long hashSourcePath(const gd::string sourcePath);

// Get the song key for a GJGameLevel's current song.
int getSongKey(GJGameLevel* level);

// @geode-ignore(unknown-resource)
// Try to extract a numeric song ID from a path like "123456.mp3".
// Returns -1 if no numeric ID found.
int extractSongIdFromPath(gd::string path);

// Resolve the cache directory from settings or fall back to the mod save dir.
// Under Wine, auto-maps Linux-style paths to Z:\ drive.
std::filesystem::path getCacheDir();

// Compute the padded WAV path for a given song key, source path, and offset.
// The source path is hashed so different nongs get different cache files.
// e.g. getPaddedPath(837148, -1500, "C:/song.mp3")
//   -> /cache/padded_837148_1a2b3c4d_2000.wav
std::filesystem::path getPaddedPath(int songKey, int totalOffset, const gd::string sourcePath);

// Enforce the max cache size: delete oldest padded files when exceeded.
// Only the padded files for the given song keys (resolved via
// MusicDownloadManager to find the original/active paths) are excluded.
// @param songKeys   List of active song keys for the current level
// @param totalOffset The total offset used to compute padded filenames
struct FileEntry {
    std::filesystem::path path;
    std::filesystem::file_time_type time;
    uintmax_t size;
};

// Result of scanning the cache directory for removable padded files.
struct CacheCollection {
    std::vector<FileEntry> removable;
    uintmax_t totalSize = 0;
    int totalFiles = 0;
    int excludedCount = 0;
};

// Scan the cache directory and collect removable padded WAV files.
// Files in excludedFiles are skipped.
CacheCollection collectRemovableCacheFiles(const std::unordered_set<std::filesystem::path>& excludedFiles);

// Delete files from collection (sorted oldest-first) until target bytes are freed.
// Returns the number of bytes actually freed.
uintmax_t deleteOldestFiles(std::vector<FileEntry>& files, uintmax_t target);

// Convenience function: collect all removable cache, prompt the user with a
// confirmation popup, and delete everything if confirmed. Shows appropriate
// notifications (no cache found / deletion result).
void promptClearAllCache();

void enforceCacheSizeLimit(const std::vector<int>& songKeys, int totalOffset);

void reduceCacheToSize(int maxSizeMB, std::unordered_set<std::filesystem::path> excludedFiles);
