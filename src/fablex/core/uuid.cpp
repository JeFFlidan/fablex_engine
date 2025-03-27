#include "uuid.h"
#include <random>

namespace fe
{

static std::random_device sRandomDevice;
static std::mt19937_64 sRandomEngine(sRandomDevice());
static std::uniform_int_distribution<uint64_t> sUniformDistribution;

UUID::UUID() : m_uuid(sUniformDistribution(sRandomEngine))
{
    if (m_uuid == 0)
        m_uuid = sUniformDistribution(sRandomEngine);
}

UUID::UUID(uint64_t uuid) : m_uuid(uuid)
{
    
}

}