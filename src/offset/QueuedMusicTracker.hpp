#pragma once

struct QueuedMusic {
    int timeRemainingMs;
    int musicID;
    bool paused = false;
    int tick(int const tickLengthMs) {
        timeRemainingMs=timeRemainingMs-tickLengthMs;
        return timeRemainingMs;
    }
    [[nodiscard]] bool shouldStart() const {
        return timeRemainingMs<=0;
    }
    [[nodiscard]] int getStartOffset() const {
        return -timeRemainingMs;
    }
};

// Tracks what channels has music delayed due to having a negative start.
class QueuedMusicTracker final {
    // Channel -> QueuedMusic
    std::map<int, QueuedMusic> queuedMusic;
    // Remaining partial milliseconds from last ticking
    float partialMs = 0;
    std::mutex mutex;
    bool callingInTracker = false;
public:
    static QueuedMusicTracker& get() {
        static QueuedMusicTracker instance;
        return instance;
    }
    void tickMs(int tickLengthMs);

    bool hasChannel(int channelID) const;

    [[nodiscard]] bool getCallingInTracker() const {
        return callingInTracker;
    }

    void tickFloatSeconds(float tickLengthSeconds);

    // Called when a channel is played, paused, or stopped, clears queued music on the channel
    void clearChannel(int channel);
    void queueChannel(int channel, int musicID, int timeRemainingMs);
    void pauseAll();

    void clear();
};
