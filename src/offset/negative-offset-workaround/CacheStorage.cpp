#include "CacheStorage.hpp"
#include "../../utils/Utils.hpp"

#include <algorithm>
#include <cstring>
#include <system_error>
#include <unordered_set>
#include <vector>

#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>

using namespace geode::prelude;

// ─── Padded file helpers ─────────────────────────────────────────────────────

// Check if a path points to an original GD song file.
// Original songs are stored as "<songID>.mp3" or "<songID>.ogg" in the
// GD songs directory. Nong songs (e.g. from jukebox) are stored elsewhere
// with arbitrary filenames.
static bool isOriginalSongPath(const gd::string sourcePath) {
    auto stem = lso::utils::getFileNameWithoutExtension(sourcePath);
    // Check if stem is all digits
    return !stem.empty() && stem.find_first_not_of("0123456789") == gd::string::npos;
}

unsigned long hashSourcePath(const gd::string sourcePath) {
    // For original GD songs (numeric filename in songs folder), return 0
    // so they use the old songKey-only naming scheme. This keeps cache
    // files compatible and avoids unnecessary path hashing.
    if (isOriginalSongPath(sourcePath)) {
        return 0;
    }
    // For nong songs (jukebox etc.), hash the full path to distinguish
    // different audio files that share the same GD song ID.
    auto p = sourcePath;
    LOG_MOD_DEBUG("hashSourcePath: nong song path '{}' -> hash {:x}", p, std::hash<gd::string>{}(p));
    return std::hash<gd::string>{}(p);
}

int getSongKey(GJGameLevel* level) {
    return (level->m_songID != 0) ? level->m_songID : (-level->m_audioTrack - 1);
}

int extractSongIdFromPath(gd::string path) {
    // Get the stem (filename without extension)
    auto filenameWithoutExt = lso::utils::getFileNameWithoutExtension(path);

    // Try to parse as integer
    int id = 0;
    auto result = std::from_chars(filenameWithoutExt.data(), filenameWithoutExt.data() + filenameWithoutExt.size(), id);
    if (result.ec == std::errc() && result.ptr == filenameWithoutExt.data() + filenameWithoutExt.size()) {
        return id;
    }
    return -1;
}

std::filesystem::path getCacheDir() {
    std::filesystem::path customPath = Mod::get()->getSettingValue<std::filesystem::path>("padded-cache-path");
    if (customPath.empty()) {
        return Mod::get()->getSaveDir() / "padded_audio_cache";
    }

    std::filesystem::path path1(customPath);

    auto result1 = geode::utils::file::readDirectory(path1, false);
    if(result1.isOk()){
        return path1;
    }

    auto result2 = geode::utils::file::createDirectoryAll(path1);
    if(result2.isOk()){
        return path1;
    }
    log::warn("Custom cache path invalid, falling back to save dir.");
    return Mod::get()->getSaveDir() / "padded_audio_cache";
}

std::filesystem::path getPaddedPath(int songKey, int totalOffset, const gd::string sourcePath) {
    int absTotal = std::abs(totalOffset);
    int paddedLengthMs = ((absTotal + 999) / 1000) * 1000;
    auto pathHash = hashSourcePath(sourcePath);
    if (pathHash == 0) {
        // Original GD song - use songKey-only naming for backward compat
        return getCacheDir() / fmt::format("padded_{}_{}.wav", songKey, paddedLengthMs);
    }
    return getCacheDir() / fmt::format("padded_{}_{:x}_{}.wav", songKey, pathHash, paddedLengthMs);
}

// ─── Cache collection helpers ──────────────────────────────────────────────────

CacheCollection collectRemovableCacheFiles(const std::unordered_set<std::filesystem::path>& excludedFiles) {

    auto cacheDir = getCacheDir();
    LOG_MOD_DEBUG("collectRemovableCacheFiles: scanning cache directory: {}", cacheDir.string());

    // Normalize excluded paths for comparison
    std::unordered_set<std::filesystem::path> excludedNorm;
    for (auto& p : excludedFiles) {
        excludedNorm.insert(p.lexically_normal());
    }

    CacheCollection result;
    std::error_code dirEc;

    if (std::filesystem::exists(cacheDir, dirEc)) {
        LOG_MOD_DEBUG("collectRemovableCacheFiles: cache dir exists, starting directory scan");
        for (auto& entry : std::filesystem::directory_iterator(cacheDir, dirEc)) {
            if (dirEc) {
                LOG_MOD_DEBUG("collectRemovableCacheFiles: directory iterator error: {}", dirEc.message());
                break;
            }
            if (!entry.is_regular_file(dirEc)) continue;
            if (dirEc) break;

            auto& p = entry.path();
            auto name = p.filename().string();
            if (name.find("padded_") != 0 || p.extension() != ".wav") continue;

            result.totalFiles++;

            // Skip excluded files
            if (excludedNorm.count(p.lexically_normal())) {
                result.excludedCount++;
                continue;
            }

            auto ft = entry.last_write_time(dirEc);
            if (dirEc) { dirEc.clear(); continue; }
            auto fs = entry.file_size(dirEc);
            if (dirEc) { dirEc.clear(); continue; }

            result.removable.push_back({entry.path(), ft, fs});
            result.totalSize += fs;
        }
    }

    LOG_MOD_DEBUG("collectRemovableCacheFiles: found {} removable files ({:.1f} MB, {} excluded, {} total on disk)",
              result.removable.size(),
              static_cast<double>(result.totalSize) / (1024.0 * 1024.0),
              result.excludedCount, result.totalFiles);

    return result;
}

