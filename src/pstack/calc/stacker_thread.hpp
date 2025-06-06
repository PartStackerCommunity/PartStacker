#ifndef PSTACK_CALC_STACKER_THREAD_HPP
#define PSTACK_CALC_STACKER_THREAD_HPP

#include "pstack/calc/stacker.hpp"
#include <atomic>
#include <optional>
#include <stdexcept>
#include <thread>

namespace pstack::calc {

class stacker_thread {
public:
    stacker_thread() = default;
    ~stacker_thread() {
        stop();
    }

    void start(stack_parameters params) {
        if (_thread.has_value()) {
            throw std::runtime_error("Thread already exists");
        }
        _thread.emplace([this, params = std::move(params)] {
            _stacker.stack(std::move(params));
        });
    }

    void stop() {
        _stacker.abort();
        if (_thread.has_value() and _thread->joinable()) {
            _thread->join();
        }
        _thread.reset();
    }

    bool running() const {
        return _stacker.running();
    }

private:
    stacker _stacker{};
    std::optional<std::thread> _thread{};
};

} // namespace pstack::calc

#endif // PSTACK_CALC_STACKER_THREAD_HPP
