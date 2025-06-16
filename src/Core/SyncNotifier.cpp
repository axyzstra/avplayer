#include "SyncNotifier.h"

namespace av {

void SyncNotifier::Notify() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_triggered = true;
    m_cond.notify_all();
}


bool SyncNotifier::Wait(int timeoutInMilliseconds) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!m_triggered) {
        if (timeoutInMilliseconds < 0) {
            m_cond.wait(lock, [this]() {
                return m_triggered.load();
            });
        } else {
            if (!m_cond.wait_for(lock, std::chrono::milliseconds(timeoutInMilliseconds), 
            [this]() {
                return m_triggered.load();
            }));
        }
    }

    Reset();
    return true;
}

void SyncNotifier::Reset() {
    m_triggered = false;
}

}