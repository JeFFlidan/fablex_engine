#pragma once

#include "core/types.h"
#include "core/uuid.h"
#include "core/math.h"
#include <string>

namespace fe
{

class FileStream;

class Archive
{
public:
    enum class Mode
    {
        READ,
        WRITE,
        READ_HEADER_ONLY
    };

    struct Header
    {
        static constexpr uint64 s_undefined = ~0u;

        uint64 version = 0;
        uint64 uuid = s_undefined;
        uint64 compressedSize = 0;
        uint64 decompressedSize = 0;
        uint64 objectType = s_undefined;
        uint64 thumbnailCompressedSize = 0;
        uint64 thumbnailDecompressedSize = 0;
    };

    // Creates empty binary archive for writing
    Archive();
    Archive(const std::string& path, Mode mode = Mode::READ);

    void save(const std::string& path);

    // Sets header UUID
    void set_uuid(UUID uuid) { m_header.uuid = uuid; }

    template<typename Enum>
    void set_object_type(Enum objectType) 
    { 
        FE_COMPILE_CHECK((std::is_enum_v<Enum>));
        m_header.objectType = std::to_underlying(objectType); 
    }

    const Header& get_header() const { return m_header; }
    uint64 get_version() const { return m_header.version; }
    UUID get_uuid() const { return m_header.uuid; }

    template<typename Enum>
    typename std::enable_if<std::is_enum_v<Enum>, Enum>::type get_object_type() const 
    {
        return static_cast<Enum>(m_header.objectType); 
    }

    bool is_write_mode() const { return m_mode == Mode::WRITE; }
    bool is_read_mode() const { return m_mode == Mode::READ; }
    bool is_read_header_only_mode() const { return m_mode == Mode::READ_HEADER_ONLY; }

    void set_thumbnail(const std::vector<uint8>& thumbnailData);
    void get_thumbnail(std::vector<uint8>& outThumbnailData);

    Archive& operator<<(bool data)
    {
        write(static_cast<uint32>(data ? 1 : 0));
        return *this;
    }

    Archive& operator<<(char data)
    {
        write(static_cast<int8>(data));
        return *this;
    }

    Archive& operator<<(unsigned char data)
    {
        write(static_cast<uint8>(data));
        return *this;
    }

    Archive& operator<<(short data)
    {
        write(static_cast<int16>(data));
        return *this;
    }

    Archive& operator<<(unsigned short data)
    {
        write(static_cast<uint16>(data));
        return *this;
    }

    Archive& operator<<(int data)
    {
        write(static_cast<int64>(data));
        return *this;
    }

    Archive& operator<<(unsigned int data)
    {
        write(static_cast<uint64>(data));
        return *this;
    }

    Archive& operator<<(long data)
    {
        write(static_cast<int64>(data));
        return *this;
    }

    Archive& operator<<(unsigned long data)
    {
        write(static_cast<uint64>(data));
        return *this;
    }

    Archive& operator<<(long long data)
    {
        write(static_cast<int64>(data));
        return *this;
    }

    Archive& operator<<(unsigned long long data)
    {
        write(static_cast<uint64>(data));
        return *this;
    }

    Archive& operator<<(UUID uuid)
    {
        write(static_cast<uint64>(uuid));
        return *this;
    }

    Archive& operator<<(float data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(double data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Float2& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Float3& data)
    {
        write(data);
        return *this;
    }
    
    Archive& operator<<(const Float4& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Int2& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Int3& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Int4& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const UInt2& data)
    {
        write(data);
        return *this;
    }
    
    Archive& operator<<(const UInt3& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const UInt4& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Float3x3& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Float3x4& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Float4x3& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const Float4x4& data)
    {
        write(data);
        return *this;
    }

    Archive& operator<<(const std::string& data)
    {
        (*this) << data.length();
        for (const char& x : data)
            (*this) << x;
        return *this;
    }

    template<typename T>
    Archive& operator<<(const std::vector<T>& data)
    {
        (*this) << data.size();
        for (const T& x : data)
            (*this) << x;
        return *this;
    }

    Archive& operator>>(bool& outData)
    {
        uint32 temp;
        read(temp);
        outData = temp == 1;
        return *this;
    }

    Archive& operator>>(char& outData)
    {
        int8 temp;
        read(temp);
        outData = static_cast<char>(temp);
        return *this;
    }

    Archive& operator>>(unsigned char& outData)
    {
        uint8 temp;
        read(temp);
        outData = static_cast<unsigned char>(temp);
        return *this;
    }

    Archive& operator>>(short& outData)
    {
        int16 temp;
        read(temp);
        outData = static_cast<short>(temp);
        return *this;
    }

    Archive& operator>>(unsigned short& outData)
    {
        uint16 temp;
        read(temp);
        outData = static_cast<unsigned short>(temp);
        return *this;
    }

    Archive& operator>>(int& outData)
    {
        int64 temp;
        read(temp);
        outData = static_cast<int>(temp);
        return *this;
    }

    Archive& operator>>(unsigned int& outData)
    {
        uint64 temp;
        read(temp);
        outData = static_cast<unsigned int>(temp);
        return *this;
    }

    Archive& operator>>(long& outData)
    {
        int64 temp;
        read(temp);
        outData = static_cast<long>(temp);
        return *this;
    }

    Archive& operator>>(unsigned long& outData)
    {
        uint64 temp;
        read(temp);
        outData = static_cast<unsigned long>(temp);
        return *this;
    }

    Archive& operator>>(long long& outData)
    {
        int64 temp;
        read(temp);
        outData = static_cast<long long>(temp);
        return *this;
    }

    Archive& operator>>(unsigned long long& outData)
    {
        uint64 temp;
        read(temp);
        outData = static_cast<unsigned long long>(temp);
        return *this;
    }

    Archive& operator>>(UUID& outUUID)
    {
        read(outUUID);
        return *this;
    }

    Archive& operator>>(float& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(double& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float2& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float3& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float4& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Int2& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Int3& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Int4& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(UInt2& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(UInt3& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(UInt4& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float3x3& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float3x4& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float4x3& outData)
    {
        read(outData);
        return *this;
    }

    Archive& operator>>(Float4x4& outData)
    {
        read(outData);
        return *this;
    }

    template<typename T>
    Archive& operator>>(std::vector<T>& outData)
    {
        uint64 size;
        (*this) >> size;
        outData.resize(size);
        
        for (uint32 i = 0; i != size; ++i)
            (*this) >> outData[i];

        return *this;
    }

    Archive& operator>>(std::string& outData)
    {
        uint64 len;
        (*this) >> len;
        outData.resize(len);
        
        for (uint32 i = 0; i != len; ++i)
            (*this) >> outData[i];

        return *this;
    }

protected:
    std::string m_path;
    Mode m_mode = Mode::READ;

    Header m_header;
    std::vector<uint8> m_thumbnailData;
    std::vector<uint8> m_data;
    uint64 m_position = 0;

    template<typename T>
    void write(const T& data)
    {
        FE_CHECK(m_mode == Mode::WRITE);
        FE_CHECK(!m_data.empty());
        
        const uint64 newPos = m_position + sizeof(T);
        if (newPos >= m_data.size())
            m_data.resize(m_data.size() * 2);

        memcpy(m_data.data() + m_position, &data, sizeof(T));
        m_position = newPos;
    }

    template<typename T>
    void read(T& outData)
    {
        FE_CHECK(m_mode == Mode::READ);
        FE_CHECK(!m_data.empty());
        FE_CHECK(m_position < m_data.size());

        outData = *reinterpret_cast<const T*>(m_data.data() + m_position);
        m_position += sizeof(T);
    }

    void create_empty();
};

}