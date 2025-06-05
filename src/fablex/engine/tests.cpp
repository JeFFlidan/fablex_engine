#include "entity/sparse_set.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

using namespace fe::engine;

struct TestComponent
{
    uint32 value = 0;
};

TEST_CASE("Tesing sparse set") 
{
    SparseSet<TestComponent> sparseSet;
    std::vector<SparseSetEntry> entries;
    uint32 entriesCount = 8;

    for (uint32 i = 0; i != entriesCount; ++i)
    {
        const SparseSetEntry& entry = entries.emplace_back();
        TestComponent* component = sparseSet.insert(entry);
        REQUIRE(component);

        component->value = entry.id();
    }

    REQUIRE(entries.size() == entriesCount);
    REQUIRE(sparseSet.size() == entriesCount);

    for (const SparseSetEntry& entry : entries)
        CHECK(sparseSet.has(entry));

    uint32 firstEntryToRemoveIdx = 4;
    uint32 secondEntryToRemoveIdx = 6;

    sparseSet.erase(entries[firstEntryToRemoveIdx]);
    sparseSet.erase(entries[secondEntryToRemoveIdx]);

    REQUIRE(sparseSet.size() == entriesCount - 2);

    CHECK(!sparseSet.has(entries[firstEntryToRemoveIdx]));
    CHECK(!sparseSet.has(entries[secondEntryToRemoveIdx]));

    sparseSet.for_each([&](TestComponent* component)
    {
        REQUIRE(component);
        CHECK(component->value < entries.size());
    });
}
