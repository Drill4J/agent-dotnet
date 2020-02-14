#pragma once

#include <array>

namespace Drill4dotNet
{
    // Appends the binary representation of the
    // given object to the bytes vector.
    // @param target : the vector to append bytes to.
    // @param value : the object to extract raw bytes from.
    template <typename T>
    void AppendAsBytes(std::vector<std::byte>& target, const T& value)
    {
        for (const auto b : (std::array<std::byte, sizeof(T)>&)value)
        {
            target.push_back(b);
        }
    }

    // Gets the value indicating whether
    // the given numeric value cannot be converted to
    // another numeric type without an overflow.
    // Returns true if the overflow happens.
    // TTarget : the type to which the value is being converted.
    // TSource : the type of the value.
    // @param value : the value to check.
    template <typename TTarget, typename TSource>
    constexpr bool Overflows(const TSource value) noexcept
    {
        constexpr bool sourceSigned{ std::numeric_limits<std::decay_t<TSource>>::is_signed };
        using Limits = std::numeric_limits<std::decay_t<TTarget>>;
        constexpr bool targetSigned{ Limits::is_signed };

        if constexpr (sourceSigned ^ targetSigned)
        {
            if constexpr (sourceSigned)
            {
                if (value < 0)
                {
                    return true;
                }
            }

            if constexpr (targetSigned)
            {
                return value > Limits::max();
            }
        }

        return !(Limits::min() <= value && value <= Limits::max());
    }
}
