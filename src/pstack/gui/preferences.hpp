#ifndef PSTACK_GUI_PREFERENCES_HPP
#define PSTACK_GUI_PREFERENCES_HPP

namespace pstack::gui {

struct preferences {
    bool invert_scroll = false;
    bool extra_parts = false;
    bool show_bounding_box = false;
    bool load_environment_popup = true;
};

} // namespace pstack::gui

#endif // PSTACK_GUI_PREFERENCES_HPP
