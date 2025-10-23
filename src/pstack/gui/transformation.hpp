#ifndef PSTACK_GUI_TRANSFORMATION_HPP
#define PSTACK_GUI_TRANSFORMATION_HPP

#include "pstack/geo/matrix4.hpp"
#include "pstack/geo/vector3.hpp"

namespace pstack::gui {

class transformation {
public:
    transformation() = default;

    const geo::matrix4<float>& for_normals() const {
        return _result_normals;
    }
    const geo::matrix4<float>& for_vertices() const {
        return _result_vertices;
    }

    void rotate_by(float rx, float ry) {
        _orientation = geo::rot4_y<float>(geo::radians{ry}) * geo::rot4_x<float>(geo::radians{rx}) * _orientation;
        _recalculate();
    }
    void scale_mesh(float factor) {
        _mesh_scale = geo::scale4(factor, factor, factor);
        _recalculate();
    }
    void zoom_by(float factor) {
        _zoom_scale = geo::scale4(factor, factor, factor) * _zoom_scale;
        _recalculate();
    }
    void scale_screen(float x, float y) {
        _screen_scale = geo::scale4(x, y, 1.0f);
        _recalculate();
    }
    void translation(geo::vector3<float> translation) {
        _translation = geo::translate4(translation.x, translation.y, translation.z);
        _recalculate();
    }

private:
    // OpenGL uses a left-handed coordinate system, but meshes are typically from a right-handed coordinate system
    static constexpr geo::matrix4<float> _mirror_z = geo::scale4(1.0f, 1.0f, -1.0f);

    geo::matrix4<float> _orientation = geo::eye4<float>;
    geo::matrix4<float> _mesh_scale = geo::eye4<float>;
    geo::matrix4<float> _zoom_scale = geo::eye4<float>;
    geo::matrix4<float> _screen_scale = geo::eye4<float>;
    geo::matrix4<float> _translation = geo::eye4<float>;

    geo::matrix4<float> _result_vertices = geo::eye4<float>;
    geo::matrix4<float> _result_normals = geo::eye4<float>;
    void _recalculate() {
        _result_normals = _orientation * _mirror_z;
        _result_vertices = _screen_scale * _zoom_scale * _mesh_scale * _result_normals * _translation;
    }
};

} // namespace pstack::gui

#endif // PSTACK_GUI_TRANSFORMATION_HPP
