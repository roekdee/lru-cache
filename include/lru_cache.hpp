#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace lru {

/// A generic, header-only Least-Recently-Used (LRU) cache.
///
/// All core operations (get, put, peek, contains) run in amortized O(1) time by
/// combining a doubly linked list that tracks recency order with a hash map
/// from key to the list node holding that key's entry. The front of the list
/// is the most-recently-used (MRU) entry; the back is the least-recently-used
/// (LRU) entry and is the first to be evicted when the cache is full.
///
/// \tparam K Key type. Must be hashable by std::hash<K> and equality comparable.
/// \tparam V Value type.
template <typename K, typename V>
class LruCache {
public:
    /// Construct a cache with a fixed maximum number of entries.
    /// \param capacity Maximum number of entries. Must be greater than zero.
    /// \throws std::invalid_argument if capacity is zero.
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("LruCache capacity must be greater than zero");
        }
    }

    /// Look up a key, returning its value if present.
    ///
    /// On a hit the entry is promoted to most-recently-used. Returns
    /// std::nullopt on a miss (which does not modify recency order).
    std::optional<V> get(const K& key) {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return std::nullopt;
        }
        // Promote to front (MRU) without copying the stored value.
        entries_.splice(entries_.begin(), entries_, it->second);
        return it->second->second;
    }

    /// Look up a key without affecting recency order.
    ///
    /// Like get(), returns the value on a hit and std::nullopt on a miss, but
    /// unlike get() the entry is NOT promoted to most-recently-used. This makes
    /// peek() a const, non-mutating read suitable for inspecting cache contents
    /// without disturbing eviction order.
    std::optional<V> peek(const K& key) const {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return std::nullopt;
        }
        return it->second->second;
    }

    /// Insert or update a key/value pair.
    ///
    /// If the key already exists its value is overwritten and the entry is
    /// promoted to most-recently-used. If inserting a new key exceeds the
    /// capacity, the least-recently-used entry is evicted first.
    void put(const K& key, V value) {
        auto it = index_.find(key);
        if (it != index_.end()) {
            it->second->second = std::move(value);
            entries_.splice(entries_.begin(), entries_, it->second);
            return;
        }

        if (entries_.size() >= capacity_) {
            evictLeastRecentlyUsed();
        }

        entries_.emplace_front(key, std::move(value));
        index_[key] = entries_.begin();
    }

    /// Return true if the key is present, without affecting recency order.
    bool contains(const K& key) const {
        return index_.find(key) != index_.end();
    }

    /// Return the current number of stored entries.
    std::size_t size() const noexcept {
        return entries_.size();
    }

    /// Return the maximum number of entries the cache can hold.
    std::size_t capacity() const noexcept {
        return capacity_;
    }

    /// Return true if the cache holds no entries.
    bool empty() const noexcept {
        return entries_.empty();
    }

    /// Remove all entries from the cache.
    void clear() noexcept {
        entries_.clear();
        index_.clear();
    }

private:
    using Entry = std::pair<K, V>;
    using EntryList = std::list<Entry>;
    using EntryIterator = typename EntryList::iterator;

    void evictLeastRecentlyUsed() {
        const Entry& lru = entries_.back();
        index_.erase(lru.first);
        entries_.pop_back();
    }

    std::size_t capacity_;
    // Front = most-recently-used, back = least-recently-used.
    EntryList entries_;
    std::unordered_map<K, EntryIterator> index_;
};

}  // namespace lru

#endif  // LRU_CACHE_HPP
