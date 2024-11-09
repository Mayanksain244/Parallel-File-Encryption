#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <random>
#include <stdexcept>

// AES constants
constexpr int Nb = 4;  // Number of columns in state
constexpr int Nk = 4;  // Number of 32-bit words in key (4 for AES-128)
constexpr int Nr = 10; // Number of rounds (10 for AES-128)

// Forward S-box
const unsigned char sBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Inverse S-box
const unsigned char invSBox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// Rcon lookup table
const unsigned char Rcon[11] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

class AES {
private:
    std::vector<unsigned char> key;
    std::vector<std::vector<unsigned char>> roundKeys;

    // Helper functions
    unsigned char gmul(unsigned char a, unsigned char b) {
        unsigned char p = 0;
        for (int i = 0; i < 8; i++) {
            if (b & 1) p ^= a;
            bool hi_bit_set = a & 0x80;
            a <<= 1;
            if (hi_bit_set) a ^= 0x1B;
            b >>= 1;
        }
        return p;
    }

    void subBytes(std::vector<std::vector<unsigned char>>& state) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                state[i][j] = sBox[state[i][j]];
    }

    void invSubBytes(std::vector<std::vector<unsigned char>>& state) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                state[i][j] = invSBox[state[i][j]];
    }

    void shiftRows(std::vector<std::vector<unsigned char>>& state) {
        for (int i = 1; i < 4; i++) {
            std::vector<unsigned char> temp = state[i];
            for (int j = 0; j < 4; j++)
                state[i][j] = temp[(j + i) % 4];
        }
    }

    void invShiftRows(std::vector<std::vector<unsigned char>>& state) {
        for (int i = 1; i < 4; i++) {
            std::vector<unsigned char> temp = state[i];
            for (int j = 0; j < 4; j++)
                state[i][j] = temp[(j - i + 4) % 4];
        }
    }

    void mixColumns(std::vector<std::vector<unsigned char>>& state) {
        for (int c = 0; c < 4; c++) {
            unsigned char s0 = state[0][c];
            unsigned char s1 = state[1][c];
            unsigned char s2 = state[2][c];
            unsigned char s3 = state[3][c];

            state[0][c] = gmul(0x02, s0) ^ gmul(0x03, s1) ^ s2 ^ s3;
            state[1][c] = s0 ^ gmul(0x02, s1) ^ gmul(0x03, s2) ^ s3;
            state[2][c] = s0 ^ s1 ^ gmul(0x02, s2) ^ gmul(0x03, s3);
            state[3][c] = gmul(0x03, s0) ^ s1 ^ s2 ^ gmul(0x02, s3);
        }
    }

    void invMixColumns(std::vector<std::vector<unsigned char>>& state) {
        for (int c = 0; c < 4; c++) {
            unsigned char s0 = state[0][c];
            unsigned char s1 = state[1][c];
            unsigned char s2 = state[2][c];
            unsigned char s3 = state[3][c];

            state[0][c] = gmul(0x0e, s0) ^ gmul(0x0b, s1) ^ gmul(0x0d, s2) ^ gmul(0x09, s3);
            state[1][c] = gmul(0x09, s0) ^ gmul(0x0e, s1) ^ gmul(0x0b, s2) ^ gmul(0x0d, s3);
            state[2][c] = gmul(0x0d, s0) ^ gmul(0x09, s1) ^ gmul(0x0e, s2) ^ gmul(0x0b, s3);
            state[3][c] = gmul(0x0b, s0) ^ gmul(0x0d, s1) ^ gmul(0x09, s2) ^ gmul(0x0e, s3);
        }
    }

    void addRoundKey(std::vector<std::vector<unsigned char>>& state, const std::vector<unsigned char>& roundKey) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                state[i][j] ^= roundKey[i + 4 * j];
    }

    void keyExpansion() {
        roundKeys.resize(Nr + 1, std::vector<unsigned char>(16));
        std::copy(key.begin(), key.end(), roundKeys[0].begin());

        for (int i = 1; i <= Nr; i++) {
            std::vector<unsigned char> temp = roundKeys[i - 1];
            
            // Rotate last word
            unsigned char t = temp[12];
            temp[12] = temp[13];
            temp[13] = temp[14];
            temp[14] = temp[15];
            temp[15] = t;

            // Apply S-box
            temp[12] = sBox[temp[12]];
            temp[13] = sBox[temp[13]];
            temp[14] = sBox[temp[14]];
            temp[15] = sBox[temp[15]];

            // XOR with Rcon
            temp[12] ^= Rcon[i];

            // Generate new round key
            for (int j = 0; j < 16; j++) {
                if (j < 4) roundKeys[i][j] = temp[j] ^ roundKeys[i-1][j];
                else roundKeys[i][j] = roundKeys[i][j-4] ^ roundKeys[i-1][j];
            }
        }
    }

