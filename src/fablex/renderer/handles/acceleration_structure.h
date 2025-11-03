#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"
#include "rhi/resources/acceleration_structure.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::AccelerationStructure, 
    rhi::create_acceleration_structure, 
    rhi::destroy_acceleration_structure
);

template<typename Base>
class AccelerationStructureInterface : public Base
{
public:
    using Base::Base;

    uint32 descriptor() const { return this->m_resource->descriptorIndex; }
    rhi::AccelerationStructureInfo::Flags flags() const { return this->m_resource->info.flags; }
};

template<typename Base>
class TLASInterface : public AccelerationStructureInterface<Base>
{
public:
    using Parent = AccelerationStructureInterface<Base>;

    using Parent::Parent;

    void set_instance_buffer_name(const std::string& name)
    {
        rhi::set_name(this->m_resource->info.tlas.instanceBuffer, name);
    }

    void write_instance(TLASInstance& instance, void* dst) const
    {
        rhi::write_top_level_acceleration_structure_instance(&instance, dst);
    }

    rhi::TLAS& operator*() const noexcept { return this->m_resource->info.tlas; }
    rhi::TLAS* operator->() const noexcept { return &this->m_resource->info.tlas; }
};

template<typename Base>
class BLASInterface : public AccelerationStructureInterface<Base>
{
public:
    using Parent = AccelerationStructureInterface<Base>;

    using Parent::Parent;

    rhi::BLAS& operator*() const noexcept { return this->m_resource->info.blas; }
    rhi::BLAS* operator->() const noexcept { return &this->m_resource->info.blas; }
};

using TLASHandleBase = HandleBase<
    rhi::AccelerationStructure, 
    AccelerationStructureCreateInfo
>;

using TLASRefBase = RefBase<rhi::AccelerationStructure>;

using BLASHandleBase = HandleBase<
    rhi::AccelerationStructure, 
    AccelerationStructureCreateInfo
>;

using BLASRefBase = RefBase<rhi::AccelerationStructure>;

using TLASHandleInterface = TLASInterface<TLASHandleBase>;
using TLASRefInterface = TLASInterface<TLASRefBase>;
using BLASHandleInterface = BLASInterface<BLASHandleBase>;
using BLASRefInterface = BLASInterface<BLASRefBase>;

}

using TLASHandle = detail::TLASHandleInterface;
using TLASRef = detail::TLASRefInterface;
using BLASHandle = detail::BLASHandleInterface;
using BLASRef = detail::BLASRefInterface;

}