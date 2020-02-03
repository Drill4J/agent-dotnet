#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

namespace Drill4dotNet
{
    // Defines possible options for type of
    // an OpCode inline argument, for reference see
    // https://docs.microsoft.com/en-us/dotnet/api/system.reflection.emit.operandtype?view=netframework-4.8
    class OpCodeArgumentType
    {
    public:
        using InlineNone = void;
        using InlineVar = uint16_t;
        using InlineI = int32_t;
        using InlineR = double;
        using InlineBrTarget = int32_t;
        using InlineI8 = int64_t;
        using InlineMethod = uint32_t;
        using InlineField = uint32_t;
        using InlineType = uint32_t;
        using InlineString = uint32_t;
        using InlineSig = uint32_t;
        using InlineRVA = uint32_t;
        using InlineTok = uint32_t;
        using InlineSwitch = std::vector<int32_t>;
        using InlinePhi = std::vector<uint16_t>;
        using ShortInlineVar = uint8_t;
        using ShortInlineI = int8_t;
        using ShortInlineR = float;
        using ShortInlineBrTarget = int8_t;
    };

    // Common base class for all opcodes.
    // Intentionally left empty.
    // Should only be used for type compatibility checking.
    class OpCodeTag
    {
    };

    // Base class with interface methods for all opcodes.
    // Not intended for direct usage by clients of the header.
    // Uses CRTP ( https://www.fluentcpp.com/2017/05/12/curiously-recurring-template-pattern/ )
    // to provide methods and values specific to the opcode.
    // Parameters:
    // TOpCode - the opcode to produce values for.
    // TOpCodeTemplate - the OpCode for a friend declaration.
    // TArgument - the type of the OpCode inline argument.
    template <
        typename TOpCode,
        typename TOpCodeTemplate,
        typename TArgument>
    class OpCodeBase : public OpCodeTag
    {
    private:
        // The constructor is hidden in the private section,
        // so only TOpCodeTemplate can inherit this class.
        OpCodeBase()
        {
        }

    public:
        // The type describing a specific instruction.
        using OpCodeType = TOpCode;

        // The type of an inline argument.
        // See OpCodeArgumentType for possible values.
        using ArgumentType = TArgument;

        // Gets the value indicating whether
        // the specific instruction has an inline argument.
        constexpr static bool HasArgument()
        {
            return !std::is_same_v<
                TArgument,
                OpCodeArgumentType::InlineNone>;
        }

        // Gets the printable name of the instruction.
        constexpr static std::wstring_view Name() noexcept
        {
            return TOpCode::CanonicalName;
        }

        friend TOpCodeTemplate;
    };

    // Base class for instructions with an argument.
    // Also not intended for direct use by clients of the header.
    // Also uses CRTP. Parameters:
    // TOpCode : the specific instruction type.
    // TArgument : the type of the inline argument.
    template <typename TOpCode, typename TArgument>
    class OpCode : public OpCodeBase<
        TOpCode,
        OpCode<TOpCode, TArgument>,
        TArgument>
    {
    private:
        TArgument m_argument;

    public:
        // Creates a new opcode value with the
        // given argument.
        // @param argument : the value to use.
        OpCode(const TArgument argument)
            : OpCodeBase(),
            m_argument{ argument }
        {
        }

        // Gets the inline argument.
        TArgument Argument() const
        {
            return m_argument;
        }

        // Sets the inline argument.
        // @param argument : the value to set.
        void SetArgument(TArgument argument)
        {
            return m_argument = argument;
        }
    };

    // Special case when there is no inline argument.
    template <typename TOpCode>
    class OpCode<TOpCode, OpCodeArgumentType::InlineNone> : public OpCodeBase<
        TOpCode,
        OpCode<TOpCode, OpCodeArgumentType::InlineNone>,
        OpCodeArgumentType::InlineNone>
    {
    public:
        OpCode()
            : OpCodeBase()
        {
        }
    };

    // Using the OPDEF_REAL_INSTRUCTION macro, the next invocation
    // of <opcode.def> will generate a set of OpCode_* classes, each
    // corresponding to a specific Intermediate Language instruction.

#define OPDEF_REAL_INSTRUCTION( \
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) \
    class OpCode_ ## canonicalName : public OpCode<\
        OpCode_ ## canonicalName , \
        OpCodeArgumentType:: ## inlineArgumentType > \
    { \
    public: \
        inline static std::wstring_view CanonicalName { L ## stringName }; \
        using OpCode::OpCode; \
    };
#include "DefineOpCodesGeneratorSpecializations.h"
#include <opcode.def>
#include "UnDefineOpCodesGeneratorSpecializations.h"
#undef OPDEF_REAL_INSTRUCTION
}

