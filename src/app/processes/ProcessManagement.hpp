#ifndef PROCESS_MANAGEMENT_HPP
#define PROCESS_MANAGEMENT_HPP


#include "Task.hpp"
#include <queue>
#include <memory>

class ProcessManagement{
private:
    std::queue<std::unique_ptr<Task>> taskQueue;

public:
    ProcessManagement();
    bool submitToQueue(std::unique_ptr<Task> task);
    void executeTasks();

};


#endif