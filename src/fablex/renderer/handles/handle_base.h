#pragma once

#include "rhi/rhi.h"
#include "core/name.h"
#include <utility>

namespace fe::renderer::detail
{

template<typename ResourceType>
struct ResourceTraits;

#define FE_DEFINE_RHI_RESOURCE_TRAITS(ResourceType, CreateFunc, DestroyFunc)   \
    template<>                                                                 \
    struct ResourceTraits<ResourceType>                                        \
    {                                                                          \
        inline static decltype(CreateFunc)& create = CreateFunc;               \
        inline static decltype(DestroyFunc)& destroy = DestroyFunc;            \
    };

template<typename ResourceType>
class ResourceAccessor
{
public:
    [[nodiscard]] ResourceType* get() const noexcept { return m_resource; }
    [[nodiscard]] ResourceType& operator*() const noexcept { return *m_resource; }
    [[nodiscard]] ResourceType* operator->() const noexcept { return m_resource; }
    [[nodiscard]] operator ResourceType*() const noexcept { return m_resource; }

    void set_name(const std::string& name) const
    {
        rhi::set_name(m_resource, name);
    }

protected:
    ResourceType* m_resource = nullptr;

    explicit ResourceAccessor(ResourceType* resource = nullptr) noexcept
        : m_resource(resource) {}

    explicit ResourceAccessor(const ResourceType* resource) noexcept
        : m_resource(const_cast<ResourceType*>(resource)) {} 
};

// If no information is required to create a resource, the ResourceInfoType must be void!
template<typename ResourceType, typename ResourceInfoType, typename ResourceTraitType = ResourceType>
class HandleBase : public ResourceAccessor<ResourceType>
{
    using Base = ResourceAccessor<ResourceType>;
    constexpr static bool s_hasNoInfo = std::is_same_v<ResourceInfoType, void>;

public:
    using Traits = ResourceTraits<ResourceTraitType>;

    HandleBase()
    {
        if constexpr (s_hasNoInfo)
            Traits::create(&this->m_resource);
    }

    template<typename T = ResourceInfoType, std::enable_if_t<!std::is_same_v<T, void>, int> = 0>
    explicit HandleBase(const T& info)
    {
        init(info);
    }

    HandleBase(HandleBase&& other) noexcept
        : Base(std::exchange(other.m_resource, nullptr)) {}

    HandleBase& operator=(HandleBase&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            this->m_resource = std::exchange(other.m_resource, nullptr);
        }
        return *this;
    }

    HandleBase(const HandleBase&) = delete;
    HandleBase& operator=(const HandleBase&) = delete;

    ~HandleBase() { reset(); }

    template<typename T = ResourceInfoType, std::enable_if_t<!std::is_same_v<T, void>, int> = 0>
    void init(const T& info)
    {
        reset();
        Traits::create(&this->m_resource, &info);
    }

    void reset()
    {
        if (this->m_resource)
        {
            Traits::destroy(this->m_resource);
            this->m_resource = nullptr;
        }
    }
};

template<typename ResourceType>
class RefBase : public ResourceAccessor<ResourceType>
{
    using Accessor = ResourceAccessor<ResourceType>;
    using Base = Accessor;

public:
    RefBase() noexcept = default;
    RefBase(const Accessor& resource) noexcept 
        : Base(resource.get()) {}

    RefBase(ResourceType* resource) noexcept
        : Base(resource) { }

    RefBase& operator=(ResourceType* inResource)
    {
        this->m_resource = inResource;
        return *this;
    }
};

#define FE_DEFINE_RHI_RESOURCE_RAII(HandleName, CreateFunc, DestroyFunc)                    \
    namespace detail                                                                        \
    {                                                                                       \
        FE_DEFINE_RHI_RESOURCE_TRAITS(rhi::HandleName, CreateFunc, DestroyFunc);            \
    }                                                                                       \
    using HandleName##Handle = detail::HandleBase<rhi::HandleName, HandleName##CreateInfo>; \
    using HandleName##Ref = detail::RefBase<rhi::HandleName>;

#define FE_DEFINE_RHI_RESOURCE_RAII_NO_CREATE_INFO(HandleName, CreateFunc, DestroyFunc)   \
    namespace detail                                                                        \
    {                                                                                       \
        FE_DEFINE_RHI_RESOURCE_TRAITS(rhi::HandleName, CreateFunc, DestroyFunc);            \
    }                                                                                       \
    using HandleName##Handle = detail::HandleBase<rhi::HandleName, void>;                   \
    using HandleName##Ref = detail::RefBase<rhi::HandleName>;

#define FE_DEFINE_RHI_RESOURCE_RAII_EXTENDED(HandleName, CreateFunc, DestroyFunc)         \
    namespace detail                                                                        \
    {                                                                                       \
        FE_DEFINE_RHI_RESOURCE_TRAITS(rhi::HandleName, CreateFunc, DestroyFunc);            \
        using HandleName##Base = HandleBase<rhi::HandleName, HandleName##CreateInfo>;       \
        using HandleName##RefBase = RefBase<rhi::HandleName>;                               \
        using HandleName##HandleInterface = HandleName##Interface<HandleName##Base>;        \
        using HandleName##RefInterface = HandleName##Interface<HandleName##RefBase>;        \
    }                                                                                       \
    using HandleName##Handle = detail::HandleName##HandleInterface;                         \
    using HandleName##Ref = detail::HandleName##RefInterface;

}