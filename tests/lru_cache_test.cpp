#include "lru_cache.hpp"

#include <optional>
#include <string>

#include <gtest/gtest.h>

using lru::LruCache;

TEST(LruCacheTest, ConstructorRejectsZeroCapacity) {
    EXPECT_THROW((LruCache<int, int>(0)), std::invalid_argument);
}

TEST(LruCacheTest, GetOnMissReturnsNullopt) {
    LruCache<int, std::string> cache(2);
    EXPECT_FALSE(cache.get(42).has_value());
}

TEST(LruCacheTest, PutThenGetReturnsValue) {
    LruCache<int, std::string> cache(2);
    cache.put(1, "one");

    auto value = cache.get(1);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "one");
    EXPECT_EQ(cache.size(), 1u);
}

TEST(LruCacheTest, ContainsDoesNotAffectRecency) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);

    // contains() must not promote key 1, so it stays the LRU entry.
    EXPECT_TRUE(cache.contains(1));
    cache.put(3, 30);  // evicts the LRU entry (key 1)

    EXPECT_FALSE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
}

TEST(LruCacheTest, PeekOnMissReturnsNullopt) {
    LruCache<int, std::string> cache(2);
    EXPECT_FALSE(cache.peek(42).has_value());
}

TEST(LruCacheTest, PeekReturnsValueWithoutPromoting) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);  // key 1 is currently the LRU entry
    cache.put(2, 20);

    // peek() returns the value but must NOT promote key 1.
    auto value = cache.peek(1);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 10);

    // Because key 1 was not promoted, it stays the LRU entry and is the one
    // evicted when a new key pushes the cache over capacity.
    cache.put(3, 30);
    EXPECT_FALSE(cache.contains(1));  // peeked-oldest got evicted
    EXPECT_TRUE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
}

TEST(LruCacheTest, PeekIsConst) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);

    const auto& const_cache = cache;
    auto value = const_cache.peek(1);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 10);
    EXPECT_FALSE(const_cache.peek(99).has_value());
    EXPECT_TRUE(const_cache.contains(1));
    EXPECT_FALSE(const_cache.contains(99));
}

TEST(LruCacheTest, ContainsOnMissReturnsFalse) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);
    EXPECT_FALSE(cache.contains(99));
}

TEST(LruCacheTest, EvictsLeastRecentlyUsedOnOverflow) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);  // capacity exceeded -> evict key 1

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_EQ(*cache.get(2), 20);
    EXPECT_EQ(*cache.get(3), 30);
    EXPECT_EQ(cache.size(), 2u);
}

TEST(LruCacheTest, GetPromotesEntryToMostRecentlyUsed) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);

    // Access key 1, making key 2 the least-recently-used.
    EXPECT_EQ(*cache.get(1), 10);
    cache.put(3, 30);  // should evict key 2, not key 1

    EXPECT_TRUE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
}

TEST(LruCacheTest, UpdatingExistingKeyMovesItToMostRecentlyUsed) {
    LruCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);

    // Re-put key 1: value updated and promoted to MRU.
    cache.put(1, 100);
    EXPECT_EQ(*cache.get(1), 100);
    EXPECT_EQ(cache.size(), 2u);

    cache.put(3, 30);  // should evict key 2 (now LRU)
    EXPECT_TRUE(cache.contains(1));
    EXPECT_FALSE(cache.contains(2));
    EXPECT_TRUE(cache.contains(3));
}

TEST(LruCacheTest, UpdatingExistingKeyDoesNotChangeSize) {
    LruCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(1, 11);
    cache.put(1, 12);
    EXPECT_EQ(cache.size(), 1u);
    EXPECT_EQ(*cache.get(1), 12);
}

TEST(LruCacheTest, CapacityOneEvictsOnEveryNewKey) {
    LruCache<int, int> cache(1);
    cache.put(1, 10);
    EXPECT_EQ(*cache.get(1), 10);

    cache.put(2, 20);  // evicts key 1
    EXPECT_FALSE(cache.contains(1));
    EXPECT_EQ(*cache.get(2), 20);
    EXPECT_EQ(cache.size(), 1u);
}

TEST(LruCacheTest, ReportsCapacityAndEmptiness) {
    LruCache<int, int> cache(5);
    EXPECT_EQ(cache.capacity(), 5u);
    EXPECT_TRUE(cache.empty());

    cache.put(1, 10);
    EXPECT_FALSE(cache.empty());
}

TEST(LruCacheTest, ClearRemovesAllEntries) {
    LruCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.clear();

    EXPECT_EQ(cache.size(), 0u);
    EXPECT_TRUE(cache.empty());
    EXPECT_FALSE(cache.contains(1));
    EXPECT_FALSE(cache.get(2).has_value());
}

TEST(LruCacheTest, WorksWithStringKeys) {
    LruCache<std::string, int> cache(2);
    cache.put("alpha", 1);
    cache.put("beta", 2);

    EXPECT_EQ(*cache.get("alpha"), 1);
    cache.put("gamma", 3);  // beta is LRU after accessing alpha

    EXPECT_TRUE(cache.contains("alpha"));
    EXPECT_FALSE(cache.contains("beta"));
    EXPECT_TRUE(cache.contains("gamma"));
}

TEST(LruCacheTest, RepeatedAccessKeepsHotKeyResident) {
    LruCache<int, int> cache(3);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);

    for (int round = 0; round < 5; ++round) {
        EXPECT_EQ(*cache.get(1), 1);  // keep key 1 hot
        cache.put(10 + round, round);  // churn other slots
    }
    EXPECT_TRUE(cache.contains(1));
}
