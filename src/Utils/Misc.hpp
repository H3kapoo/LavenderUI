#pragma once

#include <print>
#include <memory>

namespace src::utils
{
inline auto genId() -> uint64_t
{
    static uint64_t id{0};
    return id++;
}

template<typename T, typename... Args>
inline auto make(Args&&... args) -> std::shared_ptr<T>
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
} // namespace src::utils