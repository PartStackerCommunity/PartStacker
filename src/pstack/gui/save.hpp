#ifndef PSTACK_GUI_SAVE_HPP
#define PSTACK_GUI_SAVE_HPP

#include "pstack/calc/part.hpp"
#include "pstack/calc/stacker.hpp"
#include "pstack/gui/preferences.hpp"
#include <expected>
#include <memory>
#include <string_view>
#include <string>
#include <vector>

namespace pstack::gui {

template <class PartType, class ResultType>
struct basic_save_state {
    preferences pref;
    calc::stack_settings stack;
    calc::sinterbox_settings sinterbox;
    std::vector<PartType> parts;
    std::vector<ResultType> results;
};

using save_state = basic_save_state<
    std::shared_ptr<const calc::part>,
    calc::stack_result
>;

std::expected<save_state, std::string> save_state_from_json(std::string_view str);
std::string save_state_to_json(const save_state& state);

} // namespace pstack::gui

#endif // PSTACK_GUI_SAVE_HPP
