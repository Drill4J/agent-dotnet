#pragma once

namespace Drill4dotNet
{
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
