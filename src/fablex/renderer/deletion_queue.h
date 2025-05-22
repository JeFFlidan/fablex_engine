#pragma once

#include <vector>
#include <functional>

namespace fe::renderer
{

using DeletionHandler = std::function<void()>;

class DeletionQueue
{
public:
    ~DeletionQueue();

    void add(const DeletionHandler& deletionHandler);
    void destroy_objects();

private:
    using DeletionHandlers = std::vector<DeletionHandler>;
    std::vector<DeletionHandlers> m_deletionHandlersPerFrame;
};

}