#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"

int executeCryption(const std::string& taskData) {
    Task task = Task::fromString(taskData);
    ReadEnv env;
    std::string envKey = env.getenv();
    int key = std::stoi(envKey);
    if (task.action == Action::ENCRYPT) {
        char ch;
        while (task.f_stream.get(ch)) {
            ch = (ch + key) % 256;
            task.f_stream.seekp(-1, std::ios::cur);
            task.f_stream.put(ch);
        }
        task.f_stream.close();
    } else {
        char ch;
        while (task.f_stream.get(ch)) {
            ch = (ch - key + 256) % 256;
            task.f_stream.seekp(-1, std::ios::cur);
            task.f_stream.put(ch);
        }
        task.f_stream.close();
    }
    return 0;
}


// #include "Cryption.hpp"
// #include "../processes/Task.hpp"
// #include "../fileHandling/ReadEnv.cpp"

// #include <openssl/evp.h>
// #include <openssl/aes.h>
// #include <openssl/err.h>

// #include <cstring>
// #include <stdexcept>
// #include <vector>

// std::vector<unsigned char> Cryption::encrypt(const std::vector<unsigned char>& plaintext, const std::string& key) {
//     EVP_CIPHER_CTX *ctx;
//     int len;
//     int ciphertext_len;

//     std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);

//     if(!(ctx = EVP_CIPHER_CTX_new())) 
//         throw std::runtime_error("EVP_CIPHER_CTX_new failed");

//     if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, (unsigned char*)key.c_str(), NULL))
//         throw std::runtime_error("EVP_EncryptInit_ex failed");

//     if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()))
//         throw std::runtime_error("EVP_EncryptUpdate failed");
//     ciphertext_len = len;

//     if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len))
//         throw std::runtime_error("EVP_EncryptFinal_ex failed");
//     ciphertext_len += len;

//     EVP_CIPHER_CTX_free(ctx);

//     ciphertext.resize(ciphertext_len);
//     return ciphertext;
// }

// std::vector<unsigned char> Cryption::decrypt(const std::vector<unsigned char>& ciphertext, const std::string& key) {
//     EVP_CIPHER_CTX *ctx;
//     int len;
//     int plaintext_len;

//     std::vector<unsigned char> plaintext(ciphertext.size());

//     if(!(ctx = EVP_CIPHER_CTX_new()))
//         throw std::runtime_error("EVP_CIPHER_CTX_new failed");

//     if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, (unsigned char*)key.c_str(), NULL))
//         throw std::runtime_error("EVP_DecryptInit_ex failed");

//     if(1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()))
//         throw std::runtime_error("EVP_DecryptUpdate failed");
//     plaintext_len = len;

//     if(1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len))
//         throw std::runtime_error("EVP_DecryptFinal_ex failed");
//     plaintext_len += len;

//     EVP_CIPHER_CTX_free(ctx);

//     plaintext.resize(plaintext_len);
//     return plaintext;
// }

// int Cryption::executeCryption(const std::string& taskData) {
//     Task task = Task::fromString(taskData);
//     ReadEnv env;
//     std::string key = env.getenv();

//     // Ensure key is exactly 32 bytes (256 bits) for AES-256
//     key.resize(32, '\0');

//     std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(task.f_stream), {});

//     std::vector<unsigned char> result;
//     if (task.action == Action::ENCRYPT) {
//         result = encrypt(buffer, key);
//     } else {
//         result = decrypt(buffer, key);
//     }

//     task.f_stream.seekp(0);
//     task.f_stream.write(reinterpret_cast<const char*>(result.data()), result.size());
//     task.f_stream.close();

//     return 0;
// }