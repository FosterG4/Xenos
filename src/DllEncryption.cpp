#include "stdafx.h"
#include "DllEncryption.h"
#include "Log.h"
#include <fstream>
#include <memory>
#include <algorithm>

// Encryption key definition
constexpr uint8_t DllEncryption::ENCRYPTION_KEY[];

// Improved memory-based encryption/decryption
void DllEncryption::XorCrypt(const uint8_t* input, uint8_t* output, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        output[i] = input[i] ^ ENCRYPTION_KEY[i % KEY_SIZE];
    }
}

bool DllEncryption::ValidateHeader(const std::vector<uint8_t>& data)
{
    if (data.size() < sizeof(uint32_t) * 2)
        return false;
        
    uint32_t magic = *reinterpret_cast<const uint32_t*>(data.data());
    return magic == MAGIC_HEADER;
}

std::unique_ptr<uint8_t[]> DllEncryption::AllocateSecureMemory(size_t size)
{
    auto memory = std::make_unique<uint8_t[]>(size);
    SecureZeroMemory(memory.get(), size);
    return memory;
}

void DllEncryption::SecureZeroMemory(void* ptr, size_t size)
{
    volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
    for (size_t i = 0; i < size; ++i)
        p[i] = 0;
}

// Fast memory-based decryption (NEW - main improvement)
bool DllEncryption::DecryptToMemory(const std::wstring& encryptedPath, std::vector<uint8_t>& decryptedData)
{
    try
    {
        std::ifstream file(encryptedPath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            xlog::Error("Failed to open encrypted file: %ls", encryptedPath.c_str());
            return false;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize < sizeof(uint32_t) * 2)
        {
            xlog::Error("Invalid encrypted file size: %ls", encryptedPath.c_str());
            return false;
        }

        file.seekg(0, std::ios::beg);

        // Read header
        uint32_t magic, originalSize;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));

        if (magic != MAGIC_HEADER)
        {
            xlog::Error("Invalid magic header in encrypted file: %ls", encryptedPath.c_str());
            return false;
        }

        if (originalSize == 0 || originalSize > 100 * 1024 * 1024) // 100MB limit
        {
            xlog::Error("Invalid original size in encrypted file: %ls", encryptedPath.c_str());
            return false;
        }

        // Read encrypted data
        size_t encryptedSize = fileSize - (sizeof(uint32_t) * 2);
        auto encryptedBuffer = AllocateSecureMemory(encryptedSize);
        file.read(reinterpret_cast<char*>(encryptedBuffer.get()), encryptedSize);
        file.close();

        // Decrypt directly to output vector
        decryptedData.resize(originalSize);
        size_t decryptSize = (encryptedSize < static_cast<size_t>(originalSize)) ? encryptedSize : static_cast<size_t>(originalSize);
        XorCrypt(encryptedBuffer.get(), decryptedData.data(), decryptSize);

        // Secure cleanup
        SecureZeroMemory(encryptedBuffer.get(), encryptedSize);

        xlog::Normal("Successfully decrypted DLL to memory: %ls (%zu bytes)", encryptedPath.c_str(), decryptedData.size());
        return true;
    }
    catch (const std::exception& e)
    {
        xlog::Error("Exception during decryption: %s", e.what());
        return false;
    }
}

bool DllEncryption::ReadEncryptedFile(const std::wstring& filePath, std::vector<uint8_t>& encryptedData)
{
    return ReadFile(filePath, encryptedData);
}

// File-based encryption (improved)
bool DllEncryption::EncryptDll(const std::wstring& inputPath, const std::wstring& outputPath)
{
    try
    {
        std::vector<uint8_t> inputData;
        if (!ReadFile(inputPath, inputData))
        {
            xlog::Error("Failed to read input file for encryption: %ls", inputPath.c_str());
            return false;
        }

        if (inputData.empty())
        {
            xlog::Error("Input file is empty: %ls", inputPath.c_str());
            return false;
        }

        // Create encrypted data with header
        std::vector<uint8_t> encryptedData;
        encryptedData.reserve(inputData.size() + sizeof(uint32_t) * 2);

        // Add magic header
        uint32_t magic = MAGIC_HEADER;
        uint32_t originalSize = static_cast<uint32_t>(inputData.size());
        
        encryptedData.insert(encryptedData.end(), 
            reinterpret_cast<const uint8_t*>(&magic), 
            reinterpret_cast<const uint8_t*>(&magic) + sizeof(magic));
        encryptedData.insert(encryptedData.end(), 
            reinterpret_cast<const uint8_t*>(&originalSize), 
            reinterpret_cast<const uint8_t*>(&originalSize) + sizeof(originalSize));

        // Encrypt data
        size_t startPos = encryptedData.size();
        encryptedData.resize(startPos + inputData.size());
        XorCrypt(inputData.data(), encryptedData.data() + startPos, inputData.size());

        // Write encrypted file
        bool success = WriteFile(outputPath, encryptedData);
        
        // Secure cleanup
        SecureZeroMemory(inputData.data(), inputData.size());
        SecureZeroMemory(encryptedData.data(), encryptedData.size());

        if (success)
            xlog::Normal("Successfully encrypted DLL: %ls -> %ls", inputPath.c_str(), outputPath.c_str());
        else
            xlog::Error("Failed to write encrypted file: %ls", outputPath.c_str());

        return success;
    }
    catch (const std::exception& e)
    {
        xlog::Error("Exception during encryption: %s", e.what());
        return false;
    }
}

