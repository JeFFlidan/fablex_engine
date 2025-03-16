#include "synchronization_manager.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

void SynchronizationManager::cleanup()
{
    for (SemaphoreArray& semaphores : m_semaphoresPerFrame)
        for (rhi::Semaphore* semaphore : semaphores)
            rhi::destroy_semaphore(semaphore);

    for (FenceArray& fences : m_freeFencesPerFrame)
        for (rhi::Fence* fence : fences)
            rhi::destroy_fence(fence);

    for (FenceArray& fences : m_usedFencesPerFrame)
        for (rhi::Fence* fence : fences)
            rhi::destroy_fence(fence);

    m_semaphoresPerFrame.clear();
    m_freeFencesPerFrame.clear();
    m_usedFencesPerFrame.clear();
    m_freeSemaphoreIndex = 0;
}

void SynchronizationManager::begin_frame()
{
    m_freeSemaphoreIndex = 0;
    uint32 frameIndex = rhi::get_frame_index();

    if (m_semaphoresPerFrame.size() < frameIndex + 1)
        m_semaphoresPerFrame.emplace_back();
    if (m_freeFencesPerFrame.size() < frameIndex + 1)
        m_freeFencesPerFrame.emplace_back();
    if (m_usedFencesPerFrame.size() < frameIndex + 1)
        m_usedFencesPerFrame.emplace_back();

    CurrentFrameFenceArrays fenceArrays = get_current_frame_fences();

    for (rhi::Fence* usedFence : fenceArrays.usedFences)
        fenceArrays.freeFences.push_back(usedFence);

    fenceArrays.usedFences.clear();
}

void SynchronizationManager::wait_fences()
{
    CurrentFrameFenceArrays fenceArrays = get_current_frame_fences();
    rhi::wait_for_fences(fenceArrays.usedFences);

    for (rhi::Fence* fence : fenceArrays.usedFences)
        fenceArrays.freeFences.push_back(fence);

    fenceArrays.usedFences.clear();
}

rhi::Semaphore* SynchronizationManager::get_semaphore()
{
    SemaphoreArray& curFrameSemaphores = m_semaphoresPerFrame.at(rhi::get_frame_index());

    if (m_freeSemaphoreIndex + 1 > curFrameSemaphores.size())
        rhi::create_semaphore(&curFrameSemaphores.emplace_back());

    return curFrameSemaphores.at(m_freeSemaphoreIndex++);
}

rhi::Fence* SynchronizationManager::get_fence()
{
    CurrentFrameFenceArrays fenceArrays = get_current_frame_fences();

    if (fenceArrays.freeFences.empty())
    {
        rhi::create_fence(&fenceArrays.usedFences.emplace_back());
    }
    else
    {
        rhi::Fence* freeFence = fenceArrays.freeFences.back();
        fenceArrays.usedFences.push_back(freeFence);
        fenceArrays.freeFences.pop_back();
    }

    return fenceArrays.usedFences.back();
}

SynchronizationManager::CurrentFrameFenceArrays SynchronizationManager::get_current_frame_fences()
{
    return CurrentFrameFenceArrays(
        m_freeFencesPerFrame.at(rhi::get_frame_index()),
        m_usedFencesPerFrame.at(rhi::get_frame_index())
    );
}

}