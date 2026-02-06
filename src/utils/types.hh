#pragma once

namespace vk_tutorial
{
struct NonCopyable
{
    constexpr NonCopyable() {};

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    NonCopyable(NonCopyable&&) noexcept = default;
    NonCopyable& operator=(NonCopyable&&) noexcept = default;
};

struct NonMovable : NonCopyable
{
    constexpr NonMovable() {};

    NonMovable(NonMovable&&) noexcept = delete;
    NonMovable& operator=(NonMovable&&) noexcept = delete;
};
}
