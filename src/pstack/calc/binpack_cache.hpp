#ifndef PSTACK_CALC_SPATIAL_HASH_HPP
#define PSTACK_CALC_SPATIAL_HASH_HPP

#include "pstack/geo/vector3.hpp"
#include <unordered_map>

namespace pstack::calc {

template <typename T> struct SpatialKey {
  T x, y, z;
  auto operator<=>(const SpatialKey &other) const = default;
};

template <typename CoordT, typename ValueType> struct BinPackCache {
  BinPackCache(CoordT cell_size) : cell_size(cell_size) {}

  SpatialKey<CoordT> key_for_vector(const geo::vector3<CoordT> &vector) const {
    return {vector.x / cell_size, vector.y / cell_size, vector.z / cell_size};
  }

  void add(const geo::vector3<CoordT> &items_aabb,
           const geo::vector3<CoordT> &box_size, const ValueType &value) {
    auto key1 = key_for_vector(items_aabb);
    auto key2 = key_for_vector(box_size);
    auto sp_value = CachedValue{value, items_aabb, box_size};
    for (auto x = key1.x; x <= key2.x; ++x) {
      for (auto y = key1.y; y <= key2.y; ++y) {
        for (auto z = key1.z; z <= key2.z; ++z) {
          cells.insert({{x, y, z}, sp_value});
        }
      }
    }
  }

  std::optional<ValueType> try_get(const geo::vector3<CoordT> &box_size) const {
    auto key = key_for_vector(box_size);
    auto range = cells.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
      const auto &cached = it->second;
      if (cached.items_aabb <= box_size && box_size <= cached.box_size) {
        return cached.value;
      }
    }
    return std::nullopt;
  }

  struct CachedValue {
    ValueType value;
    geo::vector3<CoordT> items_aabb;
    geo::vector3<CoordT> box_size;
  };

  std::unordered_multimap<SpatialKey<CoordT>, CachedValue> cells;
  CoordT cell_size;
};

} // namespace pstack::calc

namespace std {
template <typename T> struct hash<pstack::calc::SpatialKey<T>> {
  std::size_t operator()(const pstack::calc::SpatialKey<T> &k) const {
    std::size_t h1 = std::hash<T>{}(k.x);
    std::size_t h2 = std::hash<T>{}(k.y);
    std::size_t h3 = std::hash<T>{}(k.z);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};
} // namespace std

#endif // PSTACK_CALC_SPATIAL_HASH_HPP
