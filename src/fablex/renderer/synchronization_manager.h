#pragma once

#include "core/types.h"
#include "handles/synchronization.h"
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

    SemaphoreRef get_semaphore();
    SemaphoreRef get_acquire_semaphore();
    FenceRef get_fence();

private:
    using SemaphoreArray = std::vector<SemaphoreHandle>;
    using FenceArray = std::vector<FenceHandle>;

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