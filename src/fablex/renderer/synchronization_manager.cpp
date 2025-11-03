#include "synchronization_manager.h"
#include "globals.h"
#include "device.h"

namespace fe::renderer
{

SynchronizationManager::SynchronizationManager()
{
    begin_frame();
}

SynchronizationManager::~SynchronizationManager()
{
    m_freeSemaphoreIndex = 0;
}

void SynchronizationManager::begin_frame()
{
    m_freeSemaphoreIndex = 0;

    if (m_semaphoresPerFrame.size() < g_frameIndex + 1)
        m_semaphoresPerFrame.emplace_back();
    if (m_freeFencesPerFrame.size() < g_frameIndex + 1)
        m_freeFencesPerFrame.emplace_back();
    if (m_usedFencesPerFrame.size() < g_frameIndex + 1)
        m_usedFencesPerFrame.emplace_back();

    CurrentFrameFenceArrays fenceArrays = get_current_frame_fences();

    for (FenceHandle& usedFence : fenceArrays.usedFences)
        fenceArrays.freeFences.push_back(std::move(usedFence));

    fenceArrays.usedFences.clear();
}

void SynchronizationManager::end_frame()
{
        
}

void SynchronizationManager::wait_fences()
{
    CurrentFrameFenceArrays fenceArrays = get_current_frame_fences();
    Device::wait_for_fences(fenceArrays.usedFences);

    for (FenceHandle& fence : fenceArrays.usedFences)
        fenceArrays.freeFences.push_back(std::move(fence));

    fenceArrays.usedFences.clear();
}

SemaphoreRef SynchronizationManager::get_semaphore()
{
    SemaphoreArray& curFrameSemaphores = m_semaphoresPerFrame.at(g_frameIndex);

    if (m_freeSemaphoreIndex + 1 > curFrameSemaphores.size())
        curFrameSemaphores.emplace_back();

    return curFrameSemaphores.at(m_freeSemaphoreIndex++);
}

SemaphoreRef SynchronizationManager::get_acquire_semaphore()
{
    if (m_acquireSemaphores.size() < g_frameIndex + 1)
        m_acquireSemaphores.emplace_back();

    return m_acquireSemaphores.at(g_frameIndex);
}

FenceRef SynchronizationManager::get_fence()
{
    CurrentFrameFenceArrays fenceArrays = get_current_frame_fences();

    if (fenceArrays.freeFences.empty())
    {
        fenceArrays.usedFences.emplace_back();
    }
    else
    {
        FenceHandle& freeFence = fenceArrays.freeFences.back();
        fenceArrays.usedFences.push_back(std::move(freeFence));
        fenceArrays.freeFences.pop_back();
    }

    return fenceArrays.usedFences.back();
}

SynchronizationManager::CurrentFrameFenceArrays SynchronizationManager::get_current_frame_fences()
{
    if (m_freeFencesPerFrame.size() < g_frameIndex + 1)
        m_freeFencesPerFrame.emplace_back();
    if (m_usedFencesPerFrame.size() < g_frameIndex + 1)
        m_usedFencesPerFrame.emplace_back();

    return CurrentFrameFenceArrays(
        m_freeFencesPerFrame.at(g_frameIndex),
        m_usedFencesPerFrame.at(g_frameIndex)
    );
}

}