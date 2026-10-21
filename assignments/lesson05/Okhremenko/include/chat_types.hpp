#pragma once

#include <cstdint>
#include <string>

namespace lesson05 {

constexpr std::uint64_t CONTEXT_LIMIT = 1024;
constexpr std::uint64_t MAX_TOKENS = 64;

struct Message {
    std::string role;
    std::string content;
};

} // namespace lesson05