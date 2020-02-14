#pragma once

#include "OpCodes.h"

#include <optional>
#include <vector>

namespace Drill4dotNet
{
    // Represents header describing various method properties.
    // Located in the beginning of method body bytes.
    // Reference: ECMA-335, Common Language Infrastructure,
    // part II.25.4 Common Intermediate Language physical layout
    // https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf
    class MethodHeader
    {
    private:
        // 2 bits for tiny header, 12 bits for fat header
        uint16_t m_flags;

        // Size of header in bytes.
        // Has value "1" for tiny header, or
        // value from the fat header (typically 12)
        uint8_t m_headerSize;

        // fat header only
        std::optional<uint16_t> m_maxStack;

        // 6 bits for tiny header, 32 bits for fat header
        AbsoluteOffset m_codeSize;

        // 32 bits in fat header only
        std::optional<uint32_t> m_localVariables;

        // If there were a fat header bigger than the current
        // standard.
        std::vector<std::byte> m_fatHeaderRemainder;

        // Identifies the tiny method header type.
        inline static const std::byte s_TinyHeaderFlag{ CorILMethod_TinyFormat };

        // Allows to get the flags from the tiny header.
        inline static const std::byte s_TinyHeaderFlagsMask{ 0b00000011 };

        // Allows to get the instructions size from the tiny header.
        inline static const std::byte s_TinyHeaderSizeMask{ ~s_TinyHeaderFlagsMask };

        // Identifies whether the method has additional data sections.
        inline static const uint16_t s_SectionsFlag{ CorILMethod_MoreSects };

    public:
        // Parses a method header from the given body bytes.
        // @param methodBody : contains method header, instructions stream,
        //     and additional data sections as a byte array.
        MethodHeader(const std::vector<std::byte>& methodBody);

        // Serializes data from this header to the given bytes vector.
        // @param target : the bytes vector to save data to.
        void AppendToBytes(std::vector<std::byte>& target) const;

        // Gets the value indicating whether
        // there are one or more additional data
        // sections after the instructions stream.
        constexpr bool HasExceptionsSections() const noexcept
        {
            return (m_flags & s_SectionsFlag) != 0;
        }

        // Size of this header, in bytes.
        constexpr uint8_t Size() const noexcept
        {
            return m_headerSize;
        }

        // Size of the instructions stream, in bytes.
        constexpr AbsoluteOffset CodeSize() const noexcept
        {
            return m_codeSize;
        }

        // Sets the size of the instructions stream, in bytes.
        void SetCodeSize(const AbsoluteOffset codeSize) noexcept;

        // Gets the value indicating whether
        // this header can store the given length of an
        // instructions stream.
        // Returns true if the codeSize can be used with SetCodeSize.
        // @param codeSize : the length of an instructions stream.
        template <typename T>
        static constexpr bool IsValidCodeSize(const T codeSize) noexcept
        {
            return !Overflows<decltype(std::declval<MethodHeader>().m_codeSize)>(codeSize);
        }
    };
}