// Delete files from collection (sorted oldest-first) until target bytes are freed.
// Returns the number of bytes actually freed.
uintmax_t deleteOldestFiles(std::vector<FileEntry>& files, uintmax_t target) {
    // Sort oldest-first
    std::sort(files.begin(), files.end(),
        [](const FileEntry& a, const FileEntry& b) { return a.time < b.time; });

    uintmax_t freed = 0;
    int deleted = 0;
    for (auto& entry : files) {
        std::error_code rmEc;
        std::filesystem::remove(entry.path, rmEc);
        if (!rmEc) {
            freed += entry.size;
            deleted++;
            LOG_MOD_DEBUG("  Deleted {} ({:.1f} MB)", entry.path.filename().string(),
                      static_cast<double>(entry.size) / (1024.0 * 1024.0));
        } else {
            log::warn("Failed to delete {}: {}", entry.path.string(), rmEc.message());
        }
        if (freed >= target) break;
    }

    LOG_MOD_DEBUG("deleteOldestFiles: freed {:.1f} MB, deleted {} file(s)",
              static_cast<double>(freed) / (1024.0 * 1024.0), deleted);
    return freed;
}

void reduceCacheToSize(int maxSizeMB, std::unordered_set<std::filesystem::path> excludedFiles) {

    auto collection = collectRemovableCacheFiles(excludedFiles);

    uintmax_t maxSizeBytes = static_cast<uintmax_t>(maxSizeMB) * 1024ULL * 1024ULL;
    LOG_MOD_DEBUG("reduceCacheToSize: {:.1f} MB / {} MB limit",
              static_cast<double>(collection.totalSize) / (1024.0 * 1024.0),
              maxSizeMB);

    if (collection.totalSize <= maxSizeBytes) {
        LOG_MOD_DEBUG("reduceCacheToSize: cache size OK, no cleanup needed");
        return;
    }

    uintmax_t target = collection.totalSize - maxSizeBytes;
    uintmax_t freed = deleteOldestFiles(collection.removable, target);

    LOG_MOD_DEBUG("reduceCacheToSize: done, {:.1f} MB over limit, freed {:.1f} MB",
              static_cast<double>(target) / (1024.0 * 1024.0),
              static_cast<double>(freed) / (1024.0 * 1024.0));
}

void enforceCacheSizeLimit(const std::vector<int>& songKeys, int totalOffset) {
    int maxSizeMB = Mod::get()->getSettingValue<int>("padded-cache-max-size");
    if (maxSizeMB < 0) {
        LOG_MOD_DEBUG("reduceCacheToSize: limit disabled (maxSizeMB={})", maxSizeMB);
        return;
    }

    MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
    if (!mdm) {
        LOG_MOD_DEBUG("enforceCacheSizeLimit: MusicDownloadManager not available, skipping cache cleanup");
        return;
    }

    // Build excluded set: only the padded files for the currently active songs.
    // Compute the expected padded file path directly from the source path,
    // without consulting any registry.
    std::unordered_set<std::filesystem::path> excluded;

    for (int songKey : songKeys) {
        auto paddedPath = getPaddedPath(songKey, totalOffset, mdm->pathForSong(songKey));
        excluded.insert(paddedPath.lexically_normal());
    }

    reduceCacheToSize(maxSizeMB, std::move(excluded));
}

void promptClearAllCache() {
    auto collection = collectRemovableCacheFiles({});

    if (collection.totalFiles <= 0) {
        FLAlertLayer::create(
            "Clear Audio Cache",
            "No negative offset audio cache found.",
            "OK"
        )->show();
        return;
    }

    createQuickPopup(
        "Clear Audio Cache",
        fmt::format("Delete all {} cached audio files ({:.2f} MB)?\nOriginal song files won't be deleted.",
            collection.totalFiles,
            static_cast<double>(collection.totalSize) / (1024.0 * 1024.0)),
        "Cancel", "Delete",
        [collection](auto*, bool btn2) mutable {
            if (btn2) {
                deleteOldestFiles(collection.removable, collection.totalSize);
                Notification::create(
                    fmt::format("Cleared {} audio cache ({:.2f} MB)",
                        collection.totalFiles,
                        static_cast<double>(collection.totalSize) / (1024.0 * 1024.0)),
                    NotificationIcon::Success
                )->show();
            }
        }
    );
}
