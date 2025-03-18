#pragma once

#include "core/types.h"
#include "rhi/fwd.h"
#include <vector>

namespace fe::renderer
{

class SynchronizationManager
{
public:
    SynchronizationManager();
    ~SynchronizationManager();

    void begin_frame();
    void end_frame();
    void wait_fences();

    rhi::Semaphore* get_semaphore();
    rhi::Semaphore* get_acquire_semaphore();
    rhi::Fence* get_fence();

private:
    using SemaphoreArray = std::vector<rhi::Semaphore*>;
    using FenceArray = std::vector<rhi::Fence*>;

    struct CurrentFrameFenceArrays
    {
        FenceArray& freeFences;
        FenceArray& usedFences;
    };

    std::vector<SemaphoreArray> m_semaphoresPerFrame;
    SemaphoreArray m_acquireSemaphores;
    std::vector<FenceArray> m_freeFencesPerFrame;
    std::vector<FenceArray> m_usedFencesPerFrame;
    uint32 m_freeSemaphoreIndex = 0;

    CurrentFrameFenceArrays get_current_frame_fences();
};

}