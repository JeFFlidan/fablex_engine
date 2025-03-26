#include "file_system.h"
#include "core/macro.h"
#include "core/logger.h"

#include <filesystem>

FE_DEFINE_LOG_CATEGORY(LogFileSystem)

namespace fe
{

FileStream::FileStream(FILE* file)
{
    FE_CHECK(file);
    m_file = file;
}

FileStream::~FileStream()
{
    close();
}

void FileStream::close()
{
    if (m_file)
        fclose(m_file);

    m_file = nullptr;
}

uint64 FileStream::read(void* data, uint64 size, uint64 count)
{
    FE_CHECK_MSG(m_file, "FileStream::read(): file is invalid.");
    return fread(data, size, count, m_file);
}

uint64 FileStream::write(const void* data, uint64 size, uint64 count)
{
    FE_CHECK_MSG(m_file, "FileStream::write(): file is invalid.");
    return fwrite(data, size, count, m_file);
}

uint64 FileStream::size() const
{
    FE_CHECK_MSG(m_file, "FileStream::size(): file is invalid.");
    long position = ftell(m_file);
    fseek(m_file, 0, SEEK_END);
    long size = ftell(m_file);
    fseek(m_file, position, SEEK_SET);
    return (uint64)size;
}

void FileSystem::init(const std::string& rootPath)
{
    m_rootPath = rootPath;
}

FileStream* FileSystem::open(const std::string& strPath, const char* mode)
{
    std::filesystem::path path = std::filesystem::path(strPath);

    if (!path.is_absolute())
        path = m_rootPath / path;

    if (!std::filesystem::exists(path) && strcmp(mode, "wb") != 0)
    {
        FE_LOG(LogFileSystem, ERROR, "Path {} is invlaid", path.string().c_str());
        return nullptr;
    }

    if (!path.has_extension())
    {
        FE_LOG(LogFileSystem, ERROR, "File from path {} has no extension", path.string().c_str());
        return nullptr;
    }

    FILE* file = nullptr;
    fopen_s(&file, path.string().c_str(), mode);

    if (!file)
    {
        FE_LOG(LogFileSystem, ERROR, "File is invalid after opening");
        return nullptr;
    }

    return new FileStream(file);
}

bool FileSystem::close(FileStream* stream)
{
    if (!stream || !stream->is_valid())
    {
        FE_LOG(LogFileSystem, ERROR, "close(): FileStream is invalid.");
        
        if (stream)
            delete stream;

        return false;
    }
    stream->close();
    delete stream;

    return true;
}

void FileSystem::read(const std::string& path, uint8** outData, uint64* outSize)
{
    FileStream* stream = create_read_stream(path, *outSize);
    if (!stream)
        return;

    *outData = new uint8[*outSize];

    read_internal(stream, path, *outData, *outSize);
}

void FileSystem::read(const std::string& path, std::vector<uint8>& outData)
{
    uint64 size;

    FileStream* stream = create_read_stream(path, size);
    if (!stream)
        return;

    outData.resize(size);

    read_internal(stream, path, outData.data(), outData.size());
}

void FileSystem::read(const std::string& path, std::string& outData)
{
    uint64 size;

    FileStream* stream = create_read_stream(path, size);
    if (!stream)
        return;

    outData.resize(size);

    read_internal(stream, path, (uint8*)outData.data(), outData.size());
}

void FileSystem::write(const std::string& path, const uint8* data, uint64 size)
{
    FE_CHECK(data);
    FE_CHECK(size);

    FileStream* stream = open(path, "wb");
    stream->write(data, sizeof(uint8), size);
    close(stream);
}

void FileSystem::write(const std::string& path, const std::vector<uint8>& data)
{
    FE_CHECK(!data.empty());

    write(path, data.data(), data.size());
}

void FileSystem::write(const std::string& path, const std::string& data)
{
    FE_CHECK(!data.empty());

    write(path, (const uint8*)data.data(), data.size());
}

std::string FileSystem::get_file_name(const std::string& path)
{
    std::string strPath = std::filesystem::path(path.c_str()).filename().string();
    if (strPath.find(".") != std::string::npos)
        strPath.erase(strPath.find("."), strPath.size());
    return strPath;
}

std::string FileSystem::get_file_extension(const std::string& path)
{
    std::string extension = std::filesystem::path(path.c_str()).extension().string();
    extension.erase(0, 1);
    return extension;
}

std::string FileSystem::get_relative_path(const std::string& path)
{
    std::filesystem::path relativePath = std::filesystem::relative(path, m_rootPath);
    return relativePath.string().c_str();
}

std::string FileSystem::get_relative_path(const std::string& rootPath, const std::string& targetPath)
{
    std::filesystem::path relativePath = std::filesystem::relative(targetPath, rootPath);
    return relativePath.string().c_str();
}

std::string FileSystem::get_absolute_path(const std::string& path)
{
    std::filesystem::path basePath(m_rootPath);
    std::filesystem::path targetPath(path);
    return (basePath / targetPath).string();
}

std::string FileSystem::get_absolute_path(const std::string& rootPath, const std::string& relativePath)
{
    std::filesystem::path basePath(rootPath);
    std::filesystem::path targetPath(relativePath);
    return (basePath / targetPath).string();
}

bool FileSystem::is_relative(const std::string& path)
{
    return std::filesystem::path(path).is_relative();
}

bool FileSystem::is_absolute(const std::string& path)
{
    return std::filesystem::path(path).is_absolute();
}

bool FileSystem::has_extension(const std::string& path)
{
    return std::filesystem::path(path).has_extension();
}

bool FileSystem::exists(const std::string& absolutePath)
{
    return std::filesystem::exists(absolutePath);
}

bool FileSystem::exists(const std::string& rootPath, const std::string& relativePath)
{
    return std::filesystem::exists(get_absolute_path(rootPath, relativePath));
}

void FileSystem::read_internal(FileStream* stream, const std::string& path, uint8* data, uint64 size)
{
    size_t readElements = stream->read(data, sizeof(uint8), size);
    FE_CHECK_MSG(readElements == size, "Data is invalid");
    FE_CHECK_MSG(data, ("Data " + std::string(path.c_str()) + " is invalid").c_str());

    bool afterClose = close(stream);
    FE_CHECK_MSG(afterClose, "Error while closing stream");
}

FileStream* FileSystem::create_read_stream(const std::string& path, uint64& outSize)
{
    FileStream* stream = open(path, "rb");

    outSize = stream->size();
    if (!outSize)
    {
        FE_LOG(LogFileSystem, ERROR, "File {} is empty", path.c_str());
        return nullptr;
    }

    return stream;
}

uint64 FileSystem::get_last_write_time(const std::string& absolutePath)
{
    auto time = std::filesystem::last_write_time(absolutePath.c_str());
    return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

}