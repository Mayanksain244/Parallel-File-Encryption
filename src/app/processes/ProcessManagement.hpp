#ifndef PROCESS_MANAGEMENT_HPP
#define PROCESS_MANAGEMENT_HPP

#include <windows.h>
#include <iostream>
#include "Task.hpp"
#include <queue>
#include <memory>
#include <atomic>
#include <semaphore>

class ProcessManagement{
    HANDLE itemsSemaphore;
    HANDLE emptySlotsSemaphore;
private:
    struct SharedMemory
    {
        std::atomic<int> size;
        char task[1000][256];
        int front;
        int rear;

        void printSharedMemory(){
            std::cout<<size<<std::endl;
        }
    };

    SharedMemory* sharedMem;
    HANDLE shmFd;
    const char* SHH_NAME = "/my_queue";
    HANDLE queueLock = CreateMutex(NULL, FALSE, NULL);

    
    // std::queue<std::unique_ptr<Task>> taskQueue;

public:
    ProcessManagement();
    ~ProcessManagement();
    bool submitToQueue(std::unique_ptr<Task> task);
    void executeTasks();

};

#endif