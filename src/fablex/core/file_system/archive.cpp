#include "archive.h"
#include "file_system.h"
#include "core/utils.h"
#include "core/logger.h"

namespace fe
{

constexpr uint64 g_archiveVersion = 1;

Archive::Archive() : m_mode(Mode::WRITE)
{
    create_empty();
}

Archive::Archive(const std::string& path, Mode mode)
    : m_path(path), m_mode(mode)
{
    switch (m_mode)
    {
    case Mode::WRITE:
    {
        create_empty();
        break;
    }
    case Mode::READ:
    {
        FE_CHECK(!m_path.empty());

        std::vector<uint8> fileData;

        FileSystem::read(path, fileData);
        memcpy(&m_header, fileData.data(), sizeof(Header));

        if (m_header.thumbnailCompressedSize && m_header.thumbnailDecompressedSize)
        {
            std::vector<uint8> thumbnailCompressedData(m_header.thumbnailCompressedSize);
            m_thumbnailData.resize(m_header.thumbnailDecompressedSize);
            memcpy(thumbnailCompressedData.data(), fileData.data() + sizeof(Header), m_header.thumbnailCompressedSize);
            Utils::decompress(thumbnailCompressedData, m_thumbnailData);
        }

        m_data.resize(m_header.decompressedSize);
        uint64 offset = sizeof(Header) + m_header.thumbnailCompressedSize;
        Utils::decompress(fileData, m_data, offset);

        break;
    }
    case Mode::READ_HEADER_ONLY:
    {
        FE_CHECK(!m_path.empty());
        FileSystem::read(m_path, sizeof(Header), reinterpret_cast<uint8*>(&m_header));

        if (m_header.thumbnailCompressedSize && m_header.thumbnailDecompressedSize)
        {
            uint64 dataSize = m_header.compressedSize + sizeof(Header);
            
            std::vector<uint8> compressedData;
            compressedData.resize(dataSize);
            m_thumbnailData.resize(m_header.thumbnailDecompressedSize);

            FileSystem::read(path, dataSize, compressedData);
            Utils::decompress(compressedData, m_thumbnailData, sizeof(Header));
        }

        break;
    }
    }
}

void Archive::save(const std::string& path)
{
    FE_CHECK(!m_data.empty());
    FE_CHECK(!path.empty());

    std::vector<uint8> compressedData;
    m_header.compressedSize = Utils::compress(m_data, compressedData);
    m_header.decompressedSize = m_data.size();

    std::vector<uint8> compressedThumbnailData;
    if (!m_thumbnailData.empty())
    {
        m_header.compressedSize = Utils::compress(m_thumbnailData, compressedThumbnailData);
        m_header.decompressedSize = m_thumbnailData.size();
    }

    uint64 offset = 0;
    std::vector<uint8> generalData(sizeof(Header) + compressedData.size() + compressedThumbnailData.size());
    
    memcpy(generalData.data(), &m_header, sizeof(Header));
    offset += sizeof(Header);
    
    if (!compressedThumbnailData.empty())
    {
        memcpy(generalData.data() + offset, compressedThumbnailData.data(), compressedThumbnailData.size());
        offset += compressedThumbnailData.size();
    }

    memcpy(generalData.data() + offset, compressedData.data(), compressedData.size());

    FileSystem::write(path, generalData);
}

void Archive::create_empty()
{
    m_data.resize(128);
    m_header.version = g_archiveVersion;
}

}