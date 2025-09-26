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

struct stack_result {
    struct piece {
        std::shared_ptr<const part> part;
        geo::matrix3<float> rotation;
        geo::vector3<float> translation;
    };
    std::vector<piece> pieces{};

    mesh mesh{};
    geo::vector3<float> size{};
    double density{};
    std::optional<sinterbox_parameters> sinterbox{};
};

struct stack_parameters {
    std::vector<std::shared_ptr<const part>> parts;

    std::function<void(double, double)> set_progress;
    std::function<void(const mesh&, const geo::point3<int>)> display_mesh;
    std::function<void(stack_result, std::chrono::system_clock::duration)> on_success;
    std::function<void()> on_failure;
    std::function<void()> on_finish;

    double resolution;
    int x_min, x_max;
    int y_min, y_max;
    int z_min, z_max;
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
