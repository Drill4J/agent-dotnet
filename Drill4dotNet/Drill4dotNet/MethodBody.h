#pragma once

#include "OpCodes.h"
#include "InstructionStream.h"
#include "ExceptionsSection.h"
#include "MethodHeader.h"

namespace Drill4dotNet
{
    // Object representation of method body bytes.
    // Allows converting back to bytes representation.
    // Reference: ECMA-335, Common Language Infrastructure,
    // part II.25.4 Common Intermediate Language physical layout
    // https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf
    class MethodBody
    {
    private:
        // The header storing information about other method structures.
        MethodHeader m_header;

        // The instructions and labels.
        InstructionStream m_stream;

        // The tool to emit new labels.
        LabelCreator m_labelCreator;

        // Sections with descriptions of try-catch and try-finally clauses.
        std::vector<ExceptionsSection> m_exceptionSections;

        // Specialized for .net's OpArgsVal to allow
        // getting instruction arguments from it.
        template <typename TOpArgsVal>
        class ArgumentConverter;

        // Converts to the long form all short branching instructions,
        // which jumps are too far to be stored in these short instructions.
        void TurnJumpsToLongIfNeeded();

        // Helper function for TurnJumpsToLongIfNeeded().
        // If the given instructions stream position contains an instruction,
        // and the instruction is a short branching instruction,
        // and the jump in the instruction is too far to be stored in the instruction,
        // replaces the instruction with the long jump alternative.
        // Returns true if the instruction has been replaced.
        bool ConvertJumpInstructionToLongIfNeeded(const StreamPosition instructionPosition);

    public:
        // Creates the object representation of the method body.
        // @param bodyBytes : the bytes of method body.
        explicit MethodBody(const std::vector<std::byte>& bodyBytes);

        // Makes a binary representation of the method body.
        std::vector<std::byte> Compile() const;

        // Inserts the given instruction into the instructions list.
        // @param position : the point at which to insert,
        //    must be in range [begin(), end()]
        // @param opcode : the instruction to insert.
        void Insert(
            const ConstStreamPosition position,
            const OpCodeVariant opcode);

        // Declares a new Label.
        // For each created label MarkLabel must be called exactly
        // 1 time to define the location the label points to.
        Label CreateLabel();

        // Defines where the given label points to.
        // @param target : the position to which the label targets.
        // @param label : the label which has not been yet defined.
        void MarkLabel(
            const ConstStreamPosition target,
            const Label label);

        // Gets the beginning of the instructions list.
        ConstStreamPosition begin() const& noexcept
        {
            return m_stream.cbegin();
        }

        // Gets the ending of the instructions list.
        ConstStreamPosition end() const& noexcept
        {
            return m_stream.cend();
        }

        // Gets the instructions stream for reading.
        const InstructionStream& Stream() const& noexcept
        {
            return m_stream;
        }

        // Gets the instructions stream.
        InstructionStream Stream() && noexcept
        {
            return std::move(m_stream);
        }

        // Gets the information about try-catch and try-finally clauses for reading.
        const std::vector<ExceptionsSection>& ExceptionSections() const& noexcept
        {
            return m_exceptionSections;
        }

        // Gets the information about try-catch and try-finally clauses.
        std::vector<ExceptionsSection> ExceptionSections() && noexcept
        {
            return std::move(m_exceptionSections);
        }
    };

    // Allows printing the instructions to standard streams.
    template <typename TChar>
    std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, const MethodBody& data)
    {
        for (const auto& element : data.Stream())
        {
            std::visit([&target, &sections = data.ExceptionSections()](const auto& instructionOrLabel)
                {
                    using T = std::decay_t<decltype(instructionOrLabel)>;
                    target << instructionOrLabel;

                    if constexpr (std::is_same_v<T, OpCodeVariant>)
                    {
                        target << std::endl;
                    }
                    else
                    {
                        target << L": ";

                        for (const auto& section : sections)
                        {
                            for (const auto& clause : section.Clauses())
                            {
                                clause.WriteTryCatchIfNeeded(target, instructionOrLabel);
                            }
                        }
                    }
                },
                element);
        }

        return target;
    }
}

