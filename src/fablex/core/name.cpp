#include "name.h"
#include "macro.h"
#include "name_registry.h"

namespace fe
{

Name::Name() : m_id(s_invalidID)
{

}

Name::Name(const std::string& strName) : m_id(s_invalidID)
{
    m_id = NameRegistry::to_id(strName);
}

Name::Name(const char* strName) : m_id(s_invalidID)
{
    m_id = NameRegistry::to_id(strName);
}

Name::Name(Name::ID id) : m_id(id)
{

}

Name::Name(const Name& other) : m_id(other.m_id)
{

}

Name::Name(Name&& other) : m_id(other.m_id)
{

}

Name& Name::operator=(const Name& other)
{
    m_id = other.m_id;
    return *this;
}

Name& Name::operator=(Name&& other)
{
    m_id = other.m_id;
    return *this;
}

bool Name::operator==(const Name& other) const
{
    return this->m_id == other.m_id;
}

bool Name::operator<(const Name& other) const
{
    return this->m_id < other.m_id;
}

const std::string& Name::to_string() const
{
    FE_CHECK(is_valid());
    return NameRegistry::to_string(m_id);
}

Name::ID Name::to_id() const
{
    FE_CHECK(is_valid());
    return m_id;
}

bool Name::is_valid() const
{
    return m_id != s_invalidID;
}

}