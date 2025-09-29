#include "pstack/gui/results_list.hpp"

namespace pstack::gui {

void results_list::initialize(wxWindow* parent) {
    list_view::initialize(parent, {
        { "Pieces", 50 },
        { "Density", 55 },
        { "Size", 110 },
        { "Triangles", 70 },
        { "Sinterbox", 90 },
    });
}

void results_list::append(calc::stack_result input) {
    auto& result = _results.emplace_back(std::move(input));
    const auto bounding = result.mesh.bounding();
    result.size = bounding.max - bounding.min;
    const double volume = result.mesh.volume_and_centroid().volume;
    result.density = volume / (result.size.x * result.size.y * result.size.z);
    list_view::append({
        std::to_string(result.pieces.size()),
        wxString::Format("%.1f%%", 100 * result.density),
        wxString::Format("%.1fx%.1fx%.1f", result.size.x, result.size.y, result.size.z),
        std::to_string(result.mesh.triangles().size()),
        (not result.sinterbox.has_value()) ? wxString("none")
            : wxString::Format("%.1f,%.1f,%.1f,%.1f", result.sinterbox->settings.clearance, result.sinterbox->settings.spacing, result.sinterbox->settings.thickness, result.sinterbox->settings.width),
    });
}

void results_list::delete_all() {
    list_view::delete_all();
    _results.clear();
}

void results_list::delete_selected() {
    list_view::delete_selected(_results);
}

} // namespace pstack::gui
