#pragma once

#include "OpCodes.h"
#include "InstructionStream.h"
#include "framework.h"

namespace Drill4dotNet
{
    // Represents one try-catch or try-finally block.
    // Can be constructed from .net api structures and
    // converted back to them.
    // Reference: ECMA-335, Common Language Infrastructure,
    // part II.25.4.6 Exception handling clauses
    // https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf
    class ExceptionClause
    {
    private:
        // Value indicating whether the instance was
        // created from a fat structure.
        bool m_fat;

        // Flags describing whether this is try-catch or try-finally
        CorExceptionFlag m_flags;

        // Label to the first instruction belonging to the try block
        Label m_tryOffset;

        // Label to the first instruction after the try block
        Label m_tryEndOffset;

        // Label to the first instruction of the catch or finally block
        Label m_handlerOffset;

        // Label to the first instruction after the catch or finally block
        Label m_handlerEndOffset;

        // mdTypeDef if it is a catch without when clause.
        // Otherwise, Label to the first instruction of the when filter.
        std::variant<mdTypeDef, Label> m_handler;

        // The instructions sequence, in which this try-catch or try-finally is used.
        const InstructionStream& m_target;

        // Adds a new label to the given instruction stream.
        // @param target : the instruction stream to create label in.
        // @param labelCreator : tool to emit new labels.
        // @param offset : absolute offset from the beginning of the target.
        static Label CreateLabelAtAbsoluteOffset(
            InstructionStream& target,
            LabelCreator& labelCreator,
            const DWORD offset);

        // Creates a new instance from the given type of header.
        // Adds new labels to the target instructions stream.
        template <typename THeader>
        ExceptionClause(
            InstructionStream& target,
            LabelCreator& labelCreator,
            const THeader& header);

        // Gets the value indicating whether both
        // the given labels can be represented by
        // a small header.
        // @param begin : the first label to check
        // @param end : the second label to check
        bool CanPutLabelPairToSmallHeader(
            const Label begin,
            const Label end) const;

        // Calculates the absolute offset.
        // @param label : the label to calculate offset of.
        CodeSize::ValueType CalculateAbsoluteOffset(const Label label) const;

        // Creates a header of the given type, storing the
        // information from this instance.
        template <typename Header>
        Header FillHeader() const;

    public:

        // Fills a new instance from the given fat header.
        // @param header : the fat header with the clause information.
        // @param target : will insert new labels into this instruction stream.
        // @param labelCreator : tool to emit new labels.
        ExceptionClause(
            const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT& header,
            InstructionStream& target,
            LabelCreator& labelCreator);

        // Fills a new instance from the given small header.
        // @param header : the small header with the clause information.
        // @param target : will insert new labels into this instruction stream.
        // @param labelCreator : tool to emit new labels.
        ExceptionClause(
            const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL& header,
            InstructionStream& target,
            LabelCreator& labelCreator);

        // Gets the value indicating whether
        // this clause can be represented by
        // a small header.
        bool CanPutToSmallHeader() const;

        // Creates a small header with information from this instance.
        // Throws std::logic_error if a fat header must be used.
        IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL FillSmallHeader() const;

        // Creates a fat header with information from this instance.
        IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT FillFatHeader() const;

        // Label to the first instruction belonging to the try block
        constexpr Label TryOffset() const noexcept
        {
            return m_tryOffset;
        }

        // Label to the first instruction after the try block
        constexpr Label TryEndOffset() const noexcept
        {
            return m_tryEndOffset;
        }

        // Label to the first instruction of the catch or finally block
        constexpr Label HandlerOffset() const noexcept
        {
            return m_handlerOffset;
        }

        // Label to the first instruction after the catch or finally block
        constexpr Label HandlerEndOffset() const noexcept
        {
            return m_handlerEndOffset;
        }

        // mdTypeDef if it is a catch without when clause.
        // Otherwise, Label to the first instruction of the when filter.
        constexpr std::variant<mdTypeDef, Label> Handler() const noexcept
        {
            return m_handler;
        }

        // True, if this clause represents try-finally or try-fault.
        // False, if this clause represents try-catch or try-catch with when.
        constexpr bool IsFinally() const noexcept
        {
            return (m_flags & CorExceptionFlag::COR_ILEXCEPTION_CLAUSE_FINALLY) != 0
                || (m_flags & CorExceptionFlag::COR_ILEXCEPTION_CLAUSE_FAULT) != 0;
        }

        // If the given label is the same as boundaries of
        // protected block or handler block, writes the
        // boundary description to the given output stream.
        // @param target : the output stream.
        // @param maybeBelongsToThisClause : the label which
        //     possibly the same as TryOffset, TryEndOffset,
        //     HandlerOffset, or HandlerEndOffset.
        template <typename TChar>
        std::basic_ostream<TChar>& WriteTryCatchIfNeeded(
            std::basic_ostream<TChar>& target,
            const Label maybeBelongsToThisClause) const
        {
            if (TryOffset() == maybeBelongsToThisClause)
            {
                target << L"try" << std::endl << L"{" << std::endl;
            }
            else if (TryEndOffset() == maybeBelongsToThisClause)
            {
                target << L"}" << std::endl;
            }
            else if (HandlerOffset() == maybeBelongsToThisClause)
            {
                if (IsFinally())
                {
                    target << L"finally";
                }
                else
                {
                    target << L" catch (";
                    std::visit([&target](const auto& methodOrLabel)
                        {
                            if constexpr (std::is_same_v<std::decay_t<decltype(methodOrLabel)>, Label>)
                            {
                                target << L"filter ";
                            }
                            else
                            {
                                target << L"ofType ";
                            }

                            target << InSquareBrackets(methodOrLabel);
                        },
                        Handler());
                    target << L")";
                }
                target << std::endl;
                target << L"{" << std::endl;
            }
            else if (HandlerEndOffset() == maybeBelongsToThisClause)
            {
                target << L"}" << std::endl;
            }

            return target;
        }
    };
}
