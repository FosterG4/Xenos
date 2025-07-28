#pragma once
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <cstdint>

// Improved DLL Encryption class with memory-based operations
class DllEncryption
{
public:
    // Magic header for encrypted DLLs
    static constexpr uint32_t MAGIC_HEADER = 0x58454E4F; // "XENO"
    
    // Encryption key (can be customized)
    static constexpr uint8_t ENCRYPTION_KEY[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE };
    static constexpr size_t KEY_SIZE = sizeof(ENCRYPTION_KEY);

    // File operations
    static bool EncryptDll(const std::wstring& inputPath, const std::wstring& outputPath);
    static bool IsEncrypted(const std::wstring& filePath);
    
    // Memory operations (NEW - for better injection performance)
    static bool DecryptToMemory(const std::wstring& encryptedPath, std::vector<uint8_t>& decryptedData);
    static bool ReadEncryptedFile(const std::wstring& filePath, std::vector<uint8_t>& encryptedData);
    
    // Legacy file operations (kept for compatibility)
    static bool DecryptDll(const std::vector<uint8_t>& encryptedData, std::vector<uint8_t>& decryptedData);
    static bool ReadFile(const std::wstring& filePath, std::vector<uint8_t>& data);
    static bool WriteFile(const std::wstring& filePath, const std::vector<uint8_t>& data);

    // Internal encryption/decryption (made public for EncryptedDllMemory access)
    static void XorCrypt(const uint8_t* input, uint8_t* output, size_t size);
    static bool ValidateHeader(const std::vector<uint8_t>& data);
    
    // Memory management helpers (made public for EncryptedDllMemory access)
    static std::unique_ptr<uint8_t[]> AllocateSecureMemory(size_t size);
    static void SecureZeroMemory(void* ptr, size_t size);

private:
};

// Encrypted DLL memory container for injection
class EncryptedDllMemory
{
public:
    EncryptedDllMemory(const std::wstring& originalPath);
    ~EncryptedDllMemory();
    
    bool LoadAndDecrypt();
    const uint8_t* GetDecryptedData() const { return m_decryptedData.get(); }
    size_t GetDecryptedSize() const { return m_decryptedSize; }
    bool IsValid() const { return m_decryptedData != nullptr; }
    const std::wstring& GetOriginalPath() const { return m_originalPath; }

private:
    std::wstring m_originalPath;
    std::unique_ptr<uint8_t[]> m_decryptedData;
    size_t m_decryptedSize;
    bool m_isEncrypted;
}; 