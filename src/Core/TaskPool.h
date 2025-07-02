#pragma once

#include <functional>
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <atomic>


namespace av {

class TaskPool {
public:
    TaskPool();
    ~TaskPool();
    void SubmitTask(std::function<void()> task);

private:
    void ThreadLoop();

private:
    std::thread m_thread;
    std::queue<std::function<void()>> m_tasks;
    std::condition_variable m_taskCondition;
    std::mutex m_taskMutex;
    std::atomic<bool> m_stopFlag{false};
};

}