#include "pstack/files/read.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace pstack::files {

// From StackOverflow https://stackoverflow.com/a/40903508
std::string read_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::in | std::ios::binary);
    if (not file.is_open()) {
        return {};
    }
    const auto size = std::filesystem::file_size(file_path);
    std::string result(size, '\0');
    file.read(result.data(), size);
    return result;
}

} // namespace pstack::files
