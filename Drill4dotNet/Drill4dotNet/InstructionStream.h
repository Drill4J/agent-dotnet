#pragma once

#include "OpCodes.h"

namespace Drill4dotNet
{
    // An item in an instructions stream. Can be an opcode
    // or a label.
    using StreamElement = std::variant<OpCodeVariant, Label>;

    // Instructions stream - sequence of opcodes, with added
    // labels before some of the opcodes.
    using InstructionStream = std::vector<StreamElement>;

    // Position in the instructions stream,
    // which content can be modified.
    using StreamPosition = InstructionStream::iterator;

    // Position in the instructions stream,
    // which content cannot be modified.
    using ConstStreamPosition = InstructionStream::const_iterator;

    // Searches for a specific instruction in the
    // given range of instructions stream.
    // TOpCode : type of instruction to search for.
    // @param start : points to the first element of
    //     the search range.
    // @param end : points to the position immediately
    //     after the last element of the search range.
    template<
        typename TOpCode,
        std::enable_if_t<OpCodeVariant::IsOpCodeClass<TOpCode>, int> = 0>
        ConstStreamPosition FindInstruction(
            const ConstStreamPosition start,
            const ConstStreamPosition end)
    {
        return std::find_if(
            start,
            end,
            [](const StreamElement& current)
            {
                if (const OpCodeVariant* instruction = std::get_if<OpCodeVariant>(&current)
                    ; instruction != nullptr)
                {
                    return instruction->HoldsAlternative<TOpCode>();
                }

                return false;
            });
    }

    // Searches for an instruction in the given
    // instructions stream, which would be
    // the jump target from the given instruction.
    // Returns cend() of stream, if no corresponding
    // position found.
    // @param stream : the instructions stream to search in.
    // @param source : the instruction to start search from.
    // @param offset : signed offset from source, in bytes, 0
    //     corresponds the instruction immediately after source.
    ConstStreamPosition ResolveJumpOffset(
        const InstructionStream& stream,
        const ConstStreamPosition source,
        const LongJump::Offset offset);

    // Searches for an instruction in the given
    // instructions stream, which would be
    // located in the given amount of bytes from the
    // beginning of the method.
    // Returns cend() of stream, if no corresponding
    // position found.
    // @param stream : the instructions stream to search in.
    // @param offset : unsigned offset, in bytes, 0 corresponds
    //     the first instruction of the method.
    ConstStreamPosition ResolveAbsoluteOffset(
        const InstructionStream& stream,
        const AbsoluteOffset offset);

    // Searches for the position of the N-th instruction from
    // the beginning of the given instructions stream.
    // Returns cend() of stream, if no corresponding
    // position found.
    // @param stream : the instructions stream to search in.
    // @param number : the number of instruction to retrieve.
    ConstStreamPosition GetNthInstruction(const InstructionStream& stream, const size_t number);

    // Calculates which jump value should be used to
    // transfer control between two given instructions.
    // @param stream : the instructions stream the instructions belong to.
    // @param from : the instruction from which jump is performed.
    // @param to : the jump target instruction.
    LongJump::Offset CalculateJumpOffset(
        const InstructionStream& stream,
        const ConstStreamPosition from,
        const ConstStreamPosition to);

    // Calculate distance in bytes between the
    // method start and the given instruction.
    // @param stream : the instructions stream the instruction belongs to.
    // @param to : the instruction to calculate distance to.
    AbsoluteOffset CalculateAbsoluteOffset(
        const InstructionStream& instructionStream,
        const ConstStreamPosition to);

    // Returns the position in the instruction stream,
    // where the given label is located.
    // @param stream : the instructions stream to search in.
    // @param label : the label to search for.
    ConstStreamPosition FindLabel(
        const InstructionStream& stream,
        const Label label);

    // Skips all the labels and returns the
    // first instruction, starting from the
    // given point in the instructions stream.
    // @param startPoint : the search start position
    // @param end : points to the position immediately
    //     after the last element of the search range.
    //     Returned if an instruction not found.
    ConstStreamPosition SkipLabels(
        const ConstStreamPosition startPoint,
        const ConstStreamPosition end);

    // Returns the position of the second instruction,
    // starting from the given point in the instructions stream.
    // Skips all labels.
    // @param startPoint : the search start position
    // @param end : points to the position immediately
    //     after the last element of the search range.
    //     Returned if the instruction not found.
    ConstStreamPosition FindNextInstruction(
        const ConstStreamPosition startPoint,
        const ConstStreamPosition end);

    // Tool to emit new labels.
    class LabelCreator
    {
    private:
        // Stores the id of the next label to emit.
        Label::Id m_nextUnusedLabelId { 0 };

    public:
        // Emits a new label.
        Label CreateLabel() noexcept;
    };
}

