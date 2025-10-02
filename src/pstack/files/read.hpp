#ifndef PSTACK_FILES_READ_HPP
#define PSTACK_FILES_READ_HPP

#include <expected>
#include <string>

namespace pstack::files {

std::expected<std::string, std::string> read_file(const std::string& file_path);

} // namespace pstack::files

#endif // PSTACK_FILES_READ_HPP
