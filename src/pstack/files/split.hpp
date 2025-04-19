#ifndef PSTACK_FILES_SPLIT_HPP
#define PSTACK_FILES_SPLIT_HPP

#include <string_view>
#include <vector>

namespace pstack::files {

std::vector<std::string_view> split(std::string_view str, char delimiter);

} // namespace pstack::files

#endif // PSTACK_FILES_SPLIT_HPP
