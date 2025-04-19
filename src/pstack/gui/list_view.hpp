#ifndef PSTACK_GUI_LIST_VIEW_HPP
#define PSTACK_GUI_LIST_VIEW_HPP

#include "pstack/calc/mesh.hpp"
#include <wx/listctrl.h>
#include <wx/string.h>
#include <utility>
#include <vector>

namespace pstack::gui {

class main_window;

class list_view {
public:
    list_view() = default;
    list_view(main_window* parent, wxSize min_size, const std::vector<std::pair<wxString, int>>& columns);

    void append(std::vector<wxString> items);
    void replace(std::size_t row_index, std::vector<wxString> items);
    void delete_row(std::size_t row_index);
    void set_text(std::size_t row_index, int column, const wxString& text);
    std::size_t rows() const {
        return _rows;
    }

    wxWindow* control() const {
        return _list;
    }

    template <class... Args>
    void bind(Args&&... args) {
        _list->Bind(std::forward<Args>(args)...);
    }

private:
    wxListView* _list = nullptr;
    std::size_t _rows = 0;
    std::size_t _columns = 0;
};

} // namespace pstack::gui

#endif // PSTACK_GUI_LIST_VIEW_HPP
