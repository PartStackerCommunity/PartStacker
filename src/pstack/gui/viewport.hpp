#ifndef PSTACK_GUI_VIEWPORT_HPP
#define PSTACK_GUI_VIEWPORT_HPP

// OpenGL utilities must be included first
#include "pstack/graphics/buffer.hpp"
#include "pstack/graphics/shader.hpp"

#include "pstack/calc/mesh.hpp"
#include "pstack/gui/transformation.hpp"

#include <wx/event.h>
#include <wx/glcanvas.h>
#include <memory>

namespace pstack::gui {

class main_window;

class viewport : public wxGLCanvas {
public:
    viewport(main_window* parent, const wxGLAttributes& canvasAttrs);

private:
    bool initialize();
    int _scroll_direction = 1;

public:
    void set_mesh(const calc::mesh& mesh, const geo::point3<float>& centroid);
    void remove_mesh();
    void render();

    void scroll_direction(bool invert_scroll) {
        _scroll_direction = invert_scroll ? -1 : 1;
    }

private:
    void render(wxDC& dc);
    void on_paint(wxPaintEvent&);
    void on_size(wxSizeEvent& event);

    void on_left_down(wxMouseEvent& evt);
    void on_motion(wxMouseEvent& evt);
    void on_left_up(wxMouseEvent& evt);
    void on_capture_lost(wxMouseCaptureLostEvent& evt);
    void on_scroll(wxMouseEvent& evt);

    wxPoint _cached_position;
    void on_move_by(wxPoint position);
    void on_finish_move();

    std::unique_ptr<wxGLContext> _opengl_context = nullptr;
    bool _opengl_initialized = false;

    graphics::shader _mesh_shader{};
    graphics::vertex_array_object _mesh_vao{};
    transformation _transform{};

    wxSize _viewport_size{};
};

} // namespace pstack::gui

#endif // PSTACK_GUI_VIEWPORT_HPP
