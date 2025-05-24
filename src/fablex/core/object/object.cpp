#include "object.h"
#include "core/file_system/archive.h"

namespace fe
{

FE_DEFINE_ROOT_OBJECT(Object);

void Object::serialize(Archive& archive) const
{
    FE_CHECK(archive.is_write_mode());
}

void Object::deserialize(Archive& archive)
{
    FE_CHECK(archive.is_read_mode());
}

}