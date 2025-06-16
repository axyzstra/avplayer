#pragma once
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>

namespace av {

class SyncNotifier {
public:
    SyncNotifier() = default;
    ~SyncNotifier() = default;

    void Notify();
    bool Wait(int timeoutInMilliseconds = -1);
    void Reset();

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_triggered{false};
    bool m_manualReset{false};
};


}