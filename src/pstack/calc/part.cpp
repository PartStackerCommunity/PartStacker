#include "pstack/calc/part.hpp"
#include "pstack/files/stl.hpp"
#include <charconv>
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>

namespace pstack::calc {

namespace {

std::optional<int> get_base_quantity(std::string name) {
    char looking_for = '.';
    if (name.ends_with(')')) {
        name.pop_back();
        looking_for = '(';
    }
    std::size_t number_length = 0;
    while (name.size() > number_length and std::isdigit(name[name.size() - number_length - 1])) {
        ++number_length;
    }
    if (number_length == 0 or not (name.size() > number_length and name[name.size() - number_length - 1] == looking_for)) {
        return std::nullopt;
    }
    std::string_view number{ name.data() + (name.size() - number_length), name.data() + name.size() };
    int out{-1};
    std::from_chars(number.data(), number.data() + number.size(), out);
    return out;
}

} // namespace

part initialize_part(part_base base) {
    part result{ std::move(base) };

    result.name = std::filesystem::path(result.mesh_file).stem().string();
    result.mesh = files::from_stl(result.mesh_file);
    result.mesh.set_baseline({ 0, 0, 0 });

    result.base_quantity = get_base_quantity(result.name);

    auto volume_and_centroid = result.mesh.volume_and_centroid();
    result.volume = volume_and_centroid.volume;
    result.centroid = volume_and_centroid.centroid;
    result.triangle_count = result.mesh.triangles().size();

    return result;
}

} // namespace pstack::calc
