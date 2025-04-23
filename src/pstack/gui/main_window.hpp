#ifndef PSTACK_GUI_MAIN_WINDOW_HPP
#define PSTACK_GUI_MAIN_WINDOW_HPP

#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/string.h>
#include <memory>
#include <optional>
#include <vector>
#include "pstack/calc/stacker_thread.hpp"
#include "pstack/gui/controls.hpp"
#include "pstack/gui/parts_list.hpp"

namespace pstack::gui {

class viewport;

class main_window : public wxFrame {
public:
    main_window(const wxString& title);

private:
    viewport* _viewport = nullptr;
    controls _controls;

    void on_select_parts(const std::vector<std::size_t>& indices);
    void set_part(std::size_t index);
    void unset_part();
    parts_list _parts_list{};
    std::shared_ptr<calc::part> _current_part = nullptr;
    std::optional<std::size_t> _current_part_index = std::nullopt;
    void enable_part_settings(bool enable);

    void on_stacking(wxCommandEvent& event);
    void on_stacking_start();
    void on_stacking_stop();
    void on_stacking_success(calc::stack_result result, std::chrono::system_clock::duration elapsed);
    void enable_on_stacking(bool starting);
    std::optional<calc::stack_result> _last_result = {};
    calc::stacker_thread _stacker_thread;

    wxMenuBar* make_menu_bar();
    std::vector<wxMenuItem*> _disableable_menu_items;

    void bind_all_controls();
    void on_new(wxCommandEvent& event);
    void on_close(wxCloseEvent& event);
    void on_export_result(wxCommandEvent& event);
    void on_export_result();
    void on_import_part(wxCommandEvent& event);
    void on_delete_part(wxCommandEvent& event);
    void on_reload_part(wxCommandEvent& event);

    wxSizer* arrange_all_controls();
    wxSizer* arrange_part_buttons();
    wxSizer* arrange_bottom_section1();
    wxSizer* arrange_bottom_section2();
    wxNotebook* arrange_tabs();
    void arrange_tab_part_settings(wxPanel* panel);
    void arrange_tab_stack_settings(wxPanel* panel);
    void arrange_tab_results(wxPanel* panel);
};

} // namespace pstack::gui

#endif // PSTACK_GUI_MAIN_WINDOW_HPP
