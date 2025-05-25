#include "file_system.h"
#include "core/macro.h"
// #include "core/logger.h"
// #include "core/platform/platform.h"
#include <random>
#include <fstream>

// TEMP
#ifdef WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#endif // WIN32

// Will return logging in file system when rename log category because current does not work with WinApi GDI 
// FE_DEFINE_LOG_CATEGORY(LogFileSystem)

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
    s_rootPath = rootPath;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10000);

    int num = dist(gen);
    s_projectPath = get_absolute_path(std::string("untitled") + std::to_string(num));
}

void FileSystem::create_project_directory(std::string projectPath)
{
    if (!projectPath.empty())
        s_projectPath = get_absolute_path(projectPath);

    std::filesystem::create_directories(s_projectPath);
}

void FileSystem::create_project(const std::string& projectName)
{
    std::string projectPath = s_rootPath + "/" + "projects" + "/" + projectName;
    std::filesystem::create_directories(projectPath);

    s_projectPath = projectPath;

    std::string filePath = s_projectPath + "/" + projectName + ".aaproj";
    std::ofstream file(filePath);
    file.close();
}

void FileSystem::set_project_path(std::string projectPath)
{
    if (is_relative(projectPath))
        s_projectPath = get_absolute_path(projectPath);
    else
        s_projectPath = projectPath;
}

std::string FileSystem::get_project_name()
{
    std::filesystem::path path(s_projectPath);
    return path.filename().string();
}

bool FileSystem::is_project_existed(std::string projectPath)
{
    if (is_relative(projectPath))
        projectPath = get_absolute_path(projectPath);

    return exists(projectPath);
}

FileStream* FileSystem::open(const std::string& strPath, const char* mode)
{
    std::filesystem::path path = std::filesystem::path(strPath);

    if (!path.is_absolute())
        path = s_rootPath / path;

    if (!std::filesystem::exists(path) && strcmp(mode, "wb") != 0)
    {
        // FE_LOG(LogFileSystem, ERROR, "Path {} is invlaid", path.string().c_str());
        return nullptr;
    }

    if (!path.has_extension())
    {
        // FE_LOG(LogFileSystem, ERROR, "File from path {} has no extension", path.string().c_str());
        return nullptr;
    }

    FILE* file = nullptr;
    fopen_s(&file, path.string().c_str(), mode);

    if (!file)
    {
        // FE_LOG(LogFileSystem, ERROR, "File is invalid after opening {}", strPath);
        return nullptr;
    }

    return new FileStream(file);
}

