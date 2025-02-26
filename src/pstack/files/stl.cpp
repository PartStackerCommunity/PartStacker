#include "pstack/files/read.hpp"
#include "pstack/files/stl.hpp"
#include "pstack/geo/triangle.hpp"
#include <charconv>
#include <ranges>
#include <sstream>
#include <string>

namespace pstack::files {

namespace {

template <std::ranges::contiguous_range R>
requires std::ranges::sized_range<R> and std::same_as<char, std::ranges::range_value_t<R>>
float parse_float(R&& r) {
    float result{};
    const char* data = &*r.begin();
    std::from_chars(data, data + r.size(), result);
    return result;
}

} // namespace

geo::mesh from_stl(const std::string& file_name) {
    std::string file = read_file(file_name);
    if (file.empty()) {
        return {};
    }
    const std::size_t file_size = file.size();
    std::istringstream ss(std::move(file));

    char header[80];
    ss.read(header, sizeof(header));

    std::uint32_t count;
    ss.read(reinterpret_cast<char*>(&count), sizeof(count));

    std::vector<geo::triangle> triangles{};

    if (file_size == 84 + count * 50) { // Binary STL
        char buffer[50];
        while (count-- != 0) {
            ss.read(buffer, sizeof(buffer));
            triangles.push_back(*reinterpret_cast<geo::triangle*>(buffer));
        }
    } else { // ASCII STL
        // facet normal ni nj nk
        //     outer loop
        //         vertex v1x v1y v1z
        //         vertex v2x v2y v2z
        //         vertex v3x v3y v3z
        //     endloop
        // endfacet

        ss.seekg(0);
        const std::vector<std::string> lines = [&ss] {
            std::vector<std::string> vec{};
            for (std::string line{}; std::getline(ss, line); ) {
                if (line.back() == '\r') {
                    line.pop_back();
                }
                vec.push_back(std::move(line));
            }
            return vec;
        }();

        for (std::size_t i = 1; i < lines.size() - 1; i += 7) {
            static constexpr auto parse_point = [](const std::string& line) {
                const auto vec = line
                    | std::views::split(' ')
                    | std::ranges::to<std::vector>();
                auto view = vec | std::views::reverse;
                return geo::point3<float>{
                    parse_float(view[2]),
                    parse_float(view[1]),
                    parse_float(view[0]),
                };
            };
            geo::vector3<float> normal = parse_point(lines[i]).as_vector();
            geo::point3<float> v1 = parse_point(lines[i + 2]);
            geo::point3<float> v2 = parse_point(lines[i + 3]);
            geo::point3<float> v3 = parse_point(lines[i + 4]);
            triangles.emplace_back(normal, v1, v2, v3);
        }
    }

    return geo::mesh(std::move(triangles));
}

void to_stl(const geo::mesh& mesh, std::string_view to_file) {

}

} // namespace pstack::files
