#include "pstack/gui/save.hpp"
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
    return {};
}

} // namespace pstack::gui
