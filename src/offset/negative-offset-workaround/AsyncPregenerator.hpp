#pragma once

#include <Geode/Geode.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;



// Represents one song to pre-generate.
struct PregenerateTask {
    int songKey;
    int totalOffset;        // negative value
    gd::string sourcePath;  // original audio file
};

// Status of a single task.
enum class PregenerateStatus {
    Pending,
    Generating,
    Completed,
    Failed
};

// Progress info for a single song.
struct SongProgress {
    int songKey;
    PregenerateStatus status;
    gd::string label;       // e.g. "837148" or "Built-in track 1"
    float progress;          // 0.0 – 1.0
    gd::string message;     // e.g. "Decoding...", "Encoding Vorbis...", "Done"
};

// Overall progress snapshot.
struct PregenerateProgress {
    std::vector<SongProgress> songs;
    int completedCount = 0;
    int failedCount = 0;
    int totalCount = 0;
};

// Callback type for progress updates.
// Called on an unspecified background thread - the callback should dispatch
// to the main thread (e.g. via Loader::get()->queueInMainThread) for UI work.
using PregenerateCallback = std::function<void(const PregenerateProgress&)>;

class AsyncPregenerator {
public:
    // Get the singleton instance.
    static AsyncPregenerator& get();

    // Start pre-generating padded files for a set of tasks.
    // If generation is already running, the new tasks are queued.
    // @param tasks      List of songs to pre-generate.
    // @param callback   Called periodically with progress updates.
    void generate(std::vector<PregenerateTask> tasks, PregenerateCallback callback);

    // Block until all current generation completes.
    void waitAll();

    // Cancel any ongoing generation. Already-started tasks will finish
    // their current file but no new ones will start.
    void cancel();

    // Check if generation is currently running.
    bool isRunning() const;

    // Check if a specific song key is currently being written.
    // If true, the padded file may be incomplete and should not be used.
    bool isSongInProgress(int songKey) const;

    // Get the current progress (thread-safe snapshot).
    PregenerateProgress getProgress() const;

private:
    AsyncPregenerator() = default;
    ~AsyncPregenerator();
    AsyncPregenerator(const AsyncPregenerator&) = delete;
    AsyncPregenerator& operator=(const AsyncPregenerator&) = delete;

    // Internal: the worker that processes one task.
    void processTask(const PregenerateTask& task);

    // Internal: the coordinator that spawns threads and calls the callback.
    void runCoordinator(std::vector<PregenerateTask> tasks, PregenerateCallback callback);

    mutable std::mutex m_mutex;

    // Progress state (protected by m_mutex)
    std::vector<SongProgress> m_progress;
    std::unordered_set<int> m_inProgressKeys;  // Song keys currently being written
    int m_completedCount = 0;
    int m_failedCount = 0;
    int m_totalCount = 0;
    bool m_running = false;
    bool m_cancelled = false;

    // Thread management
    std::vector<std::thread> m_threads;
    std::thread m_coordinatorThread;
};

// Convenience: collect all song keys for a level into a list of
// PregenerateTask. Returns empty vector if no negative offset workaround
// is needed.
//
// @param level       The GJGameLevel.
// @param totalOffset The total offset (original + user). If >= 0, returns empty.
// @return List of pre-generation tasks, one per song key.
std::vector<PregenerateTask> collectPregenerateTasks(GJGameLevel* level, int totalOffset);
