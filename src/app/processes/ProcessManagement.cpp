#include <windows.h>
#include "ProcessManagement.hpp"
#include <iostream>
#include <cstring>
#include "../encryptDecrypt/Cryption.hpp"
#include <atomic>
#include <semaphore>

ProcessManagement::ProcessManagement(){
    sem_t* itemsSemaphore = sem_open("/itmes_Semaphore",O_CREAT , 0666 ,0);
    sem_t* emptySlotsSemaphore = sem_open("/empty_slots_semaphore",O_CREAT , 0666 ,1000);

    shmFd = shm_open(SHH_NAME , O_CREAT | O_RDWR , 0666)
    ftruncate(shmFd , sizeof(SharedMemory));
    sharedMem = static_cast<SharedMemory *> (mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE , MAP_SHARED . shmFd , 0));
    sharedMem->front = 0;
    sharedMem->rear = 0;
    sharedMem->size.store(0);
}

~ProcessManagement(){
    unmap(sharedMem, sizeof(SharedMemory));
    shm_unlink(SHH_NAME);
}


bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task){
    sem_wait(emptySlotsSemaphore);
    std::unique_lock<std::mutex> queueLock;

    if sharedMem->size.load() >= 1000{
        return false;
    }

    strcpy(shareMem->task[sharedMem->rear], task->tostring().c_str());
    sharedMem->rear = (sharedMem->rear + 1) % 1000;
    sharedMem->size.fetch_add(1);

    lock.unlock();
    sem_post(itemsSemaphore)

    std::thread thread_1(&ProcessManagement::executeTask,this);
    thread_1.detach();

    return true;
}

void ProcessManagement::executeTasks(){
    sem_wait(itemsSemaphore);
    std::unique_lock<std::mutex> queueLock;

    char taskStr[256];
    strcpy(taskStr,sharedMem->tasks[sharedMem->front])
    sharedMem->front = (sharedMem->front + 1) % 1000;
    sharedMem->size.fetch_sub(1);

    lock.unlock();
    sem_post(emptySlotsSemaphore);


    std::cout<<"Executing child process"<<std::endl;
    executeCryption(taskStr);
    
}
