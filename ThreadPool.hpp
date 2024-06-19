#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

#include "Task.hpp"

class ThreadPool {
public:
    ThreadPool(size_t numThreads) 
    {
        for (size_t i = 0; i < numThreads; ++i)
            workers.emplace_back(&ThreadPool::workerThread, this);
    }

    ~ThreadPool() 
    {
        pause = true;
        has_task.notify_all();
        for (std::thread& worker : workers)
            if (worker.joinable())
                worker.join();
    }

    void enqueueTask(Task&& task) 
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(std::move(task));
        }
        has_task.notify_one();
    }

    void resize(size_t numThreads) 
    {
        pause = true;
        has_task.notify_all();
        for (std::thread& worker : workers)
            if (worker.joinable())
                worker.join();

        workers.clear();
        workers.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i)
            workers.emplace_back(&ThreadPool::workerThread, this);

        pause = false;
        has_task.notify_all();
    }

    size_t size() const 
    {
        return workers.size();
    }

private:
    std::vector<std::thread> workers;
    std::priority_queue<Task> tasks;
    std::mutex queueMutex;
    std::condition_variable has_task;
    std::atomic_bool pause = false;

    void workerThread() 
    {
        while (!pause) 
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                has_task.wait(lock, [this] {
                    return !tasks.empty() || pause;
                    });

                if (pause) return;

                if (!tasks.empty()) {
                    task = std::move(tasks.top());
                    tasks.pop();
                }
                else continue;
            }
            task.func();
        }
    }
};

