#include "pstack/files/read.hpp"
#include "pstack/files/stl.hpp"
#include "pstack/geo/triangle.hpp"
#include <array>
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>

namespace pstack::files {

calc::mesh from_stl(const std::string& file_path) {
    auto file_expected = read_file(file_path);
    if (not file_expected.has_value()) {
        return {};
    }
    std::string& file = *file_expected;
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
                auto vec_view = line
                              | std::views::split(' ')
                              | std::views::transform([](auto&& x) { return std::string(x.begin(), x.end()); });
                const auto vec = std::vector<std::string>(vec_view.begin(), vec_view.end());
                auto view = vec | std::views::reverse;
                return geo::point3<float>{
                    std::stof(view[2]),
                    std::stof(view[1]),
                    std::stof(view[0]),
                };
            };
            geo::vector3<float> normal = parse_point(lines[i]).as_vector();
            geo::point3<float> v1 = parse_point(lines[i + 2]);
            geo::point3<float> v2 = parse_point(lines[i + 3]);
            geo::point3<float> v3 = parse_point(lines[i + 4]);

#if defined(__cpp_aggregate_paren_init) and __cpp_aggregate_paren_init >= 201902L
            triangles.emplace_back(normal, v1, v2, v3);
#else
            triangles.push_back(geo::triangle{normal, v1, v2, v3});
#endif
        }
    }

    return calc::mesh(std::move(triangles));
}

void to_stl(const calc::mesh& mesh, const std::string& file_path) {
    std::ofstream file(file_path, std::ios::out | std::ios::binary);

    const std::array<std::byte, 80> header{};
    file.write(reinterpret_cast<const char*>(header.data()), header.size());

    const std::uint32_t count = static_cast<std::uint32_t>(mesh.triangles().size());
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    std::array<std::byte, 50> triangle{};
    for (const auto& t : mesh.triangles()) {
        reinterpret_cast<float*>(triangle.data())[ 0] = t.normal.x;
        reinterpret_cast<float*>(triangle.data())[ 1] = t.normal.y;
        reinterpret_cast<float*>(triangle.data())[ 2] = t.normal.z;
        reinterpret_cast<float*>(triangle.data())[ 3] = t.v1.x;
        reinterpret_cast<float*>(triangle.data())[ 4] = t.v1.y;
        reinterpret_cast<float*>(triangle.data())[ 5] = t.v1.z;
        reinterpret_cast<float*>(triangle.data())[ 6] = t.v2.x;
        reinterpret_cast<float*>(triangle.data())[ 7] = t.v2.y;
        reinterpret_cast<float*>(triangle.data())[ 8] = t.v2.z;
        reinterpret_cast<float*>(triangle.data())[ 9] = t.v3.x;
        reinterpret_cast<float*>(triangle.data())[10] = t.v3.y;
        reinterpret_cast<float*>(triangle.data())[11] = t.v3.z;
        file.write(reinterpret_cast<const char*>(triangle.data()), triangle.size());
    }
}

} // namespace pstack::files
