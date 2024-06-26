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
        destructorCalled = true;
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
        size_t currentSize = size();
        if (numThreads > currentSize)
        {
            for (int i = currentSize; i < numThreads; ++i)
                workers.emplace_back(&ThreadPool::workerThread, this);
        }
        else 
        {

            workersToKillMutex.lock();
            for (int i = numThreads; i < currentSize; ++i)
            {
                workersToKill.push_back(workers[i].get_id());
            }
            workersToKillMutex.unlock();
            has_task.notify_all();

            for (int i = numThreads; i < currentSize; ++i)
            {
               
                if (workers[i].joinable())
                    workers[i].join();
            }
            workers.resize(numThreads);
        }
    }

    size_t size() const 
    {
        return workers.size();
    }

private:
    std::vector<std::thread> workers;
    std::vector<std::thread::id> workersToKill;
    std::priority_queue<Task> tasks;
    std::mutex queueMutex;
    std::mutex workersToKillMutex;
    std::condition_variable has_task;
    std::atomic_bool destructorCalled = false;

    void workerThread() 
    {
        while (true) 
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                has_task.wait(lock, [this] {
                    return !tasks.empty() || destructorCalled || workersToKill.size() != 0;
                    });

                if (destructorCalled) return;

                std::unique_lock<std::mutex> killLock(workersToKillMutex);
                if (auto it = std::find(workersToKill.begin(), workersToKill.end(), std::this_thread::get_id());
                    it != workersToKill.end())
                {
                    workersToKill.erase(it);
                    return;
                }
                killLock.unlock();

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

