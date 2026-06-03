# lru-cache

A header-only LRU cache for C++17. `get`, `put`, and `contains` are O(1).

![CI](https://github.com/roekdee/lru-cache/actions/workflows/ci.yml/badge.svg)

It's a `LruCache<K, V>` that works with any hashable key. `get` hands back a `std::optional<V>` so a miss is just `nullopt` instead of an exception. Reads and updates bump the entry to most-recently-used; when you go over capacity the least-recently-used entry gets dropped.

No build step — just drop `include/lru_cache.hpp` into your project.

```cpp
#include "lru_cache.hpp"

lru::LruCache<int, std::string> cache(2);  // capacity = 2
cache.put(1, "one");
cache.put(2, "two");

if (auto v = cache.get(1)) {        // hit, promotes key 1
    std::cout << *v << '\n';
}

cache.put(3, "three");              // over capacity -> evicts key 2
cache.contains(2);                  // false
```

## Build & test

Needs CMake 3.14+ and a C++17 compiler. GoogleTest is pulled in via `FetchContent`.

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## How it works

A `std::list` holds entries in recency order (front = MRU, back = LRU). A `std::unordered_map<K, list::iterator>` maps each key to its node, so promoting an entry is a `splice` and evicting is a `pop_back` — both constant time, and neither invalidates the other iterators.

## Notes

I leaned on `std::list` + iterators specifically because `splice` moves a node without invalidating the iterators stored in the map. That's the whole trick that keeps everything O(1) without re-hashing or copying values around.

What it doesn't do: it's not thread-safe — wrap your own mutex if you need concurrent access. There's also no TTL or size-in-bytes limit, just a fixed entry count. A per-entry expiry is the first thing I'd add if I picked this back up.

## License

MIT — see [LICENSE](LICENSE).
