#include "pstack/files/stl.hpp"
#include "pstack/gui/main_window.hpp"
#include "pstack/gui/parts_list.hpp"
#include "pstack/gui/viewport.hpp"

#include <cstdlib>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/colourdata.h>
#include <wx/filedlg.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>

namespace pstack::gui {

constexpr int outside_border = 20;
constexpr int tab_padding = 7;
constexpr int inner_border = 5;
constexpr int button_height = 25;
const wxSize button_size = wxSize(-1, button_height);
#ifdef _WIN32
const wxColour background_colour = wxColour(0xF0, 0xF0, 0xF0);
#endif

main_window::main_window(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
{
#ifdef _WIN32
    SetBackgroundColour(background_colour);
#endif

    Bind(wxEVT_CLOSE_WINDOW, &main_window::on_close, this);
    SetMenuBar(make_menu_bar());

    wxGLAttributes attrs;
    attrs.PlatformDefaults().Defaults().EndList();
    if (not wxGLCanvas::IsDisplaySupported(attrs)) {
        wxMessageBox("wxGLCanvas::IsDisplaySupported() returned false", "PartStacker fatal error", wxICON_ERROR);
        std::exit(EXIT_FAILURE);
    }
    _viewport = new viewport(this, attrs);
    _viewport->SetMinSize(FromDIP(wxSize(640, -1)));

    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(_viewport, 1, wxEXPAND);

    _parts_list.update_label();

    auto right_sizer = new wxBoxSizer(wxVERTICAL);
    right_sizer->Add(_parts_list.control(), 1, wxEXPAND);
    right_sizer->AddSpacer(FromDIP(inner_border));
    right_sizer->Add(make_part_buttons(), 0, wxEXPAND);
    right_sizer->AddSpacer(FromDIP(inner_border));
    right_sizer->Add(_parts_list.label());
    right_sizer->AddSpacer(FromDIP(inner_border));
    right_sizer->Add(make_tabs(), 0, wxEXPAND);
    right_sizer->AddSpacer(FromDIP(inner_border));
    right_sizer->Add(make_bottom_section1(), 0, wxEXPAND);
    right_sizer->AddSpacer(FromDIP(inner_border));
    right_sizer->Add(make_bottom_section2(), 0, wxEXPAND);
    reset_fields();

    sizer->Add(right_sizer, 0, wxEXPAND | wxALL, FromDIP(outside_border));
    SetSizerAndFit(sizer);
}

void main_window::after_show() {
    _viewport->on_move_by({0, 0});
    _viewport->render();
}

void main_window::reset_fields() {
    _quantity_spinner->SetValue(1);
    _min_hole_spinner->SetValue(1);
    _minimize_checkbox->SetValue(false);
    _radio_none->SetValue(false);
    _radio_cubic->SetValue(false);
    _radio_arbitrary->SetValue(false);

    _clearance_spinner->SetValue(0.8);
    _spacing_spinner->SetValue(6.0);
    _thickness_spinner->SetValue(0.8);
    _width_spinner->SetValue(1.1);
    _sinterbox_checkbox->SetValue(true);

    _initial_x_spinner->SetValue(150);
    _initial_y_spinner->SetValue(150);
    _initial_z_spinner->SetValue(30);
    _maximum_x_spinner->SetValue(156);
    _maximum_y_spinner->SetValue(156);
    _maximum_z_spinner->SetValue(90);

    _min_clearance_spinner->SetValue(1);
}

void main_window::select_parts(const std::vector<std::size_t>& indices) {
    const auto size = indices.size();
    _delete_button->Enable(size != 0);
    _change_button->Enable(size == 1);
    _reload_button->Enable(size != 0);
    if (size == 1) {
        set_part(indices[0]);
    } else {
        unset_part();
    }
}

void main_window::set_part(const std::size_t index) {
    enable_part_settings(true);
    _current_part = &_parts_list.at(index);
    _current_part_index.emplace(index);
    _quantity_spinner->SetValue(_current_part->quantity);
    _min_hole_spinner->SetValue(_current_part->min_hole);
    _minimize_checkbox->SetValue(_current_part->rotate_min_box);
    switch (_current_part->rotation_index) {
        case 0: {
            _radio_none->SetValue(true);
            _radio_cubic->SetValue(false);
            _radio_arbitrary->SetValue(false);
            break;
        }
        case 1: {
            _radio_cubic->SetValue(true);
            _radio_none->SetValue(false);
            _radio_arbitrary->SetValue(false);
            break;
        }
        case 2: {
            _radio_arbitrary->SetValue(true);
            _radio_none->SetValue(false);
            _radio_cubic->SetValue(false);
            break;
        }
#if defined(__cpp_lib_unreachable) and __cpp_lib_unreachable >= 202202L
        default: std::unreachable();
#else
        default: __builtin_unreachable();
#endif
    }
    _viewport->set_mesh(_current_part->mesh, _current_part->centroid);
}

void main_window::unset_part() {
    enable_part_settings(false);
    _current_part = nullptr;
    _current_part_index = std::nullopt;
    return;
}

void main_window::enable_part_settings(bool enable) {
    _quantity_spinner->Enable(enable);
    _min_hole_spinner->Enable(enable);
    _minimize_checkbox->Enable(enable);
    _radio_none->Enable(enable);
    _radio_arbitrary->Enable(enable);
    _radio_cubic->Enable(enable);
    _preview_button->Enable(enable);
    _copy_button->Enable(enable);
    _mirror_button->Enable(enable);
}

void main_window::on_stacking(wxCommandEvent& event) {
    if (_stack_button->GetLabelText() == "Stop") {
        on_stacking_stop();
    } else {
        on_stacking_start();
    }
    event.Skip();
}

void main_window::on_stacking_start() {
    const auto triangles = _parts_list.total_triangles();
    if (triangles == 0) {
        return;
    } else if (triangles > 1'000'000) {
        if (wxMessageBox("The finished model will exceed 1,000,000 triangles. Continue stacking?", "Warning", wxYES_NO | wxYES_DEFAULT | wxICON_INFORMATION) != wxYES) {
            return;
        }
    }

    calc::stacker_parameters params {
        .parts = _parts_list.get_all(),

        .set_progress = [this](double progress, double total) {
            CallAfter([=] {
                _progress_bar->SetValue(static_cast<int>(100 * progress / total));
            });
        },
        .display_mesh = [this](const calc::mesh& mesh, int max_x, int max_y, int max_z) {
            CallAfter([=] {
                // Make a copy of `mesh`, otherwise we encounter a data race
                _viewport->set_mesh(mesh, { max_x / 2.0f, max_y / 2.0f, max_z / 2.0f });
            });
        },
        .on_success = [this](calc::mesh mesh, const std::chrono::duration<double> elapsed) {
            CallAfter([=, mesh = std::move(mesh)] {
                on_stacking_success(std::move(mesh), elapsed);
                _export_button->Enable();
            });
        },
        .on_failure = [this] {
            CallAfter([=] {
                _export_button->Disable();
                _last_result.reset();
                wxMessageBox("Could not stack parts within maximum bounding box", "Stacking failed");
            });
        },
        .on_finish = [this] {
            CallAfter([=] {
                _stacker_thread.stop();
                enable_on_stacking(false);
            });
        },

        .resolution = _min_clearance_spinner->GetValue(),
        .x_min = _initial_x_spinner->GetValue(), .x_max = _maximum_x_spinner->GetValue(),
        .y_min = _initial_y_spinner->GetValue(), .y_max = _maximum_y_spinner->GetValue(),
        .z_min = _initial_z_spinner->GetValue(), .z_max = _maximum_z_spinner->GetValue(),
    };
    enable_on_stacking(true);
    _stacker_thread.start(std::move(params));
}

void main_window::on_stacking_stop() {
    if (wxMessageBox("Abort stacking?", "Warning", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) == wxYES) {
        _stacker_thread.stop();
    }
}

void main_window::on_stacking_success(calc::mesh mesh, const std::chrono::duration<double> elapsed) {
    _last_result.emplace(std::move(mesh));
    auto bounding = _last_result->bounding();
    if (_sinterbox_checkbox->GetValue()) {
        const double offset = _thickness_spinner->GetValue() + _clearance_spinner->GetValue();
        _last_result->set_baseline(geo::origin3<float> + offset);
        const calc::sinterbox_parameters params {
            .min = bounding.min + offset,
            .max = bounding.max + offset,
            .clearance = _clearance_spinner->GetValue(),
            .thickness = _thickness_spinner->GetValue(),
            .width = _width_spinner->GetValue(),
            .spacing = _spacing_spinner->GetValue() + 0.00013759,
        };
        _last_result->add_sinterbox(params);
        bounding = _last_result->bounding();
    }

    const auto size = bounding.max - bounding.min;
    const auto centroid = (size / 2) + geo::origin3<float>;
    _viewport->set_mesh(*_last_result, centroid);

    const double volume = _last_result->volume_and_centroid().volume;
    const double percent_volume = 100 * volume / (size.x * size.y * size.z);

    const auto message = wxString::Format(
        "Stacking complete!\n\nElapsed time: %.1fs\n\nFinal bounding box: %.1fx%.1fx%.1fmm (%.1f%% density).\n\nWould you like to save the result now?",
        elapsed.count(), size.x, size.y, size.z, percent_volume);
    if (wxMessageBox(message, "Stacking complete", wxYES_NO | wxYES_DEFAULT | wxICON_INFORMATION) == wxYES) {
        on_export();
    }
}

void main_window::enable_on_stacking(const bool starting) {
    const bool enable = not starting;
    enable_part_settings(enable and _current_part_index.has_value());
    _parts_list.control()->Enable(enable);
    for (wxMenuItem* item : _disableable_menu_items) {
        item->Enable(enable);
    }

    if (enable) {
        static thread_local std::vector<std::size_t> selected{};
        _parts_list.get_selected(selected);
        _import_button->Enable();
        _delete_button->Enable(selected.size() != 0);
        _change_button->Enable(selected.size() == 1);
        _reload_button->Enable(selected.size() != 0);
    } else {
        _import_button->Disable();
        _delete_button->Disable();
        _change_button->Disable();
        _reload_button->Disable();
    }

    _clearance_spinner->Enable(enable);
    _spacing_spinner->Enable(enable);
    _thickness_spinner->Enable(enable);
    _width_spinner->Enable(enable);
    _sinterbox_checkbox->Enable(enable);

    _initial_x_spinner->Enable(enable);
    _initial_y_spinner->Enable(enable);
    _initial_z_spinner->Enable(enable);
    _maximum_x_spinner->Enable(enable);
    _maximum_y_spinner->Enable(enable);
    _maximum_z_spinner->Enable(enable);

    _min_clearance_spinner->Enable(enable);
    _section_view_checkbox->Enable(enable);
    // _export_button is handled elsewhere
    _stack_button->SetLabelText(enable ? "Start" : "Stop");
    _progress_bar->SetValue(0);
}

wxMenuBar* main_window::make_menu_bar() {
    auto menu_bar = new wxMenuBar();
    enum class menu_item {
        _ = 0, // Menu items cannot be 0 on Mac
        new_, open, save, close,
        import, export_,
        about, website,
    };
    menu_bar->Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        switch (menu_item{ event.GetId() }) {
            case menu_item::new_: {
                return on_new(event);
            }
            case menu_item::open: {
                wxMessageBox("Not yet implemented", "Error", wxICON_WARNING);
                break;
            }
            case menu_item::save: {
                wxMessageBox("Not yet implemented", "Error", wxICON_WARNING);
                break;
            }
            case menu_item::close: {
                Close();
                break;
            }
            case menu_item::import: {
                return on_import(event);
            }
            case menu_item::export_: {
                return on_export(event);
            }
            case menu_item::about: {
                constexpr auto str =
                    "PartStacker Community Edition\n\n"
                    "PartStacker Community Edition is a continuation of PartStacker, (c)opyright Tom van der Zanden 2011-2013.\nVisit https://github.com/TomvdZanden/PartStacker/.\n\n"
                    "PartStacker Community Edition is (c)opyright Braden Ganetsky 2025.\nVisit https://github.com/PartStackerCommunity/PartStacker/.\n\n"
                    "Both the original and the Community Edition are licensed under the GNU General Public License v3.0.";
                wxMessageBox(str, "PartStacker Community Edition");
                break;
            }
            case menu_item::website: {
                wxLaunchDefaultBrowser("https://github.com/PartStackerCommunity/PartStacker/");
                break;
            }
        }
        event.Skip();
    });

    auto file_menu = new wxMenu();
    _disableable_menu_items.push_back(file_menu->Append((int)menu_item::new_, "&New\tCtrl-N", "Clear the current working session"));
    _disableable_menu_items.push_back(file_menu->Append((int)menu_item::open, "&Open\tCtrl-O", "Open PartStacker settings file"));
    _disableable_menu_items.push_back(file_menu->Append((int)menu_item::save, "&Save\tCtrl-S", "Save PartStacker settings"));
    file_menu->Append((int)menu_item::close, "&Close\tShift-Esc", "Close PartStacker");
    menu_bar->Append(file_menu, "&File");

    auto import_menu = new wxMenu();
    _disableable_menu_items.push_back(import_menu->Append((int)menu_item::import, "&Import\tCtrl-I", "Open mesh files"));
    _disableable_menu_items.push_back(import_menu->Append((int)menu_item::export_, "&Export\tCtrl-E", "Save last result as mesh file"));
    menu_bar->Append(import_menu, "&Mesh");

    auto help_menu = new wxMenu();
    help_menu->Append((int)menu_item::about, "&About", "About PartStacker");
    help_menu->Append((int)menu_item::website, "Visit &website", "Open PartStacker GitHub");
    menu_bar->Append(help_menu, "&Help");

    return menu_bar;
}

void main_window::on_new(wxCommandEvent& event) {
    if (_parts_list.rows() == 0 or
        wxMessageBox("Clear the current working session?",
                     "Warning",
                     wxYES_NO | wxNO_DEFAULT | wxICON_INFORMATION) == wxYES)
    {
        reset_fields();
        _parts_list.delete_all();
        unset_part();
        _viewport->remove_mesh();
        _last_result.reset();
        _export_button->Disable();
    }
    event.Skip();
}

void main_window::on_close(wxCloseEvent& event) {
    if (_parts_list.rows() != 0 and event.CanVeto()) {
        if (wxMessageBox("Close PartStacker?",
                         "Warning",
                         wxYES_NO | wxNO_DEFAULT | wxICON_INFORMATION) != wxYES)
        {
            event.Veto();
            return;
        }
    }
    _stacker_thread.stop();
    event.Skip();
}

void main_window::on_export(wxCommandEvent& event) {
    on_export();
    event.Skip();
}

void main_window::on_export() {
    if (not _last_result.has_value()) {
        wxMessageBox("Nothing to export", "Error", wxICON_WARNING);
        return;
    }

    wxFileDialog dialog(this, "Export mesh", "", "",
                        "STL files (*.stl)|*.stl",
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    const wxString path = dialog.GetPath();
    files::to_stl(*_last_result, path.ToStdString());
}

void main_window::on_import(wxCommandEvent& event) {
    wxFileDialog dialog(this, "Import mesh", "", "",
                        "STL files (*.stl)|*.stl",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxArrayString paths;
    dialog.GetPaths(paths);

    for (const auto& path : paths) {
        _parts_list.append(path.ToStdString());
    }
    _parts_list.update_label();
    if (paths.size() == 1) {
        auto& part = _parts_list.at(_parts_list.rows() - 1);
        _viewport->set_mesh(part.mesh, part.centroid);
    }

    event.Skip();
}

void main_window::on_delete(wxCommandEvent& event) {
    static thread_local std::vector<std::size_t> selected{};
    _parts_list.get_selected(selected);
    const auto message = wxString::Format("Delete %s %zu item%s?", selected.size() == 1 ? "this" : "these", selected.size(), selected.size() == 1 ? "" : "s");
    wxMessageDialog dialog(this, message, "Warning", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (dialog.ShowModal() == wxID_YES) {
        _parts_list.delete_selected();
        _parts_list.update_label();
        select_parts({});
    }
    event.Skip();
}

void main_window::on_change(wxCommandEvent& event) {
    wxFileDialog dialog(this, "Change mesh", "", "",
                        "STL files (*.stl)|*.stl",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    _parts_list.change(dialog.GetPath().ToStdString(), _current_part_index.value());
    _parts_list.update_label();
    set_part(_current_part_index.value());

    event.Skip();
}

void main_window::on_reload(wxCommandEvent& event) {
    static thread_local std::vector<std::size_t> selected{};
    _parts_list.get_selected(selected);
    for (const std::size_t row : selected) {
        _parts_list.reload_file(row);
    }
    _parts_list.update_label();
    select_parts({});
    event.Skip();
}

wxSizer* main_window::make_part_buttons() {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    const auto make_button = [this](const char* text, bool enable, void(main_window::*fn)(wxCommandEvent&)) {
        auto button = new wxButton(this, wxID_ANY, text);
        button->SetMinSize(FromDIP(button_size));
        button->Bind(wxEVT_BUTTON, fn, this);
        button->Enable(enable);
        return button;
    };
    _import_button = make_button("Import", true, &main_window::on_import);
    _delete_button = make_button("Delete", false, &main_window::on_delete);
    _change_button = make_button("Change", false, &main_window::on_change);
    _reload_button = make_button("Reload", false, &main_window::on_reload);
    sizer->Add(_import_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(inner_border));
    sizer->Add(_delete_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(inner_border));
    sizer->Add(_change_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(inner_border));
    sizer->Add(_reload_button, 1, wxEXPAND);
    return sizer;
}

wxSizer* main_window::make_bottom_section1() {
    auto sizer = new wxGridBagSizer(FromDIP(inner_border), FromDIP(inner_border));

    auto text1 = new wxStaticText(this, wxID_ANY, "Minimum clearance:");
    auto text2 = new wxStaticText(this, wxID_ANY, "Section view:");
    sizer->Add(text1, wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
    sizer->Add(text2, wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

    _min_clearance_spinner = new wxSpinCtrlDouble(this);
    _min_clearance_spinner->SetDigits(2);
    _min_clearance_spinner->SetIncrement(0.05);
    _min_clearance_spinner->SetRange(0.5, 2);
    sizer->Add(_min_clearance_spinner, wxGBPosition(0, 1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

    _section_view_checkbox = new wxCheckBox(this, wxID_ANY, "");
    sizer->Add(_section_view_checkbox, wxGBPosition(1, 1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

    new wxGBSizerItem(sizer, wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);

    _export_button = new wxButton(this, wxID_ANY, "Export result as STL");
    _export_button->SetMinSize(FromDIP(button_size));
    _export_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { return on_export(event); });
    _export_button->Disable();
    sizer->Add(_export_button, wxGBPosition(1, 3));

    sizer->AddGrowableCol(2, 1);
    return sizer;
}

wxSizer* main_window::make_bottom_section2() {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);

    _progress_bar = new wxGauge(this, wxID_ANY, 100);
    _progress_bar->SetMinSize(FromDIP(button_size));

    _stack_button = new wxButton(this, wxID_ANY, "Start");
    _stack_button->SetMinSize(FromDIP(button_size));
    _stack_button->Bind(wxEVT_BUTTON, &main_window::on_stacking, this);

    sizer->Add(_progress_bar, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(inner_border));
    sizer->Add(_stack_button);

    return sizer;
}

wxWindow* main_window::make_tabs() {
    auto notebook = new wxNotebook(this, wxID_ANY);

    const auto make_tab_panel = [notebook](std::size_t index, const char* label) -> wxPanel* {
        auto panel_base = new wxPanel(notebook);
        auto panel_base_sizer = new wxBoxSizer(wxVERTICAL);
        panel_base->SetSizer(panel_base_sizer);
        notebook->InsertPage(index, panel_base, label);
        auto panel = new wxPanel(panel_base);
        panel_base_sizer->Add(panel, 1, wxEXPAND | wxALL, notebook->FromDIP(tab_padding));
        return panel;
    };

    make_tab_part_settings(make_tab_panel(0, "Part Settings"));
    make_tab_sinterbox(make_tab_panel(1, "Sinterbox"));
    make_tab_bounding_box(make_tab_panel(2, "Bounding Box"));

    return notebook;
}

void main_window::make_tab_part_settings(wxPanel* panel) {
    auto panel_sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panel_sizer);
    auto top_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
    panel_sizer->Add(top_sizer, 0, wxEXPAND);
    panel_sizer->AddSpacer(panel->FromDIP(inner_border));
    panel_sizer->Add(bottom_sizer);

    {
        auto label_sizer = new wxGridSizer(3, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
        auto text1 = new wxStaticText(panel, wxID_ANY, "Quantity:");
        auto text2 = new wxStaticText(panel, wxID_ANY, "Minimum hole:");
        auto text3 = new wxStaticText(panel, wxID_ANY, "Minimize box:");
        label_sizer->Add(text1, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer->Add(text2, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer->Add(text3, 0, wxALIGN_CENTER_VERTICAL);

        auto button_sizer = new wxGridSizer(3, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
        auto make_spinner = [&panel](int maximum) {
            auto spinner = new wxSpinCtrl(panel);
            spinner->SetRange(0, maximum);
            return spinner;
        };
        _quantity_spinner = make_spinner(200);
        _min_hole_spinner = make_spinner(100);
        _minimize_checkbox = new wxCheckBox(panel, wxID_ANY, "");
        button_sizer->Add(_quantity_spinner, 0, wxALIGN_CENTER_VERTICAL);
        button_sizer->Add(_min_hole_spinner, 0, wxALIGN_CENTER_VERTICAL);
        button_sizer->Add(_minimize_checkbox, 0, wxALIGN_CENTER_VERTICAL);
        _quantity_spinner->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& event) {
            _current_part->quantity = event.GetPosition();
            _parts_list.reload_quantity(_current_part_index.value());
        });
        _min_hole_spinner->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& event) { _current_part->min_hole = event.GetPosition(); });
        _minimize_checkbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) { _current_part->rotate_min_box = event.IsChecked(); });

        auto radio_box = new wxStaticBoxSizer(wxVERTICAL, panel, "Part rotations");
        auto radio_sizer = new wxGridSizer(2, 2, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
        radio_box->Add(radio_sizer, 1, wxEXPAND | wxALL, panel->FromDIP(tab_padding));
        _radio_none = new wxRadioButton(panel, wxID_ANY, "None");
        _radio_arbitrary = new wxRadioButton(panel, wxID_ANY, "Arbitrary");
        _radio_cubic = new wxRadioButton(panel, wxID_ANY, "Cubic");
        radio_sizer->Add(_radio_none, 0, wxEXPAND);
        radio_sizer->Add(_radio_arbitrary, 0, wxEXPAND);
        radio_sizer->Add(_radio_cubic, 0, wxEXPAND);
        _radio_none->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& event) { _current_part->rotation_index = 0; });
        _radio_arbitrary->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& event) { _current_part->rotation_index = 2; });
        _radio_cubic->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& event) { _current_part->rotation_index = 1; });

        top_sizer->Add(label_sizer, 0, wxEXPAND);
        top_sizer->AddSpacer(panel->FromDIP(3 * inner_border));
        top_sizer->Add(button_sizer, 0, wxEXPAND);
        top_sizer->AddStretchSpacer();
        top_sizer->Add(radio_box, 0, wxEXPAND);
    }

    {
        _preview_button = new wxButton(panel, wxID_ANY, "Preview");
        _preview_button->SetMinSize(panel->FromDIP(button_size));
        _preview_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
            wxMessageBox("Not yet implemented", "Error", wxICON_WARNING);
        });
        _copy_button = new wxButton(panel, wxID_ANY, "Copy");
        _copy_button->SetMinSize(panel->FromDIP(button_size));
        _copy_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
            _parts_list.append(*_current_part);
            _parts_list.update_label();
            event.Skip();
        });
        _mirror_button = new wxButton(panel, wxID_ANY, "Mirror");
        _mirror_button->SetMinSize(panel->FromDIP(button_size));
        _mirror_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
            _current_part->mirrored = not _current_part->mirrored;
            _current_part->mesh.mirror_x();
            _current_part->mesh.set_baseline({ 0, 0, 0 });
            _parts_list.reload_text(_current_part_index.value());
            set_part(_current_part_index.value());
            event.Skip();
        });
        bottom_sizer->Add(_preview_button);
        bottom_sizer->AddSpacer(panel->FromDIP(inner_border));
        bottom_sizer->Add(_copy_button);
        bottom_sizer->AddSpacer(panel->FromDIP(inner_border));
        bottom_sizer->Add(_mirror_button);
    }

    enable_part_settings(false);
}