bool DllEncryption::IsEncrypted(const std::wstring& filePath)
{
    try
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        uint32_t magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        return file.good() && magic == MAGIC_HEADER;
    }
    catch (...)
    {
        return false;
    }
}

// Legacy compatibility functions
bool DllEncryption::DecryptDll(const std::vector<uint8_t>& encryptedData, std::vector<uint8_t>& decryptedData)
{
    if (!ValidateHeader(encryptedData))
        return false;

    if (encryptedData.size() < sizeof(uint32_t) * 2)
        return false;

    uint32_t originalSize = *reinterpret_cast<const uint32_t*>(encryptedData.data() + sizeof(uint32_t));
    size_t encryptedSize = encryptedData.size() - (sizeof(uint32_t) * 2);

    decryptedData.resize(originalSize);
    size_t decryptSize = (encryptedSize < static_cast<size_t>(originalSize)) ? encryptedSize : static_cast<size_t>(originalSize);
    XorCrypt(encryptedData.data() + sizeof(uint32_t) * 2, 
             decryptedData.data(), 
             decryptSize);

    return true;
}

bool DllEncryption::ReadFile(const std::wstring& filePath, std::vector<uint8_t>& data)
{
    try
    {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return false;

        size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize == 0)
            return false;

        file.seekg(0, std::ios::beg);
        data.resize(fileSize);
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        
        return file.good();
    }
    catch (...)
    {
        return false;
    }
}

bool DllEncryption::WriteFile(const std::wstring& filePath, const std::vector<uint8_t>& data)
{
    try
    {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }
    catch (...)
    {
        return false;
    }
}

// EncryptedDllMemory implementation
EncryptedDllMemory::EncryptedDllMemory(const std::wstring& originalPath)
    : m_originalPath(originalPath)
    , m_decryptedData(nullptr)
    , m_decryptedSize(0)
    , m_isEncrypted(false)
{
}

EncryptedDllMemory::~EncryptedDllMemory()
{
    if (m_decryptedData)
    {
        DllEncryption::SecureZeroMemory(m_decryptedData.get(), m_decryptedSize);
    }
}

bool EncryptedDllMemory::LoadAndDecrypt()
{
    m_isEncrypted = DllEncryption::IsEncrypted(m_originalPath);
    
    if (m_isEncrypted)
    {
        std::vector<uint8_t> decryptedData;
        if (!DllEncryption::DecryptToMemory(m_originalPath, decryptedData))
        {
            xlog::Error("Failed to decrypt DLL to memory: %ls", m_originalPath.c_str());
            return false;
        }

        m_decryptedSize = decryptedData.size();
        m_decryptedData = DllEncryption::AllocateSecureMemory(m_decryptedSize);
        std::memcpy(m_decryptedData.get(), decryptedData.data(), m_decryptedSize);
        
        // Secure cleanup of temporary vector
        DllEncryption::SecureZeroMemory(decryptedData.data(), decryptedData.size());
        
        xlog::Normal("Loaded encrypted DLL to secure memory: %ls (%zu bytes)", m_originalPath.c_str(), m_decryptedSize);
    }
    else
    {
        // Load regular DLL
        std::vector<uint8_t> fileData;
        if (!DllEncryption::ReadFile(m_originalPath, fileData))
        {
            xlog::Error("Failed to read regular DLL: %ls", m_originalPath.c_str());
            return false;
        }

        m_decryptedSize = fileData.size();
        m_decryptedData = DllEncryption::AllocateSecureMemory(m_decryptedSize);
        std::memcpy(m_decryptedData.get(), fileData.data(), m_decryptedSize);
        
        xlog::Normal("Loaded regular DLL to memory: %ls (%zu bytes)", m_originalPath.c_str(), m_decryptedSize);
    }

    return true;
} 