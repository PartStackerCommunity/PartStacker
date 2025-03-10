#ifndef PSTACK_GUI_MAIN_WINDOW_HPP
#define PSTACK_GUI_MAIN_WINDOW_HPP

#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/gauge.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/string.h>
#include <optional>
#include <vector>
#include "pstack/calc/stacker_thread.hpp"
#include "pstack/gui/parts_list.hpp"

namespace pstack::gui {

class viewport;

class main_window : public wxFrame {
public:
    main_window(const wxString& title);
    void after_show();

private:
    viewport* _viewport = nullptr;

    void reset_fields();
    void select_parts(const std::vector<std::size_t>& indices);
    void set_part(std::size_t index);
    void unset_part();
    parts_list _parts_list{ this, wxSize(380, 240), &main_window::select_parts };
    part_properties* _current_part = nullptr;
    std::optional<std::size_t> _current_part_index = std::nullopt;
    void enable_part_settings(bool enable);
    wxSpinCtrl* _quantity_spinner = nullptr;
    wxSpinCtrl* _min_hole_spinner = nullptr;
    wxCheckBox* _minimize_checkbox = nullptr;
    wxRadioButton* _radio_none = nullptr;
    wxRadioButton* _radio_arbitrary = nullptr;
    wxRadioButton* _radio_cubic = nullptr;
    wxButton* _preview_button = nullptr;
    wxButton* _copy_button = nullptr;
    wxButton* _mirror_button = nullptr;

    wxButton* _import_button = nullptr;
    wxButton* _delete_button = nullptr;
    wxButton* _change_button = nullptr;
    wxButton* _reload_button = nullptr;

    wxSpinCtrlDouble* _clearance_spinner = nullptr;
    wxSpinCtrlDouble* _spacing_spinner = nullptr;
    wxSpinCtrlDouble* _thickness_spinner = nullptr;
    wxSpinCtrlDouble* _width_spinner = nullptr;
    wxCheckBox* _sinterbox_checkbox = nullptr;

    wxSpinCtrl* _initial_x_spinner = nullptr;
    wxSpinCtrl* _initial_y_spinner = nullptr;
    wxSpinCtrl* _initial_z_spinner = nullptr;
    wxSpinCtrl* _maximum_x_spinner = nullptr;
    wxSpinCtrl* _maximum_y_spinner = nullptr;
    wxSpinCtrl* _maximum_z_spinner = nullptr;

    void on_stacking(wxCommandEvent& event);
    void on_stacking_start();
    void on_stacking_stop();
    void on_stacking_success(calc::mesh mesh, std::chrono::duration<double> elapsed);
    void enable_on_stacking(bool starting);
    std::optional<calc::mesh> _last_result = {};
    wxSpinCtrlDouble* _min_clearance_spinner = nullptr;
    wxCheckBox* _section_view_checkbox = nullptr;
    wxButton* _export_button = nullptr;
    wxButton* _stack_button = nullptr;
    wxGauge* _progress_bar = nullptr;
    calc::stacker_thread _stacker_thread;

    wxMenuBar* make_menu_bar();
    std::vector<wxMenuItem*> _disableable_menu_items;

    wxSizer* make_part_buttons();
    wxSizer* make_bottom_section1();
    wxSizer* make_bottom_section2();

    wxWindow* make_tabs();
    void make_tab_part_settings(wxPanel* panel);
    void make_tab_sinterbox(wxPanel* panel);
    void make_tab_bounding_box(wxPanel* panel);

    void on_new(wxCommandEvent& event);
    void on_close(wxCloseEvent& event);
    void on_export(wxCommandEvent& event);
    void on_export();
    void on_import(wxCommandEvent& event);
    void on_delete(wxCommandEvent& event);
    void on_change(wxCommandEvent& event);
    void on_reload(wxCommandEvent& event);
};

} // namespace pstack::gui

#endif // PSTACK_GUI_MAIN_WINDOW_HPP
