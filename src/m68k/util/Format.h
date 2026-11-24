#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace m68k::util {

/*
 * Helpers standing in for the Java formatting APIs used by the original code.
 */

/// Java Integer.toHexString(int) - lowercase, no padding, two's complement for negatives.
inline std::string toHexString(int32_t value) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%x", static_cast<uint32_t>(value));
    return std::string(buf);
}

namespace detail {

// snprintf needs a plain char pointer for %s; unwrap std::string arguments.
template <typename T>
decltype(auto) unwrap(const T& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        return value.c_str();
    } else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
        return value ? "true" : "false";
    } else {
        return value;
    }
}

} // namespace detail

/// Java String.format() for the subset of format specifiers used by this code base.
template <typename... Args>
std::string format(const char* fmt, Args... args) {
    const int len = std::snprintf(nullptr, 0, fmt, detail::unwrap(args)...);
    if (len <= 0) {
        return std::string();
    }

    std::vector<char> buffer(static_cast<size_t>(len) + 1, '\0');
    std::snprintf(buffer.data(), buffer.size(), fmt, detail::unwrap(args)...);
    return std::string(buffer.data(), static_cast<size_t>(len));
}

} // namespace m68k::util
