#pragma once

#include <vector>

namespace fe::renderer
{

template<typename HandleType>
class HandleVector
{
public:
    using ValueType = HandleType;
    using ContainerType = std::vector<HandleType>;
    using Iterator = typename ContainerType::iterator;
    using ConstIterator = typename ContainerType::const_iterator;
    using ReverseIterator = typename ContainerType::reverse_iterator;
    using ConstReverseIterator = typename ContainerType::const_reverse_iterator;

    HandleVector() = default;
    HandleVector(HandleVector&&) noexcept = default;
    HandleVector& operator=(HandleVector&&) noexcept = default;
    HandleVector(const HandleVector&) = delete;
    HandleVector& operator=(const HandleVector&) = delete;

    template<typename... Args>
    HandleType& emplace(Args&&... args)
    {
        return m_handles.emplace_back(std::forward<Args>(args)...);
    }

    void add(HandleType&& handle)
    {
        m_handles.emplace_back(std::move(handle));
    }

    void add(HandleType& handle)
    {
        m_handles.emplace_back(std::move(handle));
    }

    void pop_back()
    {
        if (!m_handles.empty())
            m_handles.pop_back();
    }

    bool remove(const HandleType& handle)
    {
        auto it = std::find_if(m_handles.begin(), m_handles.end(),
            [&](const HandleType& h) { return h == handle; });

        if (it != m_handles.end())
        {
            if (it + 1 != m_handles.end()) // only move if not last
                *it = std::move(m_handles.back());
            m_handles.pop_back();
            return true;
        }
        return false;
    }

    void clear()
    {
        m_handles.clear();
    }

    HandleType& front()
    {
        return m_handles.front();
    }

    const HandleType& front() const
    {
        return m_handles.front();
    }

    HandleType& back()
    {
        return m_handles.back();
    }

    const HandleType& back() const
    {
        return m_handles.back();
    }

    [[nodiscard]] size_t size() const noexcept { return m_handles.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_handles.empty(); }

    Iterator begin() noexcept { return m_handles.begin(); }
    Iterator end() noexcept { return m_handles.end(); }
    ConstIterator begin() const noexcept { return m_handles.begin(); }
    ConstIterator end() const noexcept { return m_handles.end(); }
    ReverseIterator rbegin() noexcept { return m_handles.rbegin(); }
    ReverseIterator rend() noexcept { return m_handles.rend(); }
    ConstReverseIterator rbegin() const noexcept { return m_handles.rbegin(); }
    ConstReverseIterator rend() const noexcept { return m_handles.rend(); }

    HandleType& operator[](size_t index) noexcept { return m_handles[index]; }
    const HandleType& operator[](size_t index) const noexcept { return m_handles[index]; }

    HandleType& at(size_t index) noexcept { return m_handles.at(index); }
    const HandleType& at(size_t index) const noexcept { return m_handles.at(index); }

private:
    std::vector<HandleType> m_handles;
};

}