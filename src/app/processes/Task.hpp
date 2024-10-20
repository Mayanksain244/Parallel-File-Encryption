#ifndef TASK_HPP
#define TASK_HPP

#include "../fileHandling/IO.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>  // For runtime_error

enum class Action {
    ENCRYPT,
    DECRYPT
};

struct Task {
    std::string filePath;
    std::fstream f_stream;
    Action action;

    // Fix the initializer order to match the declaration order
    Task(std::fstream&& stream, Action act, std::string filePath) 
        : filePath(filePath), f_stream(std::move(stream)), action(act) {}

    // Converts Task to a string format
    std::string toString() const {
        std::ostringstream oss;
        oss << filePath << "," << (action == Action::ENCRYPT ? "ENCRYPT" : "DECRYPT");
        return oss.str();
    }

    // Static function to create a Task from a string
    static Task fromString(const std::string& taskData) {
        std::istringstream iss(taskData);
        std::string filePath;
        std::string actionStr;

        // Parse file path and action
        if (std::getline(iss, filePath, ',') && std::getline(iss, actionStr)) {
            Action action = (actionStr == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;

            // Create an IO object to handle file I/O
            IO io(filePath);
            std::fstream f_stream = io.getFileStream();  // No need to use std::move here

            // Ensure the file is open
            if (f_stream.is_open()) {
                return Task(std::move(f_stream), action, filePath);
            } else {
                throw std::runtime_error("Failed to open file: " + filePath);
            }
        } else {
            throw std::runtime_error("Invalid task data format");
        }
    }
};

#endif  // TASK_HPP



// // Task.hpp
// #ifndef TASK_HPP
// #define TASK_HPP

// #include "../fileHandling/IO.hpp"
// #include <fstream>
// #include <string>
// #include <sstream>

// enum class Action {
//     ENCRYPT,
//     DECRYPT
// };

// struct Task {
//     std::string filePath;
//     std::fstream f_stream;
//     Action action;

//     Task(std::fstream&& stream, Action act, std::string filePath) : f_stream(std::move(stream)), action(act), filePath(filePath) {}

//     std::string toString() const {
//         std::ostringstream oss;
//         oss << filePath << "," << (action == Action::ENCRYPT ? "ENCRYPT" : "DECRYPT");
//         return oss.str();
//     }

//     static Task fromString(const std::string& taskData) {
//         std::istringstream iss(taskData);
//         std::string filePath;
//         std::string actionStr;

//         if (std::getline(iss, filePath, ',') && std::getline(iss, actionStr)) {
//             Action action = (actionStr == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;
//             IO io(filePath);
//             std::fstream f_stream = std::move(io.getFileStream());
//             if (f_stream.is_open()) {
//                 return Task(std::move(f_stream), action, filePath);
//             } else {
//                 throw std::runtime_error("Failed to open file: " + filePath);
//             }
//         } else {
//             throw std::runtime_error("Invalid task data format");
//         }
//     }
// };

// #endif