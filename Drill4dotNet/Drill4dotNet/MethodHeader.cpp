#include "pch.h"

#include "MethodHeader.h"

namespace Drill4dotNet
{
    MethodHeader::MethodHeader(const std::vector<std::byte>& methodBody)
    {
        if (methodBody.empty())
        {
            throw std::runtime_error("Method header expected");
        }

        std::byte firstByte = *methodBody.cbegin();
        if ((firstByte & s_TinyHeaderFlagsMask) == s_TinyHeaderFlag)
        { // tiny header
            if (methodBody.size() < sizeof(IMAGE_COR_ILMETHOD_TINY))
            {
                throw std::runtime_error("Unexpected method header end");
            }

            m_flags = static_cast<uint16_t>(firstByte & s_TinyHeaderFlagsMask);
            m_headerSize = 1;
            m_maxStack = std::nullopt;
            m_codeSize = static_cast<AbsoluteOffset>(firstByte & s_TinyHeaderSizeMask) >> 2;
            m_localVariables = std::nullopt;
        }
        else
        { // fat header
            if (methodBody.size() < sizeof(IMAGE_COR_ILMETHOD_FAT))
            {
                throw std::runtime_error("Unexpected method header end");
            }

            const IMAGE_COR_ILMETHOD_FAT& fatHeader
                = *reinterpret_cast<const IMAGE_COR_ILMETHOD_FAT*>(methodBody.data());
            m_flags = fatHeader.Flags;
            m_headerSize = static_cast<uint8_t>(sizeof(uint32_t) * fatHeader.Size);
            m_maxStack = static_cast<uint16_t>(fatHeader.MaxStack);
            m_codeSize = fatHeader.CodeSize;
            m_localVariables = fatHeader.LocalVarSigTok == 0
                ? std::optional<uint32_t>(std::nullopt)
                : fatHeader.LocalVarSigTok;

            if (methodBody.size() < m_headerSize)
            {
                throw std::runtime_error("Unexpected method header end");
            }

            m_fatHeaderRemainder.assign(
                methodBody.cbegin() + sizeof(fatHeader),
                methodBody.cbegin() + m_headerSize);
        }
    }

    void MethodHeader::AppendToBytes(std::vector<std::byte>& target) const
    {
        if (m_headerSize == sizeof(std::byte)) // Dangerous, need to recalculate header type
        { // tiny header
            target.push_back(std::byte{ m_flags | m_codeSize << 2 });
        }
        else
        { // fat header
            IMAGE_COR_ILMETHOD_FAT header;
            header.Flags = m_flags;
            header.Size = m_headerSize / sizeof(uint32_t); // Dangerous, assert m_headerSize / sizeof(int32_t) == 0
            header.MaxStack = m_maxStack.value(); // Dangerous, may be std::nullopt
            header.LocalVarSigTok = m_localVariables.has_value() ? *m_localVariables : 0;
            header.CodeSize = m_codeSize; // Dangerous, need to recalculate m_codeSize

            AppendAsBytes(target, header);
            std::copy(
                m_fatHeaderRemainder.cbegin(),
                m_fatHeaderRemainder.cend(),
                std::back_inserter(target));
        }
    }

    void MethodHeader::SetCodeSize(const AbsoluteOffset codeSize) noexcept
    {
        m_codeSize = codeSize;
    }
}
