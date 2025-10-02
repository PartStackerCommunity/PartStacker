#ifndef PSTACK_GUI_RESULTS_LIST_HPP
#define PSTACK_GUI_RESULTS_LIST_HPP

#include "pstack/calc/stacker.hpp"
#include "pstack/gui/list_view.hpp"

namespace pstack::gui {

class results_list : public list_view {
public:
    results_list() = default;
    void initialize(wxWindow* parent);

    // Non-copyable and non-movable, because of the bound callback
    results_list(const results_list&) = delete;
    results_list& operator=(const results_list&) = delete;

    void append(calc::stack_result result);
    void delete_all();
    void delete_selected();

    calc::stack_result& at(std::size_t row) {
        return _results.at(row);
    }
    const std::vector<calc::stack_result>& get_all() const {
        return _results;
    }

private:
    std::vector<calc::stack_result> _results;
};

} // namespace pstack::gui

#endif // PSTACK_GUI_RESULTS_LIST_HPP
