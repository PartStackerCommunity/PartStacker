#include "pstack/gui/save.hpp"
#include <jsoncons/json.hpp>
#include <cstddef>
#include <expected>
#include <string_view>
#include <string>
#include <ranges>
#include <utility>
#include <vector>

namespace pstack::gui {
namespace {

struct save_part : calc::part_base {
    bool in_parts_list;
};

struct save_sinterbox_parameters : calc::sinterbox_settings, calc::sinterbox_bounding {};

using save_result = calc::stack_result_base<std::size_t, save_sinterbox_parameters>;

using internal_save_state = basic_save_state<save_part, save_result>;

} // namespace
} // namespace pstack::gui

JSONCONS_N_MEMBER_TRAITS(pstack::gui::preferences, 0, // Nothing is required
    invert_scroll, extra_parts, show_bounding_box
);
JSONCONS_N_MEMBER_TRAITS(pstack::calc::stack_settings, 0, // Nothing is required
    resolution, x_min, x_max, y_min, y_max, z_min, z_max
);
JSONCONS_N_MEMBER_TRAITS(pstack::calc::sinterbox_settings, 0, // Nothing is required
    clearance, thickness, width, spacing
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::save_part,
    mesh_file, quantity, mirrored, min_hole, rotation_index, rotate_min_box, in_parts_list
);

JSONCONS_ALL_MEMBER_TRAITS(pstack::geo::point3<float>,
    x, y, z
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::geo::vector3<float>,
    x, y, z
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::geo::matrix3<float>,
    xx, xy, xz, yx, yy, yz, zx, zy, zz
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::save_result::piece,
    part, rotation, translation
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::save_sinterbox_parameters,
    clearance, thickness, width, spacing, min, max
);
JSONCONS_N_MEMBER_TRAITS(pstack::gui::save_result, 1,
    pieces,     // mandatory
    sinterbox   // optional
);

JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::internal_save_state,
    pref, stack, sinterbox, parts, results
);

namespace pstack::gui {

namespace {

internal_save_state to_internal(const save_state& state) {
    std::vector all_parts = state.parts; // Copy

    std::vector<save_result> results{};
    for (const auto& in_result : state.results) {
        auto& out_result = results.emplace_back();
        if (in_result.sinterbox.has_value()) {
            out_result.sinterbox.emplace();
            static_cast<calc::sinterbox_settings&>(*out_result.sinterbox) = in_result.sinterbox->settings;
            static_cast<calc::sinterbox_bounding&>(*out_result.sinterbox) = in_result.sinterbox->bounding;
        }
        for (const auto& in_piece : in_result.pieces) {
            auto& out_piece = out_result.pieces.emplace_back();
            out_piece.rotation = in_piece.rotation;
            out_piece.translation = in_piece.translation;
            auto it = std::ranges::find(all_parts, in_piece.part);
            if (it != all_parts.end()) {
                out_piece.part = std::ranges::distance(all_parts.begin(), it);
            } else {
                out_piece.part = all_parts.size();
                all_parts.push_back(in_piece.part);
            }
        }
    }

    std::vector<save_part> parts{};
    for (std::size_t i = 0; i != all_parts.size(); ++i) {
        const calc::part_base& part = *(all_parts[i]);
        parts.emplace_back(part);
        parts.back().in_parts_list = (i < state.parts.size());
    }

    return internal_save_state{
        .pref = state.pref,
        .stack = state.stack,
        .sinterbox = state.sinterbox,
        .parts = std::move(parts),
        .results = std::move(results),
    };
};

} // namespace

std::expected<save_state, std::string> save_state_from_json(std::string_view str) {
    return {};
}

std::string save_state_to_json(const save_state& state) {
    return jsoncons::json(to_internal(state)).to_string();
}

} // namespace pstack::gui
