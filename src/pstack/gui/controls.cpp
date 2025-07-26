#include "pstack/gui/constants.hpp"
#include "pstack/gui/controls.hpp"
#include "pstack/gui/main_window.hpp"
#include <array>

namespace pstack::gui {

namespace {

std::pair<wxNotebook*, std::vector<wxPanel*>> make_tab_panels(wxWindow* parent, const std::vector<const char*> labels) {
    wxNotebook* const notebook = new wxNotebook(parent, wxID_ANY);
    std::vector<wxPanel*> panels;
    for (std::size_t i = 0; i != labels.size(); ++i) {
        wxPanel* const panel_base = new wxPanel(notebook);
        wxBoxSizer* const panel_base_sizer = new wxBoxSizer(wxVERTICAL);
        panel_base->SetSizer(panel_base_sizer);
        notebook->InsertPage(i, panel_base, labels[i]);
        wxPanel* const panel = new wxPanel(panel_base);
        panel_base_sizer->Add(panel, 1, wxEXPAND | wxALL, notebook->FromDIP(constants::tab_padding));
        panels.push_back(panel);
    }
    return { notebook, std::move(panels) };
}

} // namespace

void controls::initialize(main_window* parent) {
    section_view_text = new wxStaticText(parent, wxID_ANY, "Section view:");
    section_view_checkbox = new wxCheckBox(parent, wxID_ANY, "");
    progress_bar = new wxGauge(parent, wxID_ANY, 100);
    stack_button = new wxButton(parent, wxID_ANY, "Stack");

    std::tie(notebook, notebook_panels) = make_tab_panels(parent, {
        "Parts",
        "Stack Settings",
        "Results",
    });

    {
        const auto panel = notebook_panels[0];

        import_part_button = new wxButton(panel, wxID_ANY, "Import");
        delete_part_button = new wxButton(panel, wxID_ANY, "Delete");
        delete_part_button->Disable();
        reload_part_button = new wxButton(panel, wxID_ANY, "Reload");
        reload_part_button->Disable();
        copy_part_button = new wxButton(panel, wxID_ANY, "Copy");
        copy_part_button->Disable();
        mirror_part_button = new wxButton(panel, wxID_ANY, "Mirror");
        mirror_part_button->Disable();

        quantity_text = new wxStaticText(panel, wxID_ANY, "Quantity:");
        min_hole_text = new wxStaticText(panel, wxID_ANY, "Minimum hole:");
        minimize_text = new wxStaticText(panel, wxID_ANY, "Minimize box:");
        quantity_spinner = new wxSpinCtrl(panel);
        quantity_spinner->SetRange(0, 200);
        quantity_spinner->Disable();
        min_hole_spinner = new wxSpinCtrl(panel);
        min_hole_spinner->SetRange(0, 100);
        min_hole_spinner->Disable();
        minimize_checkbox = new wxCheckBox(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
        minimize_checkbox->Disable();
        wxArrayString rotation_choices;
        rotation_choices.Add("None");
        rotation_choices.Add("Cubic");
        rotation_choices.Add("Arbitrary");
        rotation_text = new wxStaticText(panel, wxID_ANY, "Rotations:");
        rotation_dropdown = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, rotation_choices);
        rotation_dropdown->Disable();
        preview_voxelization_button = new wxButton(panel, wxID_ANY, "Preview voxelization");
        preview_voxelization_button->Disable();
        preview_bounding_box_button = new wxButton(panel, wxID_ANY, "Preview bounding box");
        preview_bounding_box_button->Disable();
    }

    {
        const auto panel = notebook_panels[1];

        initial_x_text = new wxStaticText(panel, wxID_ANY, "Initial X:");
        initial_y_text = new wxStaticText(panel, wxID_ANY, "Initial Y:");
        initial_z_text = new wxStaticText(panel, wxID_ANY, "Initial Z:");
        maximum_x_text = new wxStaticText(panel, wxID_ANY, "Maximum X:");
        maximum_y_text = new wxStaticText(panel, wxID_ANY, "Maximum Y:");
        maximum_z_text = new wxStaticText(panel, wxID_ANY, "Maximum Z:");
        const auto make_spinner = [panel] {
            auto spinner = new wxSpinCtrl(panel);
            spinner->SetRange(10, 250);
            return spinner;
        };
        initial_x_spinner = make_spinner();
        initial_y_spinner = make_spinner();
        initial_z_spinner = make_spinner();
        maximum_x_spinner = make_spinner();
        maximum_y_spinner = make_spinner();
        maximum_z_spinner = make_spinner();
        min_clearance_text = new wxStaticText(panel, wxID_ANY, "Minimum clearance:");
        min_clearance_spinner = new wxSpinCtrlDouble(panel);
        min_clearance_spinner->SetDigits(2);
        min_clearance_spinner->SetIncrement(0.05);
        min_clearance_spinner->SetRange(0.5, 2);

        constexpr auto make_tooltip = [](const char* state, char dir) {
            return wxString::Format("%s size of the bounding box %c direction", state, dir);
        };
        initial_x_text->SetToolTip(make_tooltip("Initial", 'X'));
        initial_y_text->SetToolTip(make_tooltip("Initial", 'Y'));
        initial_z_text->SetToolTip(make_tooltip("Initial", 'Z'));
        maximum_x_text->SetToolTip(make_tooltip("Maximum", 'X'));
        maximum_y_text->SetToolTip(make_tooltip("Maximum", 'Y'));
        maximum_z_text->SetToolTip(make_tooltip("Maximum", 'Z'));
        initial_x_spinner->SetToolTip(make_tooltip("Initial", 'X'));
        initial_y_spinner->SetToolTip(make_tooltip("Initial", 'Y'));
        initial_z_spinner->SetToolTip(make_tooltip("Initial", 'Z'));
        maximum_x_spinner->SetToolTip(make_tooltip("Maximum", 'X'));
        maximum_y_spinner->SetToolTip(make_tooltip("Maximum", 'Y'));
        maximum_z_spinner->SetToolTip(make_tooltip("Maximum", 'Z'));

        const wxString min_clearance_tooltip =
            "The minimum distance maintained between stacked parts. "
            "Also the voxel size fed into the stacking algorithm.";
        min_clearance_text->SetToolTip(min_clearance_tooltip);
        min_clearance_spinner->SetToolTip(min_clearance_tooltip);
    }

    {
        const auto panel = notebook_panels[2];
        export_result_button = new wxButton(panel, wxID_ANY, "Export");
        export_result_button->Disable();
        delete_result_button = new wxButton(panel, wxID_ANY, "Delete");
        delete_result_button->Disable();
        sinterbox_result_button = new wxButton(panel, wxID_ANY, "Add sinterbox");
        sinterbox_result_button->Disable();
        clearance_text = new wxStaticText(panel, wxID_ANY, "Clearance:");
        spacing_text = new wxStaticText(panel, wxID_ANY, "Spacing:");
        thickness_text = new wxStaticText(panel, wxID_ANY, "Thickness:");
        width_text = new wxStaticText(panel, wxID_ANY, "Width:");
        const auto make_spinner = [&panel](double minimum, double maximum, double increment) {
            auto spinner = new wxSpinCtrlDouble(panel);
            spinner->SetIncrement(increment);
            spinner->SetRange(minimum, maximum);
            return spinner;
        };
        clearance_spinner = make_spinner(0.1, 4, 0.1);
        spacing_spinner = make_spinner(1, 20, 0.5);
        thickness_spinner = make_spinner(0.1, 4, 0.1);
        width_spinner = make_spinner(0.1, 4, 0.1);
    }

    reset_values();
}

void controls::reset_values() {
    min_clearance_spinner->SetValue(1);

    quantity_spinner->SetValue(1);
    min_hole_spinner->SetValue(1);
    minimize_checkbox->SetValue(false);
    rotation_dropdown->SetSelection(1);

    clearance_spinner->SetValue(0.8);
    spacing_spinner->SetValue(6.0);
    thickness_spinner->SetValue(0.8);
    width_spinner->SetValue(1.1);

    initial_x_spinner->SetValue(150);
    initial_y_spinner->SetValue(150);
    initial_z_spinner->SetValue(30);
    maximum_x_spinner->SetValue(156);
    maximum_y_spinner->SetValue(156);
    maximum_z_spinner->SetValue(90);
}

} // namespace pstack::gui
