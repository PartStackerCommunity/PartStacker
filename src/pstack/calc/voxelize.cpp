#include "pstack/calc/bool.hpp"
#include "pstack/calc/voxelize.hpp"
#include "pstack/util/mdarray.hpp"
#include <algorithm>
#include <cmath>
#include <span>
#include <stack>
#include <vector>

namespace pstack::calc {

int voxelize(const mesh& mesh, const util::mdspan<int, 3> voxels, const int index, const std::size_t carver_size) {
    util::mdarray<Bool, 3> actual_triangles(voxels.extents());
    util::mdarray<Bool, 3> visited(voxels.extents());
    util::mdarray<Bool, 3> carved(voxels.extents());

    // First render each part, placing voxels at the position of each triangle
    for (const geo::triangle& t : mesh.triangles()) {
        const geo::vector3 a = t.v1.as_vector();
        const geo::vector3 b = t.v2 - t.v1;
        const geo::vector3 c = t.v3 - t.v1;

        // Approximate the step size
        const auto db = 0.1 / std::max({ std::abs(b.x), std::abs(b.y), std::abs(b.z) });
        const auto dc = 0.1 / std::max({ std::abs(c.x), std::abs(c.y), std::abs(c.z) });

        // Voxelize
        for (float p = 0; p < 1; p += db) {
            for (float q = 0; q < 1 - p; q += dc) {
                const geo::vector3 pos = a + p * b + q * c;
                // Round to the nearest integer
                // This is a hot path, so `std::round()`, `std::nearbyint()`, `std::floor()` are much slower
                const auto x = static_cast<std::size_t>(pos.x + 0.5f);
                const auto y = static_cast<std::size_t>(pos.y + 0.5f);
                const auto z = static_cast<std::size_t>(pos.z + 0.5f);
                actual_triangles[x, y, z] = true;
            }
        }
    }

    if (carver_size > 0) {
        std::stack<geo::point3<int>> stack{};

        // Carving step
        for (int x = 0; x < voxels.extent(0); ++x) {
            for (int y = 0; y < voxels.extent(1); ++y) {
                for (int z = 0; z < voxels.extent(2); ++z) {
                    if (x == 0 || y == 0 || z == 0 || x == voxels.extent(0) - carver_size || y == voxels.extent(1) - carver_size || z == voxels.extent(2) - carver_size) {
#if defined(__cpp_aggregate_paren_init) and __cpp_aggregate_paren_init >= 201902L
                        stack.emplace(x, y, z);
#else
                        stack.push(geo::point3<int>{x, y, z});
#endif
                    }
                }
            }
        }

        while (not stack.empty()) {
            const auto [x, y, z] = stack.top();
            stack.pop();

            // Check if we need to do work here
            if (x < 0 || y < 0 || z < 0 || x > voxels.extent(0) - carver_size || y > voxels.extent(1) - carver_size || z > voxels.extent(2) - carver_size || visited[x, y, z]) {
                continue;
            }
            visited[x, y, z] = true;

            const bool good = [&] {
                for (std::size_t i = 0; i < carver_size; ++i) {
                    for (std::size_t j = 0; j < carver_size; ++j) {
                        for (std::size_t k = 0; k < carver_size; ++k) {
                            if (actual_triangles[x + i, y + j, z + k]) {
                                return false;
                            }
                        }
                    }
                }
                return true;
            }();
            if (not good) {
                continue;
            }

            for (std::size_t i = 0; i < carver_size; ++i) {
                for (std::size_t j = 0; j < carver_size; ++j) {
                    for (std::size_t k = 0; k < carver_size; ++k) {
                        carved[x + i, y + j, z + k] = true;
                    }
                }
            }

#if defined(__cpp_aggregate_paren_init) and __cpp_aggregate_paren_init >= 201902L
            stack.emplace(x - 1, y, z);
            stack.emplace(x + 1, y, z);

            stack.emplace(x, y - 1, z);
            stack.emplace(x, y + 1, z);

            stack.emplace(x, y, z - 1);
            stack.emplace(x, y, z + 1);
#else
            stack.push(geo::point3<int>{x - 1, y, z});
            stack.push(geo::point3<int>{x + 1, y, z});

            stack.push(geo::point3<int>{x, y - 1, z});
            stack.push(geo::point3<int>{x, y + 1, z});

            stack.push(geo::point3<int>{x, y, z - 1});
            stack.push(geo::point3<int>{x, y, z + 1});
#endif
        }

        // #region convexivy

        // Make convex in z-direction
        for (int x = 0; x < voxels.extent(0); ++x) {
            for (int y = 0; y < voxels.extent(1); ++y) {
                int minV = std::numeric_limits<int>::max();
                int maxV = std::numeric_limits<int>::min();

                for (int z = 0; z < voxels.extent(2); ++z) {
                    if (actual_triangles[x, y, z]) {
                        minV = std::min(z, minV);
                        maxV = std::max(z, maxV);
                    }
                }

                for (int z = minV; z < maxV; z++) {
                    actual_triangles[x, y, z] = true;
                }
            }
        }

        // Make convex in y-direction
        for (int x = 0; x < voxels.extent(0); ++x) {
            for (int z = 0; z < voxels.extent(2); ++z) {
                int minV = std::numeric_limits<int>::max();
                int maxV = std::numeric_limits<int>::min();

                for (int y = 0; y < voxels.extent(1); ++y) {
                    if (actual_triangles[x, y, z]) {
                        minV = std::min(y, minV);
                        maxV = std::max(y, maxV);
                    }
                }

                for (int y = minV; y < maxV; y++) {
                    actual_triangles[x, y, z] = true;
                }
            }
        }

        // Make convex in x-direction
        for (int z = 0; z < voxels.extent(2); ++z) {
            for (int y = 0; y < voxels.extent(1); ++y) {
                int minV = std::numeric_limits<int>::max();
                int maxV = std::numeric_limits<int>::min();

                for (int x = 0; x < voxels.extent(0); ++x) {
                    if (actual_triangles[x, y, z]) {
                        minV = std::min(x, minV);
                        maxV = std::max(x, maxV);
                    }
                }

                for (int x = minV; x < maxV; x++) {
                    actual_triangles[x, y, z] = true;
                }
            }
        }

        // #endregion
    }

    // Expand by one voxel in all directions
    for (std::size_t x = 0; x < voxels.extent(0) - 1; ++x) {
        for (std::size_t y = 0; y < voxels.extent(1) - 1; ++y) {
            for (std::size_t z = 0; z < voxels.extent(2) - 1; ++z) {
                if (not carved[x, y, z] and actual_triangles[x, y, z]) {
#if defined(MDSPAN_USE_BRACKET_OPERATOR) and MDSPAN_USE_BRACKET_OPERATOR == 0
                    voxels(x + 1, y + 1, z + 1) |= index;
                    voxels(x + 1, y + 1, z) |= index;
                    voxels(x + 1, y, z + 1) |= index;
                    voxels(x + 1, y, z) |= index;
                    voxels(x, y + 1, z) |= index;
                    voxels(x, y + 1, z + 1) |= index;
                    voxels(x, y, z + 1) |= index;
#else
                    voxels[x + 1, y + 1, z + 1] |= index;
                    voxels[x + 1, y + 1, z] |= index;
                    voxels[x + 1, y, z + 1] |= index;
                    voxels[x + 1, y, z] |= index;
                    voxels[x, y + 1, z] |= index;
                    voxels[x, y + 1, z + 1] |= index;
                    voxels[x, y, z + 1] |= index;
#endif
                }
            }
        }
    }

    // Calculate and return volume by counting the voxels
    const std::span<const int> all_voxels{ voxels.data_handle(), voxels.size() };
    return std::count_if(all_voxels.begin(), all_voxels.end(), [index](int i) {
        return (i & index) != 0;
    });
}

} // namespace pstack::calc
