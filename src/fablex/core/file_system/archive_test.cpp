#include "archive_test.h"
#include "file_system.h"
#include "core/logger.h"

namespace fe
{

constexpr uint32 VALUE1 = 25;
constexpr uint64 VALUE2 = 126;
constexpr Float3 VALUE3 = {2, 4, 8};
const std::string VALUE4 = "Archive Test";
const std::vector<float> VALUE5 = {1.5f, 2.5f, 3.7f};
const std::string PATH = "archive_test.feasset";

void ArchiveTest::run()
{
    UUID uuid;

    Archive writeArchive;
    writeArchive.set_uuid(uuid);
    writeArchive << VALUE1 << VALUE2 << VALUE3 << VALUE4 << VALUE5;
    writeArchive.save(FileSystem::get_absolute_path(PATH));

    Archive readHeaderArchive(PATH, Archive::Mode::READ_HEADER_ONLY);
    FE_CHECK(readHeaderArchive.is_read_header_only_mode());
    FE_LOG(LogDefault, INFO, "UUID from header: {}", (uint64)readHeaderArchive.get_uuid());
    FE_CHECK(readHeaderArchive.get_uuid() == uuid);

    Archive readArchive(PATH, Archive::Mode::READ);
    FE_CHECK(readArchive.is_read_mode());
    uint32 readValue1;
    uint64 readValue2;
    Float3 readValue3;
    std::string readValue4;
    std::vector<float> readValue5;
    readArchive >> readValue1 >> readValue2 >> readValue3 >> readValue4 >> readValue5;

    FE_CHECK(VALUE1 == readValue1);
    FE_CHECK(VALUE2 == readValue2);
    FE_CHECK(VALUE3.x == readValue3.x && VALUE3.y == readValue3.y && VALUE3.z == readValue3.z);
    FE_CHECK(VALUE4 == VALUE4);
    FE_CHECK(VALUE5 == VALUE5);
}

}