public:
    AES(const std::vector<unsigned char>& initialKey) : key(initialKey) {
        if (key.size() != 16) {
            throw std::runtime_error("Invalid key size. AES-128 requires a 16-byte key.");
        }
        keyExpansion();
    }

    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& input) {
        if (input.size() != 16) {
            throw std::runtime_error("Input size must be 16 bytes for AES-128.");
        }

        // Initialize state array
        std::vector<std::vector<unsigned char>> state(4, std::vector<unsigned char>(4));
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                state[i][j] = input[i + 4 * j];

        // Initial round
        addRoundKey(state, roundKeys[0]);

        // Main rounds
        for (int round = 1; round < Nr; round++) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKeys[round]);
        }

        // Final round
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKeys[Nr]);

        // Convert state back to output
        std::vector<unsigned char> output(16);
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                output[i + 4 * j] = state[i][j];

        return output;
    }

    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& input) {
        if (input.size() != 16) {
            throw std::runtime_error("Input size must be 16 bytes for AES-128.");
        }

        // Initialize state array
        std::vector<std::vector<unsigned char>> state(4, std::vector<unsigned char>(4));
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                state[i][j] = input[i + 4 * j];

        // Initial round
        addRoundKey(state, roundKeys[Nr]);

        // Main rounds
        for (int round = Nr - 1; round > 0; round--) {
            invShiftRows(state);
            invSubBytes(state);
            addRoundKey(state, roundKeys[round]);
            invMixColumns(state);
        }

        // Final round
        invShiftRows(state);
        invSubBytes(state);
        addRoundKey(state, roundKeys[0]);

        // Convert state back to output
        std::vector<unsigned char> output(16);
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                output[i + 4 * j] = state[i][j];

        return output;
    }
};

// Function to generate a random 128-bit key
std::vector<unsigned char> generateAESKey() {
    std::vector<unsigned char> key(16);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < 16; i++) {
        key[i] = static_cast<unsigned char>(dis(gen));
    }
    
    return key;
}

// Main execution function that matches your existing structure
int executeCryption(const std::string& taskData) {
    Task task = Task::fromString(taskData);
    ReadEnv env;

    // Generate or load AES key
    std::vector<unsigned char> key = generateAESKey();
    AES aes(key);

    // First read the entire file content
    std::vector<unsigned char> fileContent;
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
        file.read(reinterpret_cast<char*>(fileContent.data()), fileSize);
        file.close();
    }

    if (task.action == Action::ENCRYPT) {
        // Pad the input to be multiple of 16 bytes
        size_t originalSize = fileContent.size();
        size_t paddingSize = (16 - (originalSize % 16)) % 16;
        fileContent.resize(originalSize + paddingSize, static_cast<unsigned char>(paddingSize));

        // Store the key at the beginning of the file
        std::vector<unsigned char> encryptedContent;
        encryptedContent.insert(encryptedContent.end(), key.begin(), key.end());
        
        // Encrypt each block
        for (size_t i = 0; i < fileContent.size(); i += 16) {
            std::vector<unsigned char> block(fileContent.begin() + i, fileContent.begin() + i + 16);
            std::vector<unsigned char> encryptedBlock = aes.encrypt(block);
            encryptedContent.insert(encryptedContent.end(), encryptedBlock.begin(), encryptedBlock.end());
        }

        // Write back to the same file
        std::ofstream outFile(task.filePath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return -1;
        }
        outFile.write(reinterpret_cast<const char*>(encryptedContent.data()), encryptedContent.size());
        outFile.close();

    } else if (task.action == Action::DECRYPT) {
        if (fileContent.size() < 16) {
            std::cerr << "Error: Invalid encrypted file format!" << std::endl;
            return -1;
        }

        // Extract the key from the beginning of the file
        std::vector<unsigned char> storedKey(fileContent.begin(), fileContent.begin() + 16);
        AES aes(storedKey);
// Remove key from the beginning of file content
        std::vector<unsigned char> encryptedData(fileContent.begin() + 16, fileContent.end());
        
        // Check if the remaining data size is valid
        if (encryptedData.size() % 16 != 0) {
            std::cerr << "Error: Corrupted encrypted file!" << std::endl;
            return -1;
        }

        // Decrypt each block
        std::vector<unsigned char> decryptedContent;
        for (size_t i = 0; i < encryptedData.size(); i += 16) {
            std::vector<unsigned char> block(encryptedData.begin() + i, encryptedData.begin() + i + 16);
            std::vector<unsigned char> decryptedBlock = aes.decrypt(block);
            decryptedContent.insert(decryptedContent.end(), decryptedBlock.begin(), decryptedBlock.end());
        }

        // Remove padding
        if (!decryptedContent.empty()) {
            unsigned char paddingSize = decryptedContent.back();
            if (paddingSize <= 16 && paddingSize <= decryptedContent.size()) {
                decryptedContent.resize(decryptedContent.size() - paddingSize);
            }
        }

        // Write back to the same file
        std::ofstream outFile(task.filePath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return -1;
        }
        outFile.write(reinterpret_cast<const char*>(decryptedContent.data()), decryptedContent.size());
        outFile.close();
    }

    return 0;
}