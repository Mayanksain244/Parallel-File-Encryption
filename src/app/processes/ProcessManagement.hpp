#ifndef PROCESS_MANAGEMENT_HPP
#define PROCESS_MANAGEMENT_HPP


#include "Task.hpp"
#include <queue>
#include <memory>
#include <atomic>
#include <semaphore>

class ProcessManagement{
    sem_t* itemsSemaphore;
    sem_t* emptySemaphore;
private:
    struct SharedMemory
    {
        std::atomic<int> size;
        char tasks[1000][256];
        int front;
        int rear;

        void printSharedMemory(){
            std::cout<<size<<std::endl;
        }
    };

    SharedMemory* sharedMem;
    int shmFd;
    const char* SHH_NAME = "/my_queue";
    std::mutex queueLock;
    
    // std::queue<std::unique_ptr<Task>> taskQueue;

public:
    ProcessManagement();
    ~ProcessManagement();
    bool submitToQueue(std::unique_ptr<Task> task);
    void executeTasks();

};


#endif