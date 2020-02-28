#include "pch.h"
#include "InstructionStream.h"

#include <cassert>
#include <numeric>

namespace Drill4dotNet
{
    // If the given stream element is an instruction, returns
    // its size with argument, in bytes. Returns 0 for label.
    // @param position : instruction or label.
    CodeSize InstructionSize(const StreamElement& position) noexcept
    {
        return std::visit(
            [](const auto& instruction)->CodeSize
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(instruction)>, OpCodeVariant>)
                {
                    return instruction.SizeWithArgument();
                }

                return 0;
            },
            position);
    }

    // Walks the given part of instructions stream,
    // until exactly the given amount of bytes is passed.
    // Returns the reached position.
    // @param start : the position where to start walk from
    // @param end : points immediately after the last element to walk.
    //     Returned if the position is not found.
    // @param target : the offset to reach.
    ConstStreamPosition ReachPositiveOffset(
        const ConstStreamPosition start,
        const ConstStreamPosition end,
        const CodeSize target)
    {
        CodeSize::NonOverflowingType distance { 0 };
        return std::find_if(
            start,
            end,
            [&distance, target](const StreamElement& instruction)
            {
                if (distance == CodeSize::NonOverflowingType{ target })
                {
                    return true;
                }

                distance = distance + InstructionSize(instruction);
                return false;
            });
    }

    // Walks the given part of instructions stream, in reverse
    // order, until exactly the given amount of bytes is passed.
    // Returns the reached position.
    // @param start : the position where to start walk from
    // @param begin : points at the last element to walk.
    // @param end : points immediately after the last element of
    //     the instruction stream. Returned if the position is not found.
    // @param target : the offset to reach. Must be negative.
    ConstStreamPosition ReachNegativeOffset(
        const ConstStreamPosition start,
        const ConstStreamPosition begin,
        const ConstStreamPosition end,
        const LongJump::Offset target)
    {
        CodeSize::NonOverflowingType distance { 0 };
        ConstStreamPosition current { start };
        while (current != begin)
        {
            --current;
            distance = distance - InstructionSize(*current);
            if (distance == target)
            {
                return current;
            }
        }

        return end;
    }

    ConstStreamPosition ResolveJumpOffset(
        const InstructionStream& stream,
        const ConstStreamPosition source,
        const LongJump::Offset offset)
    {
        if (source == stream.cend())
        {
            return stream.cend();
        }

        const ConstStreamPosition origin = FindNextInstruction(source, stream.cend());

        if (offset == 0)
        {
            return origin;
        }

        ConstStreamPosition target;
        const bool forward = offset > 0;
        if (forward)
        {
            if (origin == stream.cend())
            {
                return stream.cend();
            }

            target = ReachPositiveOffset(
                origin,
                stream.cend(),
                offset);
        }
        else
        {
            target = ReachNegativeOffset(
                origin,
                stream.cbegin(),
                stream.cend(),
                offset);
        }

        return SkipLabels(target, stream.end());
    }

    ConstStreamPosition SkipLabels(
        const ConstStreamPosition startPoint,
        const ConstStreamPosition end)
    {
        return std::find_if(
            startPoint,
            end,
            [](const StreamElement& instructionOrLabel)
            {
                return std::holds_alternative<OpCodeVariant>(instructionOrLabel);
            });
    }

    ConstStreamPosition FindNextInstruction(
        const ConstStreamPosition startPoint,
        const ConstStreamPosition end)
    {
        auto currentInstruction = SkipLabels(startPoint, end);
        if (currentInstruction == end)
        {
            return end;
        }

        ++currentInstruction;
        if (currentInstruction == end)
        {
            return end;
        }

        return SkipLabels(currentInstruction, end);
    }

    ConstStreamPosition ResolveAbsoluteOffset(
        const InstructionStream& stream,
        const CodeSize offset)
    {
        return SkipLabels(
            ReachPositiveOffset(
                stream.cbegin(),
                stream.cend(),
                offset),
            stream.cend());
    }

    ConstStreamPosition GetNthInstruction(const InstructionStream& stream, const size_t number)
    {
        ptrdiff_t instructions{ 0 };
        return SkipLabels(
            std::find_if(
                stream.cbegin(),
                stream.cend(),
                [&instructions, number](const StreamElement& variant)
                {
                    const bool targetReached = instructions == number;
                    if (!targetReached && std::holds_alternative<OpCodeVariant>(variant))
                    {
                        ++instructions;
                    }

                    return targetReached;
                }),
            stream.cend());
    }

    // Calculates the distance, in bytes,
    // between two given positions in the instructions stream.
    // 0 corresponds to the from location.
    // @param from : the first instruction to count.
    // @param to : the first instruction not to count.
    //     Must be after from.
    CodeSize::NonOverflowingType CalculateDistance(
        const ConstStreamPosition from,
        const ConstStreamPosition to)
    {
        assert(from <= to);

        return std::accumulate(
            from,
            to,
            CodeSize::NonOverflowingType { 0 },
            [](const CodeSize::NonOverflowingType sum, const StreamElement& variant) noexcept
            {
                return sum + InstructionSize(variant);
            });
    }

    LongJump::Offset CalculateJumpOffset(
        const InstructionStream& stream,
        const ConstStreamPosition from,
        const ConstStreamPosition to)
    {
        const ConstStreamPosition origin = FindNextInstruction(from, stream.cend());
        const int64_t result = origin > to
            ? int64_t { -1 } * CalculateDistance(to, origin)
            : CalculateDistance(origin, to);

        if (!LongJump::CanSafelyStoreOffset(result))
        {
            throw std::overflow_error("The relative jump is too far and cannot be represented as a long jump");
        }

        return static_cast<LongJump::Offset>(result);
    }

    CodeSize CalculateAbsoluteOffset(
        const InstructionStream& instructionStream,
        const ConstStreamPosition to)
    {
        return CalculateDistance(instructionStream.cbegin(), to);
    }

    ConstStreamPosition FindLabel(
        const InstructionStream& stream,
        const Label label)
    {
        return std::find_if(
            stream.cbegin(),
            stream.cend(),
            [label](const auto& maybeLabel)
            {
                if (const Label* const candidate = std::get_if<Label>(&maybeLabel)
                    ; candidate != nullptr)
                {
                    return *candidate == label;
                }

                return false;
            });
    }

    Label LabelCreator::CreateLabel() noexcept
    {
        return Label(m_nextUnusedLabelId++);
    }
}
