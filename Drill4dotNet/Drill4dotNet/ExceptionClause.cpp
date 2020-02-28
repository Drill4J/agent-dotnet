#include "pch.h"
#include "ExceptionClause.h"
#include <limits>

namespace Drill4dotNet
{
    // Maximum size of a protected code block, which can be represented by a small header.
    const CodeSize::NonOverflowingType s_MaxSmallExceptionClauseCodeSize = std::numeric_limits<uint8_t>::max();

    // Maximum offset of a protected code block, which can be represented by a small header.
    const CodeSize::NonOverflowingType s_MaxSmallExceptionClauseOffset = std::numeric_limits<uint16_t>::max();

    template <typename THeader>
    ExceptionClause::ExceptionClause(
        InstructionStream& target,
        LabelCreator& labelCreator,
        const THeader& header)
        : m_fat{ std::is_same_v<THeader, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT> },
        m_flags{ static_cast<CorExceptionFlag>(header.Flags) },
        m_tryOffset{ CreateLabelAtAbsoluteOffset(
            target,
            labelCreator,
            header.TryOffset) },
        m_tryEndOffset{ CreateLabelAtAbsoluteOffset(
            target,
            labelCreator,
            header.TryOffset + header.TryLength) },
        m_handlerOffset{ CreateLabelAtAbsoluteOffset(
            target,
            labelCreator,
            header.HandlerOffset) },
        m_handlerEndOffset{ CreateLabelAtAbsoluteOffset(
            target,
            labelCreator,
            header.HandlerOffset + header.HandlerLength) },
        m_target{ target }
    {
        if ((header.Flags & COR_ILEXCEPTION_CLAUSE_FILTER) != 0)
        {
            m_handler = CreateLabelAtAbsoluteOffset(
                target,
                labelCreator,
                header.FilterOffset);
        }
        else
        {
            m_handler = mdTypeDef{ header.ClassToken };
        }
    }

    ExceptionClause::ExceptionClause(
        const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT& header,
        InstructionStream& target,
        LabelCreator& labelCreator)
        : ExceptionClause(
            target,
            labelCreator,
            header)
    {
    }

    ExceptionClause::ExceptionClause(
        const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL& header,
        InstructionStream& target,
        LabelCreator& labelCreator)
        : ExceptionClause(
            target,
            labelCreator,
            header)
    {
    }

    Label ExceptionClause::CreateLabelAtAbsoluteOffset(
        InstructionStream& target,
        LabelCreator& labelCreator,
        const DWORD offset)
    {
        const ConstStreamPosition insertionPoint = ResolveAbsoluteOffset(target, offset);
        if (insertionPoint == target.cend())
        {
            throw std::runtime_error("Could not find an instruction by the given absolute offset.");
        }

        Label result{ labelCreator.CreateLabel() };
        target.insert(insertionPoint, result);
        return result;
    }

    bool ExceptionClause::CanPutToSmallHeader() const
    {
        if (m_fat)
        {
            return false;
        }

        return CanPutLabelPairToSmallHeader(m_tryOffset, m_tryEndOffset)
            && CanPutLabelPairToSmallHeader(m_handlerOffset, m_handlerEndOffset);
    }

    bool ExceptionClause::CanPutLabelPairToSmallHeader(
        const Label begin,
        const Label end) const
    {
        const CodeSize::NonOverflowingType beginOffset { CalculateAbsoluteOffset(begin) };
        if (beginOffset > s_MaxSmallExceptionClauseOffset)
        {
            return false;
        }

        const CodeSize::NonOverflowingType endOffset { CalculateAbsoluteOffset(end) };
        return endOffset - beginOffset <= s_MaxSmallExceptionClauseCodeSize;
    }

    CodeSize::ValueType ExceptionClause::CalculateAbsoluteOffset(const Label label) const
    {
        const ConstStreamPosition labelPosition { FindLabel(m_target, label) };
        if (labelPosition == m_target.cend())
        {
            throw std::logic_error("Compilation failed: unresolved label.");
        }

        return CodeSize::ValueType{ Drill4dotNet::CalculateAbsoluteOffset(m_target, labelPosition) };
    }

    template <typename Header>
    Header ExceptionClause::FillHeader() const
    {
        Header result{};
        result.Flags = m_flags;
        result.TryOffset =  CalculateAbsoluteOffset(m_tryOffset);
        result.TryLength = CalculateAbsoluteOffset(m_tryEndOffset) - result.TryOffset;
        result.HandlerOffset = CalculateAbsoluteOffset(m_handlerOffset);
        result.HandlerLength = CalculateAbsoluteOffset(m_handlerEndOffset) - result.HandlerOffset;
        return std::visit([&result, this](const auto& exceptionTypeOrFilter)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(exceptionTypeOrFilter)>, Label>)
            {
                result.FilterOffset = CalculateAbsoluteOffset(exceptionTypeOrFilter);
            }
            else
            {
                result.ClassToken = exceptionTypeOrFilter;
            }

            return result;
        },
            m_handler);
    }

    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL ExceptionClause::FillSmallHeader() const
    {
        if (!CanPutToSmallHeader())
        {
            throw std::logic_error("Cannot put this exception clause to a small header");
        }

        return FillHeader<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
    }

    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT ExceptionClause::FillFatHeader() const
    {
        return FillHeader<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
    }
}
