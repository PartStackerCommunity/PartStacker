#ifndef PSTACK_GUI_PARTS_LIST_HPP
#define PSTACK_GUI_PARTS_LIST_HPP

#include "pstack/calc/part.hpp"
#include "pstack/gui/list_view.hpp"
#include <wx/stattext.h>
#include <memory>
#include <optional>
#include <string_view>

namespace pstack::gui {

class parts_list : public list_view {
public:
    parts_list() = default;
    void initialize(wxWindow* parent);

    // Non-copyable and non-movable, because of the bound callback
    parts_list(const parts_list&) = delete;
    parts_list& operator=(const parts_list&) = delete;

    void append(std::string mesh_file);
    void append(calc::part part);
    void change(std::string mesh_file, std::size_t row);
    void reload_file(std::size_t row);
    void reload_text(std::size_t row);
    void reload_all_text();
    void reload_quantity(std::size_t row);
    void delete_all();
    void delete_selected();
    calc::part& at(std::size_t row) {
        return *_parts.at(row);
    }
    std::vector<std::shared_ptr<const calc::part>> get_all() const;

    void update_label();
    wxWindow* label() const {
        return _label;
    }
    std::size_t total_parts() const {
        return _total_parts;
    }
    double total_volume() const {
        return _total_volume;
    }
    std::size_t total_triangles() const {
        return _total_triangles;
    }

    void show_extra(bool value) {
        _show_extra = value;
    }

private:
    std::vector<std::shared_ptr<calc::part>> _parts;
    bool _show_extra = false;

    wxStaticText* _label{};
    std::size_t _total_parts{};
    double _total_volume{};
    std::size_t _total_triangles{};
};

} // namespace pstack::gui

#endif // PSTACK_GUI_PARTS_LIST_HPP