bool FileSystem::close(FileStream* stream)
{
    if (!stream || !stream->is_valid())
    {
        // FE_LOG(LogFileSystem, ERROR, "close(): FileStream is invalid.");
        
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

void FileSystem::read(const std::string& path, uint64 size, uint8* outData)
{
    FE_CHECK(outData);
    FileStream* stream = open(path, "rb");

    size_t readElements = stream->read(outData, sizeof(uint8), size);
    FE_CHECK(readElements != 0);

    bool afterClose = close(stream);
    FE_CHECK_MSG(afterClose, "Error while closing stream");
}

void FileSystem::read(const std::string& path, uint64 size, std::vector<uint8>& outData)
{
    uint64 offset = 0;

    if (size >= outData.size())
    {
        offset = outData.size();
        outData.resize(outData.size() + size);
    }
    
    FileStream* stream = open(path, "rb");
    size_t readElements = stream->read(outData.data() + offset, sizeof(uint8), size);
    FE_CHECK(readElements != 0);

    bool afterClose = close(stream);
    FE_CHECK_MSG(afterClose, "Error while closing stream");
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
        strPath.erase(strPath.find_last_of("."), strPath.size());
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
    std::filesystem::path relativePath = std::filesystem::relative(path, s_rootPath);
    return relativePath.string().c_str();
}

std::string FileSystem::get_relative_path(const std::string& rootPath, const std::string& targetPath)
{
    std::filesystem::path relativePath = std::filesystem::relative(targetPath, rootPath);
    return relativePath.string().c_str();
}

std::string FileSystem::get_absolute_path(const std::string& path)
{
    std::filesystem::path basePath(s_rootPath);
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

void FileSystem::for_each_file(
    const std::string& path, 
    const std::unordered_set<std::string>& extensions,
    const ForEachCallback& callback,
    DirectoryIteratorType iteratorType
)
{
    if (is_relative(path))
    {
        // FE_LOG(LogFileSystem, ERROR, "FileSystem::for_each_file(): Path {} is relative.", path);
        return;
    }

    if (!exists(path))
    {
        // FE_LOG(LogFileSystem, ERROR, "FileSystem::for_each_file(): Path {} does not exist.", path);
        return;
    }

    switch (iteratorType)
    {
    case DirectoryIteratorType::DEFAULT:
    {
        for (auto& dirEntry : std::filesystem::directory_iterator(path))
        {
            std::string path = dirEntry.path().string();

            if (dirEntry.is_directory() || !extensions.contains(get_file_extension(path)))
                continue;
    
            callback(dirEntry);
        }

        break;
    }
    case DirectoryIteratorType::RECURSIVE:
    {
        for (auto& dirEntry : std::filesystem::recursive_directory_iterator(path))
        {
            std::string path = dirEntry.path().string();

            if (dirEntry.is_directory() || !extensions.contains(get_file_extension(path)))
                continue;
    
            callback(dirEntry);
        }

        break;
    }
    }

}

std::string FileSystem::open_file_dialog(const std::vector<std::string>& extensions)
{
    std::string filterStr = build_filter_string(extensions, "Files");

    char fileName[MAX_PATH] = {};

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filterStr.c_str();
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Open File";

    if (GetOpenFileNameA(&ofn))
        return std::string(fileName);

    return {};
}

void FileSystem::open_files_dialog(const std::vector<std::string>& extensions, std::vector<std::string>& outFiles)
{
    std::string filterStr = build_filter_string(extensions, "Files");

    char buffer[4096] = {};

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filterStr.c_str();
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = sizeof(buffer);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    ofn.lpstrTitle = "Open Files";

    outFiles.clear();

    if (GetOpenFileNameA(&ofn))
    {
        char* ptr = buffer;
        std::string directory = ptr;
        ptr += directory.size() + 1;

        if (*ptr == '\0')
        {
            outFiles.push_back(directory);
        }
        else
        {
            while (*ptr)
            {
                outFiles.push_back(directory + "\\" + ptr);
                ptr += strlen(ptr) + 1;
            }
        }
    }
}

std::string FileSystem::save_file_dialog(const std::vector<std::string>& extensions)
{
    std::string filterStr = build_filter_string(extensions, "Files");;

    char fileName[MAX_PATH] = {};

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filterStr.c_str();
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Save File";

    if (GetSaveFileNameA(&ofn))
        return std::string(fileName);

    return {};
}

std::string FileSystem::open_directory_dialog()
{
    BROWSEINFO browseInfo = {};
    browseInfo.lpszTitle = "Select Folder";
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST itemIdList = SHBrowseForFolder(&browseInfo);
    if (!itemIdList)
        return {};

    char folderPath[MAX_PATH];
    if (!SHGetPathFromIDListA(itemIdList, folderPath))
        return {};

    CoTaskMemFree(itemIdList);
    return std::string(folderPath);
}

void FileSystem::read_internal(FileStream* stream, const std::string& path, uint8* data, uint64 size)
{
    size_t readElements = stream->read(data, sizeof(uint8), size);
    FE_CHECK_MSG(readElements == size, "Data is invalid");
    FE_CHECK_MSG(data, "Data is invalid");

    bool afterClose = close(stream);
    FE_CHECK_MSG(afterClose, "Error while closing stream");
}

FileStream* FileSystem::create_read_stream(const std::string& path, uint64& outSize)
{
    FileStream* stream = open(path, "rb");

    outSize = stream->size();
    if (!outSize)
    {
        // FE_LOG(LogFileSystem, ERROR, "File {} is empty", path.c_str());
        return nullptr;
    }

    return stream;
}

std::string FileSystem::rename_file(const std::string& oldAbsolutePath, const std::string& newName)
{
    if (is_absolute(newName))
    {
        if (exists(oldAbsolutePath))
            std::filesystem::rename(oldAbsolutePath, newName);

        return newName;
    }

    std::filesystem::path oldPath(oldAbsolutePath);
    std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());

    if (exists(oldAbsolutePath))
        std::filesystem::rename(oldPath, newPath);
    
    return newPath.string();
}

uint64 FileSystem::get_last_write_time(const std::string& absolutePath)
{
    auto time = std::filesystem::last_write_time(absolutePath.c_str());
    return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

std::string FileSystem::build_filter_string(const std::vector<std::string>& extensions, const std::string& description)
{
    if (extensions.empty())
    {
        // Default "All Files" filter string
        std::string filterString = "All Files";
        filterString.push_back('\0');
        filterString += "*.*";
        filterString.push_back('\0');
        filterString.push_back('\0');
        return filterString;
    }

    std::ostringstream descStream;
    std::ostringstream extStream;

    descStream << description << " (";

    bool first = true;
    for (const auto& ext : extensions)
    {
        if (!first) descStream << ", ";
        descStream << "*." << ext;
        first = false;
    }
    descStream << ")";

    first = true;
    for (const auto& ext : extensions)
    {
        if (!first) extStream << ";";
        extStream << "*." << ext;
        first = false;
    }

    std::string filterString;
    filterString += descStream.str();
    filterString.push_back('\0');
    filterString += extStream.str();
    filterString.push_back('\0');
    filterString += "All Files";
    filterString.push_back('\0');
    filterString += "*.*";
    filterString.push_back('\0');
    filterString.push_back('\0');

    return filterString;
}

}