#pragma once

#include "handles/buffer.h"

namespace fe::renderer
{

class StagingBuffer
{
public:
    StagingBuffer(const BufferCreateInfo& info)
        : m_buffer(info)
    {
        m_data = m_buffer.mapped_data();
        m_buffer.set_name("STAGING");
    }

    void reset()
    {
        m_buffer.reset();
    }

    void add_to_offset(uint64 offsetDelta)
    {
        m_offset += offsetDelta;
        m_data += offsetDelta;
    }

    void set_offset(uint64 offset)
    {
        m_offset = offset;
    }

    template<typename ValueType>
    void write(const ValueType& value, uint64 index)
    {
        memcpy(reinterpret_cast<ValueType*>(m_data) + index, &value, sizeof(ValueType));
    }

    void write_chunk(const void* dataToWrite, uint64 size)
    {
        memcpy(m_data, dataToWrite, size);
    }

    uint64 offset() const { return m_offset; }
    BufferHandle& handle() { return m_buffer; }
    const BufferHandle& handle() const { return m_buffer; }

private:
    BufferHandle m_buffer;
    uint8* m_data = nullptr;
    uint64 m_offset = 0;
};

}