void main_window::make_tab_sinterbox(wxPanel* panel) {
    auto panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(panel_sizer);

    auto label_sizer = new wxGridSizer(4, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
    auto text1 = new wxStaticText(panel, wxID_ANY, "Clearance:");
    auto text2 = new wxStaticText(panel, wxID_ANY, "Spacing:");
    auto text3 = new wxStaticText(panel, wxID_ANY, "Thickness:");
    auto text4 = new wxStaticText(panel, wxID_ANY, "Width:");
    label_sizer->Add(text1, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer->Add(text2, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer->Add(text3, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer->Add(text4, 0, wxALIGN_CENTER_VERTICAL);

    auto button_sizer = new wxGridSizer(4, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
    auto make_spinner = [&panel](double minimum, double maximum, double increment) {
        auto spinner = new wxSpinCtrlDouble(panel);
        spinner->SetIncrement(increment);
        spinner->SetRange(minimum, maximum);
        return spinner;
    };
    _clearance_spinner = make_spinner(0.1, 4, 0.1);
    _spacing_spinner = make_spinner(1, 20, 0.5);
    _thickness_spinner = make_spinner(0.1, 4, 0.1);
    _width_spinner = make_spinner(0.1, 4, 0.1);
    button_sizer->Add(_clearance_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer->Add(_spacing_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer->Add(_thickness_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer->Add(_width_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

    auto checkbox_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto generate_text = new wxStaticText(panel, wxID_ANY, "Generate sinterbox:");
    _sinterbox_checkbox = new wxCheckBox(panel, wxID_ANY, "");
    checkbox_sizer->Add(generate_text, 0, wxALIGN_CENTER_VERTICAL);
    checkbox_sizer->AddSpacer(panel->FromDIP(2 * inner_border));
    checkbox_sizer->Add(_sinterbox_checkbox, 0, wxALIGN_CENTER_VERTICAL);

    panel_sizer->Add(label_sizer, 0, wxEXPAND);
    panel_sizer->AddSpacer(panel->FromDIP(8 * inner_border + 2));
    panel_sizer->Add(button_sizer, 0, wxEXPAND);
    panel_sizer->AddSpacer(panel->FromDIP(2 * inner_border));
    panel_sizer->Add(checkbox_sizer, 0, wxALIGN_LEFT | wxALIGN_TOP | wxUP, panel->FromDIP(4));
}

void main_window::make_tab_bounding_box(wxPanel* panel) {
    auto panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(panel_sizer);

    auto make_spinner = [panel]() {
        auto spinner = new wxSpinCtrl(panel);
        spinner->SetRange(10, 250);
        return spinner;
    };

    auto label_sizer1 = new wxGridSizer(4, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
    auto text1 = new wxStaticText(panel, wxID_ANY, "Initial X:");
    auto text2 = new wxStaticText(panel, wxID_ANY, "Initial Y:");
    auto text3 = new wxStaticText(panel, wxID_ANY, "Initial Z:");
    label_sizer1->Add(text1, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer1->Add(text2, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer1->Add(text3, 0, wxALIGN_CENTER_VERTICAL);

    auto button_sizer1 = new wxGridSizer(4, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
    _initial_x_spinner = make_spinner();
    _initial_y_spinner = make_spinner();
    _initial_z_spinner = make_spinner();
    button_sizer1->Add(_initial_x_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer1->Add(_initial_y_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer1->Add(_initial_z_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

    auto label_sizer2 = new wxGridSizer(4, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
    auto text4 = new wxStaticText(panel, wxID_ANY, "Maximum X:");
    auto text5 = new wxStaticText(panel, wxID_ANY, "Maximum Y:");
    auto text6 = new wxStaticText(panel, wxID_ANY, "Maximum Z:");
    label_sizer2->Add(text4, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer2->Add(text5, 0, wxALIGN_CENTER_VERTICAL);
    label_sizer2->Add(text6, 0, wxALIGN_CENTER_VERTICAL);

    auto button_sizer2 = new wxGridSizer(4, 1, panel->FromDIP(inner_border), panel->FromDIP(inner_border));
    _maximum_x_spinner = make_spinner();
    _maximum_y_spinner = make_spinner();
    _maximum_z_spinner = make_spinner();
    button_sizer2->Add(_maximum_x_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer2->Add(_maximum_y_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
    button_sizer2->Add(_maximum_z_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

    panel_sizer->Add(label_sizer1, 0, wxEXPAND);
    panel_sizer->AddSpacer(panel->FromDIP(4 * inner_border));
    panel_sizer->Add(button_sizer1, 0, wxEXPAND);
    panel_sizer->AddSpacer(panel->FromDIP(8 * inner_border));
    panel_sizer->Add(label_sizer2, 0, wxEXPAND);
    panel_sizer->AddSpacer(panel->FromDIP(4 * inner_border));
    panel_sizer->Add(button_sizer2, 0, wxEXPAND);
    panel_sizer->AddStretchSpacer();
}

} // namespace pstack::gui
