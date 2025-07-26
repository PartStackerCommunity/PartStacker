#include "pstack/files/stl.hpp"
#include "pstack/gui/constants.hpp"
#include "pstack/gui/main_window.hpp"
#include "pstack/gui/parts_list.hpp"
#include "pstack/gui/viewport.hpp"

#include <cstdlib>
#include <wx/colourdata.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

namespace pstack::gui {

main_window::main_window(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
{
#ifdef _WIN32
    SetBackgroundColour(constants::form_background_colour);
#endif

    SetMenuBar(make_menu_bar());

    _controls.initialize(this);
    _parts_list.initialize(_controls.notebook_panels[0]);
    _results_list.initialize(_controls.notebook_panels[2]);
    bind_all_controls();

    wxGLAttributes attrs;
    attrs.PlatformDefaults().Defaults().EndList();
    if (not wxGLCanvas::IsDisplaySupported(attrs)) {
        wxMessageBox("wxGLCanvas::IsDisplaySupported() returned false", "PartStacker fatal error", wxICON_ERROR);
        std::exit(EXIT_FAILURE);
    }
    _viewport = new viewport(this, attrs);
    _viewport->SetMinSize(FromDIP(constants::min_viewport_size));

    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(_viewport, 1, wxEXPAND);
    sizer->Add(arrange_all_controls(), 0, wxEXPAND | wxALL, FromDIP(constants::outer_border));
    SetSizerAndFit(sizer);
}

void main_window::on_select_parts(const std::vector<std::size_t>& indices) {
    const bool any_selected = not indices.empty();
    _controls.delete_part_button->Enable(any_selected);
    _controls.reload_part_button->Enable(any_selected);
    _controls.copy_part_button->Enable(any_selected);
    _controls.mirror_part_button->Enable(any_selected);
    enable_part_settings(any_selected);
    _current_parts.clear();
    if (not any_selected) {
        return;
    }

    std::optional<int> quantity{};
    std::optional<int> min_hole{};
    std::optional<bool> rotate_min_box{};
    std::optional<int> rotation_index{};
    bool first_time = true;
    for (const std::size_t index : indices) {
        calc::part& part = _parts_list.at(index);
        _current_parts.emplace_back(&part, index);
        if (first_time) {
            first_time = false;
            quantity.emplace(part.quantity);
            min_hole.emplace(part.min_hole);
            rotate_min_box.emplace(part.rotate_min_box);
            rotation_index.emplace(part.rotation_index);
        } else {
            if (quantity.has_value() and *quantity != part.quantity) {
                quantity.reset();
            }
            if (min_hole.has_value() and *min_hole != part.min_hole) {
                min_hole.reset();
            }
            if (rotate_min_box.has_value() and *rotate_min_box != part.rotate_min_box) {
                rotate_min_box.reset();
            }
            if (rotation_index.has_value() and *rotation_index != part.rotation_index) {
                rotation_index.reset();
            }
        }
    }
    if (quantity.has_value()) {
        _controls.quantity_spinner->SetValue(*quantity);
    } else {
        _controls.quantity_spinner->SetValue("");
    }
    if (min_hole.has_value()) {
        _controls.min_hole_spinner->SetValue(*min_hole);
    } else {
        _controls.min_hole_spinner->SetValue("");
    }
    if (rotate_min_box.has_value()) {
        _controls.minimize_checkbox->SetValue(*rotate_min_box);
    } else {
        _controls.minimize_checkbox->Set3StateValue(wxCHK_UNDETERMINED);
    }
    if (rotation_index.has_value()) {
        _controls.rotation_dropdown->SetSelection(*rotation_index);
    } else {
        _controls.rotation_dropdown->SetSelection(wxNOT_FOUND);
    }

    if (_current_parts.size() == 1) {
        const calc::part& part = *_current_parts[0].part;
        _viewport->set_mesh(part.mesh, part.centroid);
    }
}

void main_window::enable_part_settings(bool enable) {
    _controls.quantity_spinner->Enable(enable);
    _controls.min_hole_spinner->Enable(enable);
    _controls.minimize_checkbox->Enable(enable);
    _controls.rotation_dropdown->Enable(enable);
    _controls.preview_voxelization_button->Enable(enable);
    _controls.preview_bounding_box_button->Enable(enable);
}

void main_window::on_select_results(const std::vector<std::size_t>& indices) {
    const auto size = indices.size();
    _controls.export_result_button->Enable(size == 1);
    _controls.delete_result_button->Enable(size != 0);
    _controls.sinterbox_result_button->Enable(size == 1);
    if (size == 1) {
        set_result(indices[0]);
    } else {
        unset_result();
    }
}

void main_window::set_result(const std::size_t index) {
    _current_result = &_results_list.at(index);
    const auto& result = _results_list.at(index);
    const auto bounding = result.mesh.bounding();
    const auto size = bounding.max - bounding.min;
    const auto centroid = (size / 2) + geo::origin3<float>;
    _viewport->set_mesh(result.mesh, centroid);
}

void main_window::unset_result() {
    _current_result = nullptr;
}

void main_window::on_switch_tab(wxBookCtrlEvent& event) {
    static thread_local std::vector<std::size_t> selected{};
    switch (event.GetSelection()) {
        case 0: {
            _parts_list.get_selected(selected);
            on_select_parts(selected);
            break;
        }
        case 2: {
            _results_list.get_selected(selected);
            if (selected.size() == 1) {
                set_result(selected[0]);
            }
            break;
        }
    }
    event.Skip();
}

void main_window::on_stacking(wxCommandEvent& event) {
    if (_controls.stack_button->GetLabelText() == "Stop") {
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

    calc::stack_parameters params {
        .parts = _parts_list.get_all(),

        .set_progress = [this](double progress, double total) {
            CallAfter([=] {
                _controls.progress_bar->SetValue(static_cast<int>(100 * progress / total));
            });
        },
        .display_mesh = [this](const calc::mesh& mesh, int max_x, int max_y, int max_z) {
            CallAfter([=] {
                // Make a copy of `mesh`, otherwise we encounter a data race
                _viewport->set_mesh(mesh, { max_x / 2.0f, max_y / 2.0f, max_z / 2.0f });
            });
        },
        .on_success = [this](calc::stack_result result, const std::chrono::system_clock::duration elapsed) {
            CallAfter([=, result = std::move(result)] {
                on_stacking_success(std::move(result), elapsed);
            });
        },
        .on_failure = [this] {
            CallAfter([=] {
                wxMessageBox("Could not stack parts within maximum bounding box", "Stacking failed");
            });
        },
        .on_finish = [this] {
            CallAfter([=] {
                _stacker_thread.stop();
                enable_on_stacking(false);
            });
        },

        .resolution = _controls.min_clearance_spinner->GetValue(),
        .x_min = _controls.initial_x_spinner->GetValue(), .x_max = _controls.maximum_x_spinner->GetValue(),
        .y_min = _controls.initial_y_spinner->GetValue(), .y_max = _controls.maximum_y_spinner->GetValue(),
        .z_min = _controls.initial_z_spinner->GetValue(), .z_max = _controls.maximum_z_spinner->GetValue(),
    };
    enable_on_stacking(true);
    _stacker_thread.start(std::move(params));
}

void main_window::on_stacking_stop() {
    if (wxMessageBox("Abort stacking?", "Warning", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) == wxYES) {
        _stacker_thread.stop();
    }
}

void main_window::on_stacking_success(calc::stack_result result, const std::chrono::system_clock::duration elapsed) {
    _results_list.append(std::move(result));
    set_result(_results_list.rows() - 1);

    const auto message = wxString::Format(
        "Stacking complete!\n\nElapsed time: %.1fs\n\nFinal bounding box: %.1fx%.1fx%.1fmm (%.1f%% density).",
        std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count(),
        _current_result->size.x, _current_result->size.y, _current_result->size.z, 100 * _current_result->density);
    wxMessageBox(message, "Stacking complete");
}

void main_window::enable_on_stacking(const bool starting) {
    const bool enable = not starting;
    enable_part_settings(enable and not _current_parts.empty());
    _parts_list.control()->Enable(enable);
    for (wxMenuItem* item : _disableable_menu_items) {
        item->Enable(enable);
    }

    if (enable) {
        static thread_local std::vector<std::size_t> selected{};
        _parts_list.get_selected(selected);
        _controls.import_part_button->Enable();
        _controls.delete_part_button->Enable(selected.size() != 0);
        _controls.reload_part_button->Enable(selected.size() != 0);
        _controls.copy_part_button->Enable(selected.size() == 1);
        _controls.mirror_part_button->Enable(selected.size() == 1);
        _results_list.get_selected(selected);
        _controls.export_result_button->Enable(selected.size() == 1);
        _controls.delete_result_button->Enable(selected.size() != 0);
        _controls.sinterbox_result_button->Enable(selected.size() != 0);
    } else {
        _controls.import_part_button->Disable();
        _controls.delete_part_button->Disable();
        _controls.reload_part_button->Disable();
        _controls.copy_part_button->Disable();
        _controls.mirror_part_button->Disable();
        _controls.export_result_button->Disable();
        _controls.delete_result_button->Disable();
        _controls.sinterbox_result_button->Disable();
    }

    _controls.clearance_spinner->Enable(enable);
    _controls.spacing_spinner->Enable(enable);
    _controls.thickness_spinner->Enable(enable);
    _controls.width_spinner->Enable(enable);

    _controls.initial_x_spinner->Enable(enable);
    _controls.initial_y_spinner->Enable(enable);
    _controls.initial_z_spinner->Enable(enable);
    _controls.maximum_x_spinner->Enable(enable);
    _controls.maximum_y_spinner->Enable(enable);
    _controls.maximum_z_spinner->Enable(enable);

    _controls.min_clearance_spinner->Enable(enable);
    _controls.section_view_checkbox->Enable(enable);
    _controls.stack_button->SetLabelText(enable ? "Stack" : "Stop");
    _controls.progress_bar->SetValue(0);
}

wxMenuBar* main_window::make_menu_bar() {
    auto menu_bar = new wxMenuBar();
    enum class menu_item {
         // Menu items cannot be 0 on Mac
        new_ = 1, open, save, close,
        import, export_,
        pref_scroll, pref_extra,
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
                return on_import_part(event);
            }
            case menu_item::export_: {
                return on_export_result(event);
            }
            case menu_item::pref_scroll: {
                _preferences.invert_scroll = not _preferences.invert_scroll;
                _viewport->scroll_direction(_preferences.invert_scroll);
                break;
            }
            case menu_item::pref_extra: {
                _preferences.extra_parts = not _preferences.extra_parts;
                _parts_list.show_extra(_preferences.extra_parts);
                _parts_list.reload_all_text();
                break;
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

    auto preferences_menu = new wxMenu();
    preferences_menu->AppendCheckItem((int)menu_item::pref_scroll, "Invert &scroll", "Change the viewport scroll direction");
    preferences_menu->AppendCheckItem((int)menu_item::pref_extra, "Display &extra parts", "Display the quantity of extra parts separately");
    menu_bar->Append(preferences_menu, "&Preferences");

    auto help_menu = new wxMenu();
    help_menu->Append((int)menu_item::about, "&About", "About PartStacker");
    help_menu->Append((int)menu_item::website, "Visit &website", "Open PartStacker GitHub");
    menu_bar->Append(help_menu, "&Help");

    return menu_bar;
}

void main_window::bind_all_controls() {
    Bind(wxEVT_CLOSE_WINDOW, &main_window::on_close, this);

    _parts_list.bind([this](const std::vector<std::size_t>& selected) {
        on_select_parts(selected);
    });
    _results_list.bind([this](const std::vector<std::size_t>& selected) {
        on_select_results(selected);
    });
    _controls.notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &main_window::on_switch_tab, this);

    _controls.import_part_button->Bind(wxEVT_BUTTON, &main_window::on_import_part, this);
    _controls.delete_part_button->Bind(wxEVT_BUTTON, &main_window::on_delete_part, this);
    _controls.reload_part_button->Bind(wxEVT_BUTTON, &main_window::on_reload_part, this);
    _controls.copy_part_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        for (auto& current_part : _current_parts) {
            _parts_list.append(*current_part.part);
        }
        _parts_list.update_label();
        event.Skip();
    });
    _controls.mirror_part_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        static thread_local std::vector<std::size_t> indices{};
        indices.clear();
        for (auto& current_part : _current_parts) {
            indices.push_back(current_part.index);
            current_part.part->mirrored = not current_part.part->mirrored;
            current_part.part->mesh.mirror_x();
            current_part.part->mesh.set_baseline({ 0, 0, 0 });
            _parts_list.reload_text(current_part.index);
        }
        on_select_parts(indices);
        event.Skip();
    });

    _controls.stack_button->Bind(wxEVT_BUTTON, &main_window::on_stacking, this);
    _controls.export_result_button->Bind(wxEVT_BUTTON, &main_window::on_export_result, this);
    _controls.delete_result_button->Bind(wxEVT_BUTTON, &main_window::on_delete_result, this);
    _controls.sinterbox_result_button->Bind(wxEVT_BUTTON, &main_window::on_sinterbox_result, this);

    _controls.quantity_spinner->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& event) {
        for (auto& current_part : _current_parts) {
            current_part.part->quantity = event.GetPosition();
            _parts_list.reload_quantity(current_part.index);
        }
        event.Skip();
    });
    _controls.min_hole_spinner->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& event) {
        for (auto& current_part : _current_parts) {
            current_part.part->min_hole = event.GetPosition();
        }
        event.Skip();
    });
    _controls.minimize_checkbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
        for (auto& current_part : _current_parts) {
            current_part.part->rotate_min_box = event.IsChecked();
        }
        event.Skip();
    });

    _controls.rotation_dropdown->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event) {
        for (auto& current_part : _current_parts) {
            current_part.part->rotation_index = _controls.rotation_dropdown->GetSelection();
        }
        event.Skip();
    });

    _controls.preview_voxelization_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        wxMessageBox("Not yet implemented", "Error", wxICON_WARNING);
    });
    _controls.preview_bounding_box_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        wxMessageBox("Not yet implemented", "Error", wxICON_WARNING);
    });

    _controls.section_view_checkbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
        wxMessageBox("Not yet implemented", "Error", wxICON_WARNING);
    });
}

