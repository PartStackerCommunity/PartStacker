#include "pstack/files/split.hpp"

namespace pstack::files {

std::vector<std::string_view> split(std::string_view str, const char delimiter) {
    std::vector<std::string_view> out;
    for (std::size_t curr = 0; curr != std::string_view::npos; ) {
        const std::size_t next = str.find(delimiter, curr);
        if (next == std::string_view::npos) {
            out.emplace_back(&str[curr], str.size() - curr);
            break;
        } else {
            out.emplace_back(&str[curr], next - curr);
            curr = next + 1;
        }
    }
    return out;
}

} // namespace pstack::files
