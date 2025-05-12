#include "pstack/calc/binpack.hpp"
#include "pstack/calc/binpack_cache.hpp"

namespace pstack::calc::binpack {

PackingResult combine(const std::array<PackingResult, 3> &results,
                      const ItemPlacement &placement) {
  PackingResult combined;
  combined.total_score = results[0].total_score + results[1].total_score +
                         results[2].total_score + placement.item.score;

  size_t total_placements = results[0].placements.size() +
                            results[1].placements.size() +
                            results[2].placements.size() + 1;
  combined.placements.reserve(total_placements);

  combined.placements.push_back(placement); // Add the current item first

  for (const auto &result : results) {
    std::copy(result.placements.begin(), result.placements.end(),
              std::back_inserter(combined.placements));
  }

  return combined;
}

PackingResult pack_items_impl(const std::vector<Item> &items,
                              const PackingBox &into_box,
                              BinPackCache<int, PackingResult> &cache,
                              const geo::vector3<int> &min_volume) {
  auto box_size = into_box.size;

  if (!fits(sort(box_size), min_volume)) {
    return {};
  }

  if (auto cached = cache.try_get(box_size); cached) {
    return *cached;
  }

  PackingResult best;

  for (auto &item : items) {
    auto [sX, sY, sZ] = item.size;
    if (sX > box_size.x || sY > box_size.y || sZ > box_size.z) {
      continue;
    }
    // We test all 6 possible ways of splitting the box
    std::array<std::array<PackingBox, 3>, 6> boxes;
    auto offsetX = PackingBox::Offset<geo::Axis::X>{sX};
    auto offsetY = PackingBox::Offset<geo::Axis::Y>{sY};
    auto offsetZ = PackingBox::Offset<geo::Axis::Z>{sZ};

    boxes[0] = into_box.split(offsetX, offsetY, offsetZ);
    boxes[1] = into_box.split(offsetX, offsetZ, offsetY);
    boxes[2] = into_box.split(offsetY, offsetX, offsetZ);
    boxes[3] = into_box.split(offsetY, offsetZ, offsetX);
    boxes[4] = into_box.split(offsetZ, offsetX, offsetY);
    boxes[5] = into_box.split(offsetZ, offsetY, offsetX);

    for (const auto &box : boxes) {
      std::array<PackingResult, 3> results;
      int score = item.score;
      for (int j = 0; j < 3; j++) {
        results[j] = pack_items_impl(items, box[j], cache, min_volume);
        score += results[j].total_score;
      }
      if (score > best.total_score) {
        for (int j = 0; j < 3; j++) {
          results[j].translate(box[j].origin);
        }
        best = combine(results, {item, into_box.origin});
      }
    }
  }

  best.translate(-into_box.origin);
  geo::vector3<int> aabb = {0, 0, 0};
  for (auto &p : best.placements) {
    aabb = geo::component_max(aabb, p.position + p.item.size);
  }
  cache.add(aabb, box_size, best);
  return best;
}

PackingResult pack_items(const std::vector<Item> &items,
                         const PackingBox &initial_box) {
  if (items.empty()) {
    return {};
  }

  BinPackCache<int, PackingResult> cache(1000);

  geo::vector3<int> min_volume{std::numeric_limits<int>::max(),
                               std::numeric_limits<int>::max(),
                               std::numeric_limits<int>::max()};

  for (const auto &item : items) {
    min_volume = geo::component_min(min_volume, geo::sort(item.size));
  }

  return pack_items_impl(items, initial_box, cache, min_volume);
}

} // namespace pstack::calc::binpack
