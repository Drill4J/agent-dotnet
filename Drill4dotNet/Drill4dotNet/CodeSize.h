#pragma once

#include <cstdint>
#include "ByteUtils.h"

namespace Drill4dotNet
{
    // Unit to measure, in bytes, sizes of instructions,
    // sizes of instructions streams.
    class CodeSize
    {
    public:
        using ValueType = uint32_t;
        using NonOverflowingType = int64_t;

    private:
        ValueType m_size;

    public:
        constexpr static bool IsValidCodeSize(const NonOverflowingType size) noexcept
        {
            using SizeType = decltype(std::declval<CodeSize>().m_size);
            return !Overflows<SizeType>(size);
        }

        constexpr CodeSize(const ValueType size) noexcept
            : m_size(size)
        {
        }

        explicit constexpr CodeSize(const NonOverflowingType size)
            : m_size(0)
        {
            if (!IsValidCodeSize(size))
            {
                throw std::runtime_error("The given size cannot be represented as CodeSize");
            }

            m_size = size;
        }

        constexpr explicit operator ValueType() const noexcept
        {
            return m_size;
        }

        constexpr explicit operator NonOverflowingType() const noexcept
        {
            return m_size;
        }
    };

    constexpr CodeSize::NonOverflowingType operator+(const CodeSize a, const CodeSize::NonOverflowingType b) noexcept
    {
        return b + CodeSize::NonOverflowingType { a };
    }

    constexpr CodeSize::NonOverflowingType operator+(const CodeSize a, const CodeSize b) noexcept
    {
        return a + CodeSize::NonOverflowingType { b };
    }

    constexpr CodeSize::NonOverflowingType operator+(const CodeSize::NonOverflowingType a, const CodeSize b) noexcept
    {
        return b + a;
    }

    constexpr CodeSize::NonOverflowingType operator-(const CodeSize a) noexcept
    {
        return -CodeSize::NonOverflowingType { a };
    }

    constexpr CodeSize::NonOverflowingType operator-(const CodeSize a, const CodeSize::NonOverflowingType b) noexcept
    {
        return a + (-b);
    }

    constexpr CodeSize::NonOverflowingType operator-(const CodeSize a, const CodeSize b) noexcept
    {
        return a + (-b);
    }

    constexpr CodeSize::NonOverflowingType operator-(const CodeSize::NonOverflowingType a, const CodeSize b) noexcept
    {
        return a + (-b);
    }

    constexpr CodeSize::NonOverflowingType operator*(const CodeSize::NonOverflowingType a, const CodeSize b) noexcept
    {
        return a * CodeSize::NonOverflowingType { b };
    }

    constexpr CodeSize::NonOverflowingType operator*(const CodeSize a, const CodeSize::NonOverflowingType b) noexcept
    {
        return b * a;
    }

    constexpr CodeSize::NonOverflowingType operator*(const CodeSize a, const CodeSize b) noexcept
    {
        return CodeSize::NonOverflowingType { a } * b;
    }

    constexpr CodeSize::NonOverflowingType operator/(const CodeSize::NonOverflowingType a, const CodeSize b) noexcept
    {
        return a / CodeSize::NonOverflowingType{ b };
    }

    constexpr CodeSize::NonOverflowingType operator/(const CodeSize a, const CodeSize::NonOverflowingType b) noexcept
    {
        return CodeSize::NonOverflowingType { a } / b;
    }

    constexpr CodeSize::NonOverflowingType operator/(const CodeSize a, const CodeSize b) noexcept
    {
        return CodeSize::NonOverflowingType { a } / b;
    }
}