void main_window::on_new(wxCommandEvent& event) {
    if (_parts_list.rows() == 0 or
        wxMessageBox("Clear the current working session?",
                     "Warning",
                     wxYES_NO | wxNO_DEFAULT | wxICON_INFORMATION) == wxYES)
    {
        _controls.reset_values();
        _parts_list.delete_all();
        on_select_parts({});
        _results_list.delete_all();
        unset_result();
        _viewport->remove_mesh();
    }
    event.Skip();
}

void main_window::on_close(wxCloseEvent& event) {
    if ((_parts_list.rows() != 0 or _results_list.rows() != 0) and event.CanVeto()) {
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

void main_window::on_import_part(wxCommandEvent& event) {
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
        const calc::part& part = _parts_list.at(_parts_list.rows() - 1);
        _viewport->set_mesh(part.mesh, part.centroid);
    }

    event.Skip();
}

void main_window::on_delete_part(wxCommandEvent& event) {
    static thread_local std::vector<std::size_t> selected{};
    _parts_list.get_selected(selected);
    const auto message = wxString::Format("Delete %s %zu item%s?", selected.size() == 1 ? "this" : "these", selected.size(), selected.size() == 1 ? "" : "s");
    wxMessageDialog dialog(this, message, "Warning", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (dialog.ShowModal() == wxID_YES) {
        _parts_list.delete_selected();
        _parts_list.update_label();
        on_select_parts({});
    }
    event.Skip();
}

void main_window::on_reload_part(wxCommandEvent& event) {
    static thread_local std::vector<std::size_t> selected{};
    _parts_list.get_selected(selected);
    for (const std::size_t row : selected) {
        _parts_list.reload_file(row);
    }
    _parts_list.update_label();
    on_select_parts({});
    event.Skip();
}

void main_window::on_export_result(wxCommandEvent& event) {
    if (nullptr == _current_result) {
        wxMessageBox("No result selected", "Error", wxICON_WARNING);
        return;
    }

    wxFileDialog dialog(this, "Export mesh", "", "",
                        "STL files (*.stl)|*.stl",
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    const wxString path = dialog.GetPath();
    files::to_stl(_current_result->mesh, path.ToStdString());
    event.Skip();
}

void main_window::on_delete_result(wxCommandEvent& event) {
    static thread_local std::vector<std::size_t> selected{};
    _results_list.get_selected(selected);
    const auto message = wxString::Format("Delete %s %zu result%s?", selected.size() == 1 ? "this" : "these", selected.size(), selected.size() == 1 ? "" : "s");
    wxMessageDialog dialog(this, message, "Warning", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (dialog.ShowModal() == wxID_YES) {
        _results_list.delete_selected();
        on_select_results({});
    }
    event.Skip();
}

void main_window::on_sinterbox_result(wxCommandEvent& event) {
    if (nullptr == _current_result) {
        wxMessageBox("No result selected", "Error", wxICON_WARNING);
        return;
    }
    if (_current_result->sinterbox.has_value()) {
        wxMessageBox("This result already has a sinterbox", "Error", wxICON_WARNING);
        return;
    }

    auto result = *_current_result; // Copy the result
    const double offset = _controls.thickness_spinner->GetValue() + _controls.clearance_spinner->GetValue();
    const auto bounding = result.mesh.bounding();
    result.mesh.set_baseline(geo::origin3<float> + offset);
    result.sinterbox = calc::sinterbox_parameters{
        .min = bounding.min + offset,
        .max = bounding.max + offset,
        .clearance = _controls.clearance_spinner->GetValue(),
        .thickness = _controls.thickness_spinner->GetValue(),
        .width = _controls.width_spinner->GetValue(),
        .spacing = _controls.spacing_spinner->GetValue() + 0.00013759,
    };
    result.mesh.add_sinterbox(*result.sinterbox);

    _results_list.append(std::move(result));
    set_result(_results_list.rows() - 1);
    event.Skip();
}

wxSizer* main_window::arrange_all_controls() {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(arrange_tabs(), 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(arrange_bottom_section1(), 0, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(arrange_bottom_section2(), 0, wxEXPAND);
    return sizer;
}

wxSizer* main_window::arrange_part_buttons() {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    _controls.import_part_button->SetMinSize(FromDIP(constants::min_button_size));
    _controls.delete_part_button->SetMinSize(FromDIP(constants::min_button_size));
    _controls.reload_part_button->SetMinSize(FromDIP(constants::min_button_size));
    _controls.copy_part_button->SetMinSize(FromDIP(constants::min_button_size));
    _controls.mirror_part_button->SetMinSize(FromDIP(constants::min_button_size));
    sizer->Add(_controls.import_part_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.delete_part_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.reload_part_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.copy_part_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.mirror_part_button, 1, wxEXPAND);
    return sizer;
}

wxSizer* main_window::arrange_result_buttons() {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    _controls.export_result_button->SetMinSize(FromDIP(constants::min_button_size));
    _controls.delete_result_button->SetMinSize(FromDIP(constants::min_button_size));
    _controls.sinterbox_result_button->SetMinSize(FromDIP(constants::min_button_size));
    sizer->Add(_controls.export_result_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.delete_result_button, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.sinterbox_result_button, 1, wxEXPAND);
    return sizer;
}

wxSizer* main_window::arrange_bottom_section1() {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(_controls.section_view_text, 0, wxALIGN_CENTER_VERTICAL);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.section_view_checkbox, 0, wxALIGN_CENTER_VERTICAL);
    sizer->AddStretchSpacer();
    return sizer;
}

wxSizer* main_window::arrange_bottom_section2() {
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    _controls.progress_bar->SetMinSize(FromDIP(constants::min_button_size));
    _controls.stack_button->SetMinSize(FromDIP(constants::min_button_size));
    sizer->Add(_controls.progress_bar, 1, wxEXPAND);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(_controls.stack_button);
    return sizer;
}

wxNotebook* main_window::arrange_tabs() {
    arrange_tab_part_settings(_controls.notebook_panels[0]);
    arrange_tab_stack_settings(_controls.notebook_panels[1]);
    arrange_tab_results(_controls.notebook_panels[2]);
    return _controls.notebook;
}

void main_window::arrange_tab_part_settings(wxPanel* panel) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    sizer->Add(_parts_list.control(), 1, wxEXPAND);
    sizer->AddSpacer(panel->FromDIP(constants::inner_border));
    sizer->Add(arrange_part_buttons(), 0, wxEXPAND);
    sizer->AddSpacer(panel->FromDIP(constants::inner_border));
    sizer->Add(_parts_list.label());
    sizer->AddSpacer(panel->FromDIP(constants::inner_border));

    auto lower_sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(lower_sizer, 0, wxEXPAND);

    {
        auto label_sizer = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        label_sizer->Add(_controls.quantity_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer->Add(_controls.min_hole_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer->Add(_controls.minimize_text, 0, wxALIGN_CENTER_VERTICAL);

        auto spinner_sizer = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        spinner_sizer->Add(_controls.quantity_spinner, 0, wxALIGN_CENTER_VERTICAL);
        spinner_sizer->Add(_controls.min_hole_spinner, 0, wxALIGN_CENTER_VERTICAL);
        spinner_sizer->Add(_controls.minimize_checkbox, 0, wxALIGN_CENTER_VERTICAL);

        auto button_sizer = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        auto rotation_sizer = new wxBoxSizer(wxHORIZONTAL);
        rotation_sizer->Add(_controls.rotation_text, 0, wxALIGN_CENTER_VERTICAL);
        rotation_sizer->AddSpacer(panel->FromDIP(2 * constants::inner_border));
        rotation_sizer->Add(_controls.rotation_dropdown, 0, wxALIGN_CENTER_VERTICAL);
        button_sizer->Add(rotation_sizer);
        button_sizer->Add(_controls.preview_voxelization_button);
        button_sizer->Add(_controls.preview_bounding_box_button);

        lower_sizer->Add(label_sizer, 0, wxEXPAND);
        lower_sizer->AddSpacer(panel->FromDIP(3 * constants::inner_border));
        lower_sizer->Add(spinner_sizer, 0, wxEXPAND);
        lower_sizer->AddSpacer(panel->FromDIP(6 * constants::inner_border));
        lower_sizer->Add(button_sizer, 0, wxEXPAND);
        lower_sizer->AddStretchSpacer();
    }
}

void main_window::arrange_tab_stack_settings(wxPanel* panel) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    auto bounding_box_sizer_ = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Bounding box");
    {
        auto bounding_box_sizer = new wxBoxSizer(wxHORIZONTAL);
        bounding_box_sizer_->Add(bounding_box_sizer, 0, wxALL, panel->FromDIP(constants::inner_border));

        auto label_sizer1 = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        label_sizer1->Add(_controls.initial_x_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer1->Add(_controls.initial_y_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer1->Add(_controls.initial_z_text, 0, wxALIGN_CENTER_VERTICAL);

        auto button_sizer1 = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        button_sizer1->Add(_controls.initial_x_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
        button_sizer1->Add(_controls.initial_y_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
        button_sizer1->Add(_controls.initial_z_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

        auto label_sizer2 = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        label_sizer2->Add(_controls.maximum_x_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer2->Add(_controls.maximum_y_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer2->Add(_controls.maximum_z_text, 0, wxALIGN_CENTER_VERTICAL);

        auto button_sizer2 = new wxGridSizer(3, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        button_sizer2->Add(_controls.maximum_x_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
        button_sizer2->Add(_controls.maximum_y_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
        button_sizer2->Add(_controls.maximum_z_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

        bounding_box_sizer->Add(label_sizer1, 0, wxEXPAND);
        bounding_box_sizer->AddSpacer(panel->FromDIP(4 * constants::inner_border));
        bounding_box_sizer->Add(button_sizer1, 0, wxEXPAND);
        bounding_box_sizer->AddSpacer(panel->FromDIP(8 * constants::inner_border));
        bounding_box_sizer->Add(label_sizer2, 0, wxEXPAND);
        bounding_box_sizer->AddSpacer(panel->FromDIP(4 * constants::inner_border));
        bounding_box_sizer->Add(button_sizer2, 0, wxEXPAND);
        bounding_box_sizer->AddStretchSpacer();
    }

    auto min_clearance_sizer = new wxBoxSizer(wxHORIZONTAL);
    min_clearance_sizer->Add(_controls.min_clearance_text, 0, wxALIGN_CENTER_VERTICAL);
    min_clearance_sizer->AddSpacer(4 * FromDIP(constants::inner_border));
    min_clearance_sizer->Add(_controls.min_clearance_spinner, 0, wxALIGN_CENTER_VERTICAL);

    sizer->Add(bounding_box_sizer_, 0, wxEXPAND | wxLEFT | wxRIGHT);
    sizer->AddSpacer(FromDIP(constants::inner_border));
    sizer->Add(min_clearance_sizer);
}

void main_window::arrange_tab_results(wxPanel* panel) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    sizer->Add(_results_list.control(), 1, wxEXPAND);
    sizer->AddSpacer(panel->FromDIP(constants::inner_border));
    sizer->Add(arrange_result_buttons(), 0, wxEXPAND);
    sizer->AddSpacer(panel->FromDIP(constants::inner_border));

    auto sinterbox_sizer_ = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Sinterbox");
    {
        auto sinterbox_sizer = new wxBoxSizer(wxHORIZONTAL);
        sinterbox_sizer_->Add(sinterbox_sizer, 0, wxALL, panel->FromDIP(constants::inner_border));

        auto label_sizer1 = new wxGridSizer(2, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        label_sizer1->Add(_controls.clearance_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer1->Add(_controls.spacing_text, 0, wxALIGN_CENTER_VERTICAL);

        auto button_sizer1 = new wxGridSizer(2, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        button_sizer1->Add(_controls.clearance_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
        button_sizer1->Add(_controls.spacing_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

        auto label_sizer2 = new wxGridSizer(2, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        label_sizer2->Add(_controls.thickness_text, 0, wxALIGN_CENTER_VERTICAL);
        label_sizer2->Add(_controls.width_text, 0, wxALIGN_CENTER_VERTICAL);

        auto button_sizer2 = new wxGridSizer(2, 1, panel->FromDIP(constants::inner_border), panel->FromDIP(constants::inner_border));
        button_sizer2->Add(_controls.thickness_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);
        button_sizer2->Add(_controls.width_spinner, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND);

        sinterbox_sizer->Add(label_sizer1, 0, wxEXPAND);
        sinterbox_sizer->AddSpacer(panel->FromDIP(6 * constants::inner_border + 2));
        sinterbox_sizer->Add(button_sizer1, 0, wxEXPAND);
        sinterbox_sizer->AddSpacer(panel->FromDIP(8 * constants::inner_border + 2));
        sinterbox_sizer->Add(label_sizer2, 0, wxEXPAND);
        sinterbox_sizer->AddSpacer(panel->FromDIP(6 * constants::inner_border + 2));
        sinterbox_sizer->Add(button_sizer2, 0, wxEXPAND);
    }

    sizer->Add(sinterbox_sizer_, 0, wxEXPAND | wxLEFT | wxRIGHT);
}

} // namespace pstack::gui
