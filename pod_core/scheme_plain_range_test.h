#pragma once

#include <string>

namespace scheme::plain::range {
bool Test(std::string const& publish_path, std::string const& output_path,
          uint64_t start, uint64_t count);
}  // namespace scheme::plain::range