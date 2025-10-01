#include "pstack/gui/save.hpp"
#include <expected>
#include <string_view>
#include <string>

namespace pstack::gui {

std::expected<save_state, std::string> save_state_from_json(std::string_view str) {
    return {};
}

std::string save_state_to_json(const save_state& state) {
    return {};
}

} // namespace pstack::gui
