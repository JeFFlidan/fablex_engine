#pragma once

#include <mutex>
#include <vector>
#include <functional>

namespace fe::renderer
{

using DeletionHandler = std::move_only_function<void()>;

class DeletionQueue
{
public:
    ~DeletionQueue();

    void add(DeletionHandler&& deletionHandler);

    template<typename HandleType>
    void add(HandleType& handle)
    {
        add([h = std::move(handle)]() mutable { h.reset(); });
    }

    void destroy_objects();

private:
    using DeletionHandlers = std::vector<DeletionHandler>;
    
    std::mutex m_mutex;
    std::vector<DeletionHandlers> m_deletionHandlersPerFrame;
};

}