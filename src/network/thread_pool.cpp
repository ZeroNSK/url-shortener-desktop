#include "thread_pool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t num_threads) : stop_flag(false) { // инициализация пула потоков
    for (size_t i = 0; i < num_threads; ++i) { 
        workers.emplace_back(&ThreadPool::workerThread, this); // создаем рабочий поток
    }
}
ThreadPool::~ThreadPool() {
    shutdown();
}
void ThreadPool::shutdown() { 
    {
        std::unique_lock<std::mutex> lock(queue_mutex); // блокируем очередь
        stop_flag = true;
    }
    condition.notify_all(); 

    for (std::thread& worker : workers) { 
        if (worker.joinable()) {
            worker.join();
        }
    }
}
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex); 
            condition.wait(lock, [this] { // поток "спит" пока нет задач
                return stop_flag || !tasks.empty(); 
            });
            if (stop_flag && tasks.empty()) { // поток выходит если остановлен и задач нет
                return;
            }
            task = std::move(tasks.front()); // берет задачу из очереди и удаляет ее
            tasks.pop();
        }
        try {
            std::cout << "DEBUG: Выполнение задачи в рабочем потоке" << std::endl;
            task(); // поток выполняет любой кусок кода 
            std::cout << "DEBUG: Задача выполнена" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Ошибка в рабочем потоке: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Неизвестная ошибка в рабочем потоке" << std::endl;
        }
    }
}