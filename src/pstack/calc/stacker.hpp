#ifndef PSTACK_CALC_STACKER_HPP
#define PSTACK_CALC_STACKER_HPP

#include "pstack/calc/part.hpp"
#include "pstack/calc/sinterbox.hpp"
#include "pstack/geo/vector3.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <optional>
#include <vector>

namespace pstack::calc {

// `stack_result_base` is the base level information about the result
template <class Part, class Sinterbox>
struct stack_result_base {
    struct piece {
        Part part;
        geo::matrix3<float> rotation;
        geo::vector3<float> translation;
    };
    std::vector<piece> pieces{};
    std::optional<Sinterbox> sinterbox{};
};

// `stack_result` is everything that can be calculated/derived from the `stack_result_base`
struct stack_result : stack_result_base<std::shared_ptr<const part>, sinterbox_parameters> {
    mesh mesh{};
    geo::vector3<float> size{};
    double density{};

    void reload_mesh();
};

struct stack_settings {
    double resolution = 1.0;
    int x_min = 150;
    int x_max = 156;
    int y_min = 150;
    int y_max = 156;
    int z_min = 30;
    int z_max = 90;
};

struct stack_parameters {
    std::vector<std::shared_ptr<const part>> parts;
    stack_settings settings;

    std::function<void(double, double)> set_progress;
    std::function<void(const mesh&, const geo::point3<int>)> display_mesh;
    std::function<void(stack_result, std::chrono::system_clock::duration)> on_success;
    std::function<void()> on_failure;
    std::function<void()> on_finish;
};

class stacker {
public:
    stacker()
        : _running(false)
    {}

    stacker(const stacker&) = delete;
    stacker& operator=(const stacker&) = delete;

    bool running() const {
        return _running;
    }

    void stack(stack_parameters params);

    void abort() {
        _running = false;
    }

private:
    std::atomic<bool> _running;
};

} // namespace pstack::calc

#endif // PSTACK_CALC_STACKER_HPP
