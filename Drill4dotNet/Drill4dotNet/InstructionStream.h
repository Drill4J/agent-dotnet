#pragma once

#include "OpCodes.h"

namespace Drill4dotNet
{
    // An opcode in an instructions stream.
    using StreamElement = OpCodeVariant;

    // Instructions stream - sequence of opcodes, with added
    // labels before some of the opcodes.
    using InstructionStream = std::vector<StreamElement>;

    // Position in the instructions stream,
    // which content can be modified.
    using StreamPosition = InstructionStream::iterator;

    // Position in the instructions stream,
    // which content cannot be modified.
    using ConstStreamPosition = InstructionStream::const_iterator;
}

