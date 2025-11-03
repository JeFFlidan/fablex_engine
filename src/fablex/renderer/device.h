#pragma once

#include "handles/command_buffer.h"
#include "handles/synchronization.h"
#include "rhi/rhi.h"
#include "rhi/resources/cmd.h"

namespace fe::renderer
{

class RendererConfig;

class SubmitInfo
{
public:
    void add_cmd(CommandBufferRef cmd)
    {
        m_info.queueType = cmd->cmdPool->queueType;
        m_info.cmdBuffers.push_back(cmd);
    }

    void add_signal_semaphore(SemaphoreRef semaphore)
    {
        m_info.signalSemaphores.push_back(semaphore);
    }

    void set_signal_semaphores(const std::vector<SemaphoreRef>& semaphores)
    {
        fill_semaphores(semaphores, m_info.signalSemaphores);
    }

    void add_wait_semaphore(SemaphoreRef semaphore)
    {
        m_info.waitSemaphores.push_back(semaphore);
    }

    void set_wait_semaphores(const std::vector<SemaphoreRef>& semaphores)
    {
        fill_semaphores(semaphores, m_info.waitSemaphores);
    }

    void set_signal_fence(FenceRef fence)
    {
        m_info.signalFence = fence;
    }

    void reset()
    {
        m_info.clear();
    }

    [[nodiscard]] rhi::SubmitInfo& get() { return m_info; }
    [[nodiscard]] const rhi::SubmitInfo& get() const { return m_info; }

private:
    rhi::SubmitInfo m_info;

    void fill_semaphores(const std::vector<SemaphoreRef>& inSemaphores, std::vector<rhi::Semaphore*>& outSemaphores)
    {
        outSemaphores.reserve(inSemaphores.size());

        for (SemaphoreRef semaphore : inSemaphores)
            outSemaphores.push_back(semaphore);
    }
};

class PresentInfo
{
public:
    void add_swap_chain(SwapChainRef swapChain)
    {
        m_info.swapChains.push_back(swapChain);
    }

    void add_wait_semaphore(SemaphoreRef semaphore)
    {
        m_info.waitSemaphores.push_back(semaphore);
    }

    [[nodiscard]] rhi::PresentInfo& get() { return m_info; }
    [[nodiscard]] const rhi::PresentInfo& get() const { return m_info; }

private:
    rhi::PresentInfo m_info;
};

class Device
{
public:
    static void init(const RendererConfig* config);
    static void cleanup();

    static void update_frame_index(SwapChainRef swapChain);
    static void submit(const SubmitInfo& submitInfo);
    static void present(const PresentInfo& presentInfo);
    static void wait_queue_idle(QueueType queueType);
    static void wait_for_fences(const std::vector<FenceHandle>& fences);

    static API api();
    static const GPUProperties& gpu_properties();
    static GPUCapability gpu_capability();
    static uint64 shader_identifier_size();
    static uint64 shader_identifier_alignment();
    static uint64 acceleration_structure_instance_size();
    static uint64 timestamp_frequency();
    static uint64 vendor_id();
    static uint64 device_id();
    static const std::string& gpu_name();
    static const std::string& drive_description();
    static bool has_capability(GPUCapability capability);
    static bool is_validation_enabled();

    static uint32 queue_count();
};

}