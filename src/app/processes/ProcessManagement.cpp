#include <windows.h>
#include "ProcessManagement.hpp"
#include <iostream>
#include <cstring>
#include "../encryptDecrypt/Cryption.hpp"
#include <atomic>
#include <mutex>
#include <thread>



ProcessManagement::ProcessManagement(){
    itemsSemaphore = CreateSemaphore(NULL, 0, 1000, NULL);
    emptySlotsSemaphore = CreateSemaphore(NULL,1000,1000,NULL);

    // shmFd = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemory) , SHH_NAME);
    shmFd = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemory), SHH_NAME);

    // ftruncate(shmFd , sizeof(SharedMemory));
    SharedMemory* sharedMem = static_cast<SharedMemory*>(MapViewOfFile(shmFd, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory)));

    sharedMem->front = 0;
    sharedMem->rear = 0;
    sharedMem->size.store(0);
}

ProcessManagement::~ProcessManagement(){
    UnmapViewOfFile(sharedMem);
    CloseHandle(shmFd);
}


bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task){
    
    WaitForSingleObject(emptySlotsSemaphore, INFINITE);
    std::unique_lock<std::mutex> queueLock;

    if(sharedMem->size.load() >= 1000){
        return false;
    }


    // Copy the task string to the shared memory, ensuring it's within the buffer limit
    strncpy(sharedMem->task[sharedMem->rear], task->toString().c_str(), 255); 
    sharedMem->task[sharedMem->rear][255] = '\0';  // Null-terminate the string

    // strcpy(shareMem->task[sharedMem->rear], task->toString().c_str());


    sharedMem->rear = (sharedMem->rear + 1) % 1000;
    sharedMem->size.fetch_add(1);

    queueLock.unlock();
    ReleaseSemaphore(itemsSemaphore, 1, NULL);

    std::thread thread_1(&ProcessManagement::executeTasks,this);
    thread_1.detach();

    return true;
}

void ProcessManagement::executeTasks(){
    WaitForSingleObject(itemsSemaphore, INFINITE);
    std::unique_lock<std::mutex> queueLock;

    char taskStr[256];
    strcpy(taskStr,sharedMem->task[sharedMem->front]);
    sharedMem->front = (sharedMem->front + 1) % 1000;
    sharedMem->size.fetch_sub(1);

    queueLock.unlock();
    ReleaseSemaphore(emptySlotsSemaphore, 1, NULL);



    std::cout<<"Executing child process"<<std::endl;
    executeCryption(taskStr);
    
}
