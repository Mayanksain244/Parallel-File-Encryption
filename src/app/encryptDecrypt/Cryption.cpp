// #include "Cryption.hpp"
// #include "../processes/Task.hpp"
// #include "../fileHandling/ReadEnv.cpp"

// int executeCryption(const std::string& taskData) {
//     Task task = Task::fromString(taskData);
//     ReadEnv env;
//     std::string envKey = env.getenv();
//     int key = std::stoi(envKey);
//     if (task.action == Action::ENCRYPT) {
//         char ch;
//         while (task.f_stream.get(ch)) {
//             ch = (ch + key) % 256;
//             task.f_stream.seekp(-1, std::ios::cur);
//             task.f_stream.put(ch);
//         }
//         task.f_stream.close();
//     } else {
//         char ch;
//         while (task.f_stream.get(ch)) {
//             ch = (ch - key + 256) % 256;
//             task.f_stream.seekp(-1, std::ios::cur);
//             task.f_stream.put(ch);
//         }
//         task.f_stream.close();
//     }
//     return 0;
// }

#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <cstdint>
#include <vector>
#include <random>

// Function to check if a number is prime
bool isPrime(long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) 
            return false;
    }
    return true;
}

// Function to generate a random prime number within a range
long long generatePrime(long long min, long long max) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<long long> dis(min, max);
    
    long long num = dis(gen);
    // Make sure num is odd
    if (num % 2 == 0) num++;
    
    // Find the next prime number
    while (!isPrime(num)) {
        num += 2;
        if (num > max) num = min | 1; // Start over if we exceed max
    }
    
    return num;
}

// Function to compute GCD using the Euclidean algorithm
long long gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Extended Euclidean Algorithm to compute modular inverse
long long modInverse(long long e, long long phi) {
    long long t = 0, newT = 1;
    long long r = phi, newR = e;

    while (newR != 0) {
        long long quotient = r / newR;
        long long tempT = t;
        t = newT;
        newT = tempT - quotient * newT;

        long long tempR = r;
        r = newR;
        newR = tempR - quotient * newR;
    }

    if (r > 1) throw std::runtime_error("No modular inverse exists.");
    if (t < 0) t += phi;

    return t;
}

// Function to compute modular exponentiation
long long modExp(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}

// Function to generate RSA keys
void generateRSAKeys(long long &n, long long &e, long long &d) {
    // Using fixed prime numbers for consistent encryption/decryption
    long long p = 61;  // Fixed prime
    long long q = 53;  // Fixed prime
    
    n = p * q;  // Modulus
    long long phi = (p - 1) * (q - 1);  // Euler's totient function

    // Fixed public exponent
    e = 17;
    
    // Calculate private key d
    try {
        d = modInverse(e, phi);
    } catch (const std::runtime_error& error) {
        std::cerr << "Error generating keys: " << error.what() << std::endl;
        throw;
    }

    std::cout << "Using RSA keys:" << std::endl;
    std::cout << "n (modulus): " << n << std::endl;
    std::cout << "e (public exponent): " << e << std::endl;
    std::cout << "d (private exponent): " << d << std::endl;
}

// RSA Encryption function
uint64_t rsaEncrypt(unsigned char ch, long long e, long long n) {
    return modExp(static_cast<long long>(ch), e, n);
}

// RSA Decryption function
unsigned char rsaDecrypt(uint64_t ch, long long d, long long n) {
    return static_cast<unsigned char>(modExp(ch, d, n));
}

int executeCryption(const std::string& taskData) {
    Task task = Task::fromString(taskData);
    ReadEnv env;

    // Generate RSA keys
    long long n, e, d;
    try {
        generateRSAKeys(n, e, d);
    } catch (const std::exception& error) {
        std::cerr << "Failed to generate RSA keys: " << error.what() << std::endl;
        return -1;
    }

    // First read the entire file content
    std::vector<char> fileContent;
    {
        std::ifstream file(task.filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file!" << std::endl;
            return -1;
        }
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        fileContent.resize(fileSize);
        file.read(fileContent.data(), fileSize);
        file.close();
    }

    // Process the content
    if (task.action == Action::ENCRYPT) {
        std::vector<uint64_t> encryptedContent;
        
        // Convert size_t to store at the beginning of file
        size_t originalSize = fileContent.size();
        encryptedContent.push_back(static_cast<uint64_t>(originalSize));
        
        // Encrypt each byte
        for (unsigned char ch : fileContent) {
            uint64_t encrypted = rsaEncrypt(ch, e, n);
            encryptedContent.push_back(encrypted);
        }

        // Write back to the same file
        std::ofstream outFile(task.filePath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return -1;
        }
        outFile.write(reinterpret_cast<const char*>(encryptedContent.data()), 
                     encryptedContent.size() * sizeof(uint64_t));
        outFile.close();

    } else if (task.action == Action::DECRYPT) {
        // Read encrypted data
        std::vector<uint64_t> encryptedContent(fileContent.size() / sizeof(uint64_t));
        std::memcpy(encryptedContent.data(), fileContent.data(), fileContent.size());

        // First value is the original file size
        size_t originalSize = static_cast<size_t>(encryptedContent[0]);
        std::vector<char> decryptedContent;
        
        // Decrypt each value (skip the first one as it's the size)
        for (size_t i = 1; i < encryptedContent.size() && decryptedContent.size() < originalSize; i++) {
            unsigned char decrypted = rsaDecrypt(encryptedContent[i], d, n);
            decryptedContent.push_back(decrypted);
        }

        // Write back to the same file
        std::ofstream outFile(task.filePath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return -1;
        }
        outFile.write(decryptedContent.data(), decryptedContent.size());
        outFile.close();
    }

    return 0;
}