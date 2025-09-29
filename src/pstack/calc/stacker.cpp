#include "pstack/calc/bool.hpp"
#include "pstack/calc/mesh.hpp"
#include "pstack/calc/rotations.hpp"
#include "pstack/calc/stacker.hpp"
#include "pstack/calc/voxelize.hpp"
#include "pstack/util/mdarray.hpp"
#include <algorithm>
#include <optional>
#include <ranges>

namespace pstack::calc {

namespace {

struct stack_state {
    struct mesh_entry {
        mesh mesh;
        geo::vector3<int> box_size;
        stack_result::piece piece;
    };

    std::vector<std::vector<mesh_entry>> meshes;
    std::vector<util::mdarray<int, 3>> voxels;
    util::mdarray<Bool, 3> space;
    std::vector<std::shared_ptr<const part>> ordered_parts;
    stack_result result;
    std::size_t total_parts;
    std::size_t total_placed;
};

void place(const util::mdspan<Bool, 3> space, const int index, const util::mdspan<const int, 3> obj, const int x, const int y, const int z) {
    const int max_i = std::min(x + obj.extent(0), space.extent(0));
    const int max_j = std::min(y + obj.extent(1), space.extent(1));
    const int max_k = std::min(z + obj.extent(2), space.extent(2));
    for (int i = x; i < max_i; ++i) {
        for (int j = y; j < max_j; ++j) {
            for (int k = z; k < max_k; ++k) {
#if defined(MDSPAN_USE_BRACKET_OPERATOR) and MDSPAN_USE_BRACKET_OPERATOR == 0
                space(i, j, k) |= (obj(i - x, j - y, k - z) & index) != 0;
#else
                space[i, j, k] |= (obj[i - x, j - y, k - z] & index) != 0;
#endif
            }
        }
    }
}

int can_place(const util::mdspan<const Bool, 3> space, int possible, const util::mdspan<const int, 3> obj, const std::size_t x, const std::size_t y, const std::size_t z) {
    const std::size_t max_i = std::min(x + obj.extent(0), space.extent(0));
    const std::size_t max_j = std::min(y + obj.extent(1), space.extent(1));
    const std::size_t max_k = std::min(z + obj.extent(2), space.extent(2));
    for (std::size_t i = x; i < max_i; ++i) {
        for (std::size_t j = y; j < max_j; ++j) {
            for (std::size_t k = z; k < max_k; ++k) {
#if defined(MDSPAN_USE_BRACKET_OPERATOR) and MDSPAN_USE_BRACKET_OPERATOR == 0
                if (space(i, j, k)) {
                    possible &= (possible ^ obj(i - x, j - y, k - z));
#else
                if (space[i, j, k]) {
                    possible &= (possible ^ obj[i - x, j - y, k - z]);
#endif
                    if (possible == 0) {
                        return 0;
                    }
                }
            }
        }
    }
    return possible;
}

std::size_t try_place(const stack_parameters& params, stack_state& state, const std::size_t part_index, const std::size_t to_place, const geo::point3<int> max) {
    std::size_t placed = 0;
    for (int s = 0; s <= max.x + max.y + max.z; ++s) {
        for (int r = std::max(0, s - max.z); r <= std::min(s, max.x + max.y); ++r) {
            const int z = s - r;
            for (int x = std::max(0, r - max.y); x <= std::min(r, max.x); ++x) {
                const int y = r - x;

                // Calculate which orientations fit in bounding box
                int bit_index = 1;
                int possible = 0;
                for (const auto& [mesh, box_size, piece] : state.meshes[part_index]) {
                    if (x + box_size.x < max.x && y + box_size.y < max.y && z + box_size.z < max.z) {
                        possible |= bit_index;
                    }
                    bit_index *= 2;
                }

                possible = can_place(state.space, possible, state.voxels[part_index], x, y, z);

                if (possible != 0) { // If it fits, figure out which rotation to use
                    bit_index = 1;
                    for (const auto& [mesh, box_size, piece] : state.meshes[part_index]) {
                        if ((possible & bit_index) == 0) {
                            bit_index *= 2;
                            continue;
                        } else {
                            const geo::vector3<float> translation = { (float)x, (float)y, (float)z };
                            state.result.mesh.add(mesh, translation);
                            auto& new_piece = state.result.pieces.emplace_back(piece);
                            new_piece.translation += translation;
                            place(state.space, bit_index, state.voxels[part_index], x, y, z); // Mark voxels as occupied
                            ++placed;
                            ++state.total_placed;
                            params.set_progress(state.total_placed, state.total_parts);
                            params.display_mesh(state.result.mesh, max);
                            if (to_place == placed) { // All instances of this part placed, move to next part
                                return placed;
                            } else { // Move to next instance of part
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Reached the end of the box, return the part we're currently at.
    return placed;
}

std::optional<stack_result> stack_impl(const stack_parameters& params, const std::atomic<bool>& running) {
    stack_state state{};
    state.ordered_parts = params.parts;
    std::ranges::sort(state.ordered_parts, std::greater{}, &part::volume);
    state.meshes.assign(state.ordered_parts.size(), {});
    state.voxels.assign(state.ordered_parts.size(), {});

    double triangles = 0;
    const double scale_factor = 1 / params.settings.resolution;
    state.total_parts = 0;
    state.total_placed = 0;
    for (const std::shared_ptr<const part> part : state.ordered_parts) {
        triangles += part->triangle_count * rotation_sets[part->rotation_index].size();
        state.total_parts += part->quantity;
    }

    double progress = 0;
    for (int i = 0; i < state.ordered_parts.size(); ++i) {
        geo::matrix3 base_rotation = geo::eye3<float>;

        if (state.ordered_parts[i]->rotate_min_box) {
            auto reduced_view = state.ordered_parts[i]->mesh.triangles()
                              | std::views::filter([i = 0](auto&&) mutable { return i++ % 16 == 0; });
            mesh reduced_mesh{ std::vector<geo::triangle>(reduced_view.begin(), reduced_view.end()) };

            static constexpr std::size_t sections = 20;
            static constexpr double angle_diff = 2 * geo::pi / sections;

            static constexpr geo::matrix3 rot_x = geo::rot3_x<float>(angle_diff);
            static constexpr geo::matrix3 rot_y = geo::rot3_y<float>(angle_diff);

            int min_box_volume = std::numeric_limits<int>::max();
            double best_x = 0;
            double best_y = 0;

            for (double x = 0; x < 2 * geo::pi; x += angle_diff) {
                reduced_mesh.rotate(rot_x);
                for (double y = 0; y < 2 * geo::pi; y += angle_diff) {
                    reduced_mesh.rotate(rot_y);
                    const auto box = reduced_mesh.bounding().box_size;
                    const int box_volume = box.x * box.y * box.z;
                    if (box_volume < min_box_volume) {
                        min_box_volume = box_volume;
                        best_x = x;
                        best_y = y;
                    }
                }
            }

            base_rotation = geo::rot3_y<float>(best_y) * geo::rot3_x<float>(best_x);
        }

        // Set up array of parts
        const auto rotations = rotation_sets[state.ordered_parts[i]->rotation_index];
        state.meshes[i].reserve(rotations.size());

        // Track bounding box size
        geo::vector3<int> max_box_size = { 1, 1, 1 };

        // Calculate all the rotations
        for (const auto& rotation : rotations) {
            if (not running) {
                return std::nullopt;
            }

            const std::shared_ptr<const part> part = state.ordered_parts[i];
            mesh m = part->mesh;
            m.scale(scale_factor);
            auto total_rotation = base_rotation * rotation;
            m.rotate(total_rotation);
            auto offset = m.set_baseline({ 0, 0, 0 });

            const auto box_size = m.bounding().box_size;
            max_box_size.x = std::max(box_size.x, max_box_size.x);
            max_box_size.y = std::max(box_size.y, max_box_size.y);
            max_box_size.z = std::max(box_size.z, max_box_size.z);

            stack_result::piece piece = { .part = part, .rotation = total_rotation, .translation = offset };
#if defined(__cpp_aggregate_paren_init) and __cpp_aggregate_paren_init >= 201902L
            state.meshes[i].emplace_back(std::move(m), box_size, std::move(piece));
#else
            state.meshes[i].push_back(stack_state::mesh_entry{std::move(m), box_size, std::move(piece)});
#endif

            progress += part->triangle_count / 2;
            params.set_progress(progress, triangles);
        }

        // Initialize space size to appropriate dimensions
        state.voxels[i] = { max_box_size.x, max_box_size.y, max_box_size.z };

        // Voxelize each rotated instance of this part
        int bit_index = 1;
        for (const auto& [mesh, box_size, piece] : state.meshes[i]) {
            if (not running) {
                return std::nullopt;
            }

            voxelize(mesh, state.voxels[i], bit_index, state.ordered_parts[i]->min_hole);
            bit_index *= 2;

            progress += state.ordered_parts[i]->triangle_count / 2;
            params.set_progress(progress, triangles);
        }
    }

    int max_x = static_cast<int>(scale_factor * params.settings.x_min);
    int max_y = static_cast<int>(scale_factor * params.settings.y_min);
    int max_z = static_cast<int>(scale_factor * params.settings.z_min);
    state.space = {
        std::max(max_x, static_cast<int>(scale_factor * params.settings.x_max)),
        std::max(max_y, static_cast<int>(scale_factor * params.settings.y_max)),
        std::max(max_z, static_cast<int>(scale_factor * params.settings.z_max))
    };

    params.set_progress(0, 1);

    for (std::size_t part_index = 0; part_index != state.ordered_parts.size(); ++part_index) {
        std::size_t to_place = state.ordered_parts[part_index]->quantity;
        while (to_place > 0) {
            if (not running) {
                return std::nullopt;
            }
            const std::size_t placed = try_place(params, state, part_index, to_place, { max_x, max_y, max_z });
            to_place -= placed;

            // If we have not placed a part, it means there are no more ways to place an instance of the current part in the box: it must be enlarged
            if (placed == 0) {
                int best = std::numeric_limits<int>::max();
                int new_x = state.space.extent(0);
                int new_y = state.space.extent(1);
                int new_z = state.space.extent(2);

                int min_box_x = std::numeric_limits<int>::max();
                int min_box_y = std::numeric_limits<int>::max();
                int min_box_z = std::numeric_limits<int>::max();
                for (const auto& [mesh, box_size, piece] : state.meshes[part_index]) {
                    min_box_x = std::min(box_size.x, min_box_x);
                    min_box_y = std::min(box_size.y, min_box_y);
                    min_box_z = std::min(box_size.z, min_box_z);
                }

                for (int s = 0; s < state.space.extent(0) + state.space.extent(1) + state.space.extent(2) - min_box_x - min_box_y - min_box_z; ++s) {
                    for (int r = std::max<std::size_t>(0, s - state.space.extent(2) - min_box_z); r <= std::min<std::size_t>(s, state.space.extent(0) + state.space.extent(1) - min_box_x - min_box_y); ++r) {
                        const int z = s - r;
                        if (std::max(z + min_box_z, max_z) * max_y * max_x > best) {
                            break;
                        }

                        for (int x = std::max<std::size_t>(0, r - state.space.extent(1) - min_box_y); x <= std::min<std::size_t>(r, state.space.extent(0) - min_box_z); ++x) {
                            const int y = r - x;
                            if (std::max(x + min_box_x, max_x) * std::max(y + min_box_y, max_y) * std::max(z + min_box_z, max_z) > best) {
                                continue;
                            }

                            // Calculate which orientations fit in bounding box
                            int bit_index = 1;
                            int possible = 0;
                            for (const auto& [mesh, box_size, piece] : state.meshes[part_index]) {
                                if (x + box_size.x < state.space.extent(0) && y + box_size.y < state.space.extent(1) && z + box_size.z < state.space.extent(2)) {
                                    possible |= bit_index;
                                }
                                bit_index *= 2;
                            }

                            possible = can_place(state.space, possible, state.voxels[part_index], x, y, z);

                            if (possible != 0) { // If it fits, figure out which rotation to use
                                bit_index = 1;
                                for (const auto& [mesh, box_size, piece] : state.meshes[part_index]) {
                                    if ((possible & bit_index) != 0) {
                                        const int new_box = std::max(max_x, x + box_size.x) * std::max(max_y, y + box_size.y) * std::max(max_z, z + box_size.z);
                                        if (new_box < best) {
                                            best = new_box;
                                            new_x = x + box_size.x;
                                            new_y = y + box_size.y;
                                            new_z = z + box_size.z;
                                        }
                                    }
                                    bit_index *= 2;
                                }
                            }
                        }
                    }
                }

                if (best == std::numeric_limits<int>::max()) {
                    return stack_result{};
                }

                max_x = std::max(max_x, new_x + 2);
                max_y = std::max(max_y, new_y + 2);
                max_z = std::max(max_z, new_z + 2);
            }
        }
    }

    state.result.mesh.scale(1 / scale_factor);
    return { std::move(state.result) };
}

} // namespace

void stacker::stack(const stack_parameters params) {
    if (_running.exchange(true)) {
        return;
    }
    const auto start = std::chrono::system_clock::now();
    std::optional<stack_result> result = stack_impl(params, _running);
    const auto elapsed = std::chrono::system_clock::now() - start;
    if (result.has_value()) {
        if (result->pieces.empty()) {
            params.on_failure();
        } else {
            params.on_success(std::move(*result), elapsed);
        }
    }
    params.on_finish();
    _running = false;
}

} // namespace pstack::calc
