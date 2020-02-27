#pragma once

#include <cstdint>
#include "ByteUtils.h"

namespace Drill4dotNet
{
    // Unit to measure, in bytes, sizes of instructions,
    // sizes of instructions streams.
    class CodeSize
    {
    private:
        uint32_t m_size;

    public:
        using NonOverflowingSize = int64_t;

        constexpr static bool IsValidCodeSize(NonOverflowingSize size) noexcept
        {
            // return !Overflows...
        }

        constexpr CodeSize(uint32_t size) noexcept
        {}

        constexpr CodeSize(NonOverflowingSize size)
        {
            if (!IsValidCodeSize(size))
            {
                throw 0; // ...
            }
        }

        operator uint32_t() const noexcept;
        operator NonOverflowing() const noexcept;

        operator <<() const;

    
    };

    NonOverflowing operator +(CodeSize a, CodeSize b);
    NonOverflowing operator +(CodeSize::NonOverflowing a, CodeSize b);
    NonOverflowing operator +(CodeSize a, CodeSize::NonOverflowing b);
}
