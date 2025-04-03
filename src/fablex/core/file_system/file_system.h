#pragma once

#include "core/types.h"
#include <vector>
#include <string>

namespace fe
{

class FileStream
{
public:
    FileStream() = default;
    FileStream(FILE* file);
    ~FileStream();

    void close();
    uint64 read(void* data, uint64 size, uint64 count);
    uint64 write(const void* data, uint64 size, uint64 count);
    uint64 size() const;

    bool is_valid() { return m_file; }

private:
    FILE* m_file = nullptr;
};

class FileSystem
{
public:
    static void init(const std::string& rootPath);

    static const std::string& get_root_path() { return m_rootPath; }

    static FileStream* open(const std::string& strPath, const char* mode);
    static bool close(FileStream* stream);

    static void read(const std::string& path, uint8** outData, uint64* outSize);
    static void read(const std::string& path, std::vector<uint8>& outData);
    static void read(const std::string& path, std::string& outData);
    static void read(const std::string& path, uint64 size, uint8* outData);
    static void read(const std::string& path, uint64 size, std::vector<uint8>& outData);

    static void write(const std::string& path, const uint8* data, uint64 size);
    static void write(const std::string& path, const std::vector<uint8>& data);
    static void write(const std::string& path, const std::string& data);

    static std::string get_file_name(const std::string& path);
    static std::string get_file_extension(const std::string& path);
    
    // Uses the root path that was configured by the init method
    static std::string get_relative_path(const std::string& path);
    static std::string get_relative_path(const std::string& rootPath, const std::string& targetPath);

    // Uses the root path that was configured by the init method
    static std::string get_absolute_path(const std::string& path);
    static std::string get_absolute_path(const std::string& rootPath, const std::string& relativePath);

    static bool is_relative(const std::string& path);
    static bool is_absolute(const std::string& path);
    static bool has_extension(const std::string& path);
    static bool exists(const std::string& absolutePath);
    static bool exists(const std::string& rootPath, const std::string& relativePath);

    static uint64 get_last_write_time(const std::string& absolutePath);

private:
    inline static std::string m_rootPath = "";

    static void read_internal(FileStream* stream, const std::string& path, uint8* data, uint64 size);
    static FileStream* create_read_stream(const std::string& path, uint64& outSize);
};

}