cmake_minimum_required(VERSION 3.10)
project(EncryptDecrypt)

set(CMAKE_CXX_STANDARD 17)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
    src/app/encryptDecrypt
    src/app/fileHandling
    src/app/processes
)

file(GLOB MAIN_SRC
    main.cpp
    src/app/processes/ProcessManagement.cpp
    src/app/fileHandling/IO.cpp
    src/app/fileHandling/ReadEnv.cpp
    src/app/encryptDecrypt/Cryption.cpp
)

add_executable(encrypt_decrypt ${MAIN_SRC})
