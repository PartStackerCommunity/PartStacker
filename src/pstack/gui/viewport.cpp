#include "pstack/graphics/global.hpp" // OpenGL utilities must be included first

#include "pstack/gui/main_window.hpp"
#include "pstack/gui/viewport.hpp"
#include <wx/dcclient.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

namespace pstack::gui {

viewport::viewport(main_window* parent, const wxGLAttributes& canvasAttrs)
    : wxGLCanvas(parent, canvasAttrs)
{
    wxGLContextAttrs ctx_attrs;
    ctx_attrs.PlatformDefaults().CoreProfile().OGLVersion(3, 3).EndList();
    _opengl_context = std::make_unique<wxGLContext>(this, nullptr, &ctx_attrs);

    if (!_opengl_context->IsOK()) {
        wxMessageBox("PartStacker requires an OpenGL 3.3 capable driver.",
                     "OpenGL version error", wxOK | wxICON_INFORMATION, this);
        _opengl_context.reset();
    }

    Bind(wxEVT_PAINT, &viewport::on_paint, this);
    Bind(wxEVT_SIZE, &viewport::on_size, this);

    Bind(wxEVT_LEFT_DOWN, &viewport::on_left_down, this);
    Bind(wxEVT_MOUSEWHEEL, &viewport::on_scroll, this);
}

constexpr auto mesh_vertex_shader_source = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    out vec4 frag_normal;
    uniform mat4 transform_vertices;
    uniform mat4 transform_normals;
    void main() {
        gl_Position = transform_vertices * vec4(aPos, 1.0);
        frag_normal = transform_normals * vec4(aNormal, 1.0);
    }
)";

constexpr auto mesh_fragment_shader_source = R"(
    #version 330 core
    in vec4 frag_normal;
    out vec4 frag_colour;
    void main() {
        const float shade_min = 63.0 / 255.0;
        const float shade_max = 206.0 / 255.0;
        const float shade_factor = shade_min - shade_max; // This is negative
        float shade = (frag_normal.z * shade_factor) + shade_min;
        frag_colour = vec4(shade, shade, shade, 1.0);
    }
)";

bool viewport::initialize() {
    if (!_opengl_context) {
        return false;
    }

    SetCurrent(*_opengl_context);

    if (const auto err = graphics::initialize();
        not err.has_value())
    {
        wxMessageBox(wxString::Format("OpenGL GLEW initialization failed with message: \"%s\"", err.error()),
                     "OpenGL initialization error", wxOK | wxICON_ERROR, this);
        return false;
    }

    if (const auto err = _mesh_shader.initialize(mesh_vertex_shader_source, mesh_fragment_shader_source);
        not err.has_value())
    {
        wxMessageBox(wxString::Format("Error in creating OpenGL shader for the mesh.\n%s", err.error()),
                     "OpenGL mesh shader error", wxOK | wxICON_ERROR, this);
        return false;
    }

    _mesh_vao.initialize();
    remove_mesh();

    return true;
}

void viewport::set_mesh(const calc::mesh& mesh, const geo::point3<float>& centroid) {
    _mesh_vao.clear();

    using vector3 = geo::vector3<float>;

    std::vector<vector3> vertices;
    std::vector<vector3> normals;
    for (const auto& t : mesh.triangles()) {
        vertices.push_back(t.v1.as_vector());
        vertices.push_back(t.v2.as_vector());
        vertices.push_back(t.v3.as_vector());
        normals.push_back(t.normal);
        normals.push_back(t.normal);
        normals.push_back(t.normal);
    }

    _mesh_vao.add_vertex_buffer(0, std::move(vertices));
    _mesh_vao.add_vertex_buffer(1, std::move(normals));

    const auto bounding = mesh.bounding();
    _transform.translation(geo::origin3<float> - centroid);
    const auto size = bounding.max - bounding.min;
    const auto zoom_factor = 1 / std::max({ size.x, size.y, size.z });
    _transform.scale_mesh(zoom_factor);

    render();
}

void viewport::remove_mesh() {
    _mesh_vao.clear();
    _mesh_vao.add_vertex_buffer(0, {});
    _mesh_vao.add_vertex_buffer(1, {});

    _transform.translation({ 0, 0, 0 });
    _transform.scale_mesh(1);

    render();
}

void viewport::render() {
    wxClientDC dc(this);
    render(dc);
}

void viewport::render(wxDC& dc) {
    if (!_opengl_initialized) {
        return;
    }

    SetCurrent(*_opengl_context);

    graphics::clear(40 / 255.0, 50 / 255.0, 120 / 255.0, 1);

    _mesh_shader.use_program();
    _mesh_shader.set_uniform("transform_vertices", _transform.for_vertices());
    _mesh_shader.set_uniform("transform_normals", _transform.for_normals());
    _mesh_vao.bind_arrays();
    graphics::draw_triangles(_mesh_vao[0].size());

    SwapBuffers();
}

void viewport::on_paint(wxPaintEvent&) {
    wxPaintDC dc(this);
    render(dc);
}

void viewport::on_size(wxSizeEvent& event) {
    const bool first_appearance = IsShownOnScreen() && !_opengl_initialized;
    if (first_appearance) {
        _opengl_initialized = initialize();
    }

    if (_opengl_initialized) {
        _viewport_size = event.GetSize() * GetContentScaleFactor();
        graphics::viewport(0, 0, _viewport_size.x, _viewport_size.y);

        static constexpr float scale_baseline = 866;
        _transform.scale_screen(scale_baseline / _viewport_size.x, scale_baseline / _viewport_size.y);

        if (first_appearance) {
            render();
        }
    }

    event.Skip();
}

void viewport::on_left_down(wxMouseEvent& evt) {
    _cached_position = evt.GetPosition();
    Bind(wxEVT_LEFT_UP, &viewport::on_left_up, this);
    Bind(wxEVT_MOTION, &viewport::on_motion, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &viewport::on_capture_lost, this);
    CaptureMouse();
}

void viewport::on_motion(wxMouseEvent& evt) {
    on_move_by(evt.GetPosition());
}

void viewport::on_left_up(wxMouseEvent& evt) {
    on_move_by(evt.GetPosition());
    on_finish_move();
}

void viewport::on_capture_lost(wxMouseCaptureLostEvent& evt) {
    on_finish_move();
}

void viewport::on_scroll(wxMouseEvent& evt) {
    const double zoom_amount = ((double)evt.GetWheelRotation() / (double)evt.GetWheelDelta()) / 4;
    const float zoom_factor = (float)std::pow(2.0, _scroll_direction * zoom_amount);
    _transform.zoom_by(zoom_factor);
    render();
}

void viewport::on_move_by(wxPoint position) {
    const auto [dx, dy] = _cached_position - position;
    _cached_position = position;
    _transform.rotate_by((float)dy / 256, (float)dx / 256);
    render();
}

void viewport::on_finish_move() {
    if (HasCapture()) {
        ReleaseMouse();
    }
    Unbind(wxEVT_LEFT_UP, &viewport::on_left_up, this);
    Unbind(wxEVT_MOTION, &viewport::on_motion, this);
    Unbind(wxEVT_MOUSE_CAPTURE_LOST, &viewport::on_capture_lost, this);
}

} // namespace pstack::gui
