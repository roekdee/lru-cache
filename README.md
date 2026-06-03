# lru-cache

A header-only, generic, O(1) LRU cache for modern C++17.

![CI](https://github.com/roekdee/lru-cache/actions/workflows/ci.yml/badge.svg)

## Features

- **Header-only** — drop `include/lru_cache.hpp` into your project, no linking required.
- **Generic** — `LruCache<K, V>` works with any hashable key and any value type.
- **O(1) operations** — `get`, `put`, and `contains` run in amortized constant time.
- **Safe lookups** — `get` returns `std::optional<V>`, so misses are explicit, not exceptions.
- **Correct recency semantics** — reads and updates promote entries to most-recently-used; the least-recently-used entry is evicted on overflow.
- **Warning-clean** — compiles cleanly under `-Wall -Wextra` (and `/W4` on MSVC).

## Usage

```cpp
#include "lru_cache.hpp"
#include <iostream>
#include <optional>
#include <string>

int main() {
    lru::LruCache<int, std::string> cache(2);  // capacity = 2

    cache.put(1, "one");
    cache.put(2, "two");

    if (std::optional<std::string> value = cache.get(1)) {
        std::cout << "1 -> " << *value << '\n';  // promotes key 1 to MRU
    }

    cache.put(3, "three");  // capacity exceeded -> evicts key 2 (the LRU entry)

    std::cout << "contains(2): " << std::boolalpha << cache.contains(2) << '\n';  // false
    std::cout << "size: " << cache.size() << '\n';                                // 2
}
```

## Build & test

Requires CMake 3.14+ and a C++17 compiler. GoogleTest is fetched automatically via `FetchContent`.

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Complexity notes

| Operation    | Time   | How                                                                 |
|--------------|--------|---------------------------------------------------------------------|
| `get`        | O(1)   | Hash-map lookup + `std::list::splice` to promote to the front.      |
| `put`        | O(1)   | Hash-map lookup + front insert; eviction is a back pop.             |
| `contains`   | O(1)   | Hash-map lookup, no recency change.                                 |
| `size`       | O(1)   | Tracked by the underlying list.                                     |

A `std::list` keeps entries in recency order (front = most-recently-used, back =
least-recently-used). A `std::unordered_map<K, list::iterator>` maps each key to
its node, so promotions and evictions splice/pop list nodes in constant time
without invalidating other iterators.

## Tech

- C++17
- `std::list` + `std::unordered_map` + `std::optional`
- CMake (`INTERFACE` library target + `ctest`)
- GoogleTest via CMake `FetchContent`
- GitHub Actions CI (Ubuntu)

## License

MIT — see [LICENSE](LICENSE).
