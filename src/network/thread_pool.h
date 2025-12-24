#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop_flag;
    
public:
    ThreadPool(size_t num_threads = 4);
    ~ThreadPool();
    
    template<class F>
    void enqueue(F&& f);
    
    void shutdown();
    
private:
    void workerThread();
};

// пеализация template метода в заголовочном файле
template<class F>
void ThreadPool::enqueue(F&& f) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        if (stop_flag) {
            std::cout << "DEBUG: Остановлен, задача не добавлена" << std::endl;
            return;
        }
        
        tasks.emplace(std::forward<F>(f));
        std::cout << "DEBUG: Задача добавлена, размер очереди: " << tasks.size() << std::endl;
    }
    condition.notify_one();
}