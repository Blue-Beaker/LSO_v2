#pragma once

struct QueuedMusic {
    int timeRemainingMs;
    int musicID;
    bool paused = false;
    int tick(int tickLengthMs) {
        timeRemainingMs=timeRemainingMs-tickLengthMs;
        return timeRemainingMs;
    }
    bool shouldStart() {
        return timeRemainingMs<=0;
    }
    int getStartOffset() {
        return -timeRemainingMs;
    }
};

class QueuedMusicTracker {
    // Channel -> QueuedMusic
    std::map<int, QueuedMusic> m_queuedMusic;
    // Remaining partial milliseconds from last ticking
    float m_partialMs = 0;
public:
    static QueuedMusicTracker& get() {
        static QueuedMusicTracker instance;
        return instance;
    }
    void tickMs(int tickLengthMs);

    bool hasChannel(int channelID);

    void tickFloatSeconds(float tickLengthSeconds);

    // Called when a channel is played, paused, or stopped, clears queued music on the channel
    void clearChannel(int channel);
    void queueChannel(int channel, int musicID, int timeRemainingMs);
    void pauseAll();

    void clear();
};
