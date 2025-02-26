#ifndef PSTACK_GUI_VIEWPORT_HPP
#define PSTACK_GUI_VIEWPORT_HPP

// OpenGL utilities must be included first
#include "pstack/graphics/buffer.hpp"
#include "pstack/graphics/shader.hpp"
#include "pstack/geo/mesh.hpp"
#include "pstack/gui/transformation.hpp"

#include <wx/event.h>
#include <wx/glcanvas.h>
#include <memory>

namespace pstack::gui {

class main_window;

class viewport : public wxGLCanvas {
public:
    viewport(main_window* parent, const wxGLAttributes& canvasAttrs);

    bool initialize();

    void on_paint(wxPaintEvent&);
    void render();
    void render(wxDC& dc);
    void on_size(wxSizeEvent& event);

    void set_mesh(const geo::mesh& mesh, const geo::point3<float>& centroid);
    void remove_mesh();

	void on_left_down(wxMouseEvent& evt);
	void on_motion(wxMouseEvent& evt);
	void on_left_up(wxMouseEvent& evt);
	void on_capture_lost(wxMouseCaptureLostEvent& evt);
    void on_scroll(wxMouseEvent& evt);

    wxPoint starting_pos;
    void on_move_by(wxPoint position);
    void on_finish_move();

private:
    std::unique_ptr<wxGLContext> _opengl_context = nullptr;
    bool _opengl_initialized = false;

    graphics::shader _shader{};
    graphics::vertex_array_object _vao{};
    transformation _transform{};

    wxSize _viewport_size{};
};

} // namespace pstack::gui

#endif // PSTACK_GUI_TRANSFORMATION_HPP
