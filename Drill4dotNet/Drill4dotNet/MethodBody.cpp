#include "pch.h"
#include "MethodBody.h"

// The TARGET_* defines are only needed for <opinfo.cpp>
#ifdef _M_AMD64
#define TARGET_AMD64
#else
#define TARGET_X86
#endif

#include <opinfo.cpp>

#ifdef _M_AMD64
#undef TARGET_AMD64
#else
#undef TARGET_X86
#endif

#if defined(_DEBUG)

// The <opinfo.cpp> requires this function to be defined, have to add a stub.
static void DbgAssertDialog(const char* szFile, int iLine, const char* szExpr)
{
}

#endif

namespace Drill4dotNet
{
    // Helps to convert OpArgsVal into values for MethodBody.
    // Provides method AppendStream, which can be called from
    // a macro for different inline argument types in an uniform way.
    template <>
    class MethodBody::ArgumentConverter<OpArgsVal>
    {
    public:
        // The instructions stream to store parsed instructions.
        std::vector<OpCodeVariant>& Target;

        // Creates a new converter with the given values.
        ArgumentConverter(
            std::vector<OpCodeVariant>& target) noexcept
            : Target { target }
        {
        }

        // Appends the next instruction to the stream.
        // TArgument : the instruction's inline argument type.
        // @param firstByte : the first byte of the instruction's code.
        // @param secondByte : the second byte of the instruction's code.
        // @param rawArgument : the value to construct the instruction's
        //     inline argument from.
        template <typename TArgument>
        void AppendStream(
            const std::byte firstByte,
            const std::byte secondByte,
            const OpArgsVal& rawArgument)
        {
            OpCodeVariant::InstructionCode code{ firstByte, secondByte };
            OpCodeVariant::VariantType argument{};
            if constexpr (std::is_same_v<TArgument, OpCodeArgumentType::InlineNone>)
            {
                argument = std::monostate{};
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::ShortInlineR>
                || std::is_same_v<TArgument, OpCodeArgumentType::InlineR>)
            {
                argument = static_cast<TArgument>(rawArgument.r);
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::InlineI8>)
            {
                argument = static_cast<TArgument>(rawArgument.i8);
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::InlineSwitch>)
            {
                auto begin = (int32_t*)(rawArgument.switch_.targets);
                auto end = begin + rawArgument.switch_.count;

                argument = std::vector<int32_t>(begin, end);
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::InlinePhi>)
            {
                auto begin = (uint16_t*)(rawArgument.phi.vars);
                auto end = begin + rawArgument.phi.count;
                argument = std::vector<uint16_t>(begin, end);
            }
            else
            {
                argument = static_cast<TArgument>(rawArgument.i);
            }

            Target.emplace_back(OpCodeVariant(code, argument));
        }
    };

    MethodBody::MethodBody(const std::vector<std::byte>& bodyBytes)
        : m_header(bodyBytes)
    {
        m_instructions = Decompile(
            bodyBytes,
            m_header.Size(),
            m_header.CodeSize());
    }

    std::vector<OpCodeVariant> MethodBody::Decompile(
        const std::vector<std::byte>& bodyBytes,
        uint8_t headerSize,
        const AbsoluteOffset codeSize)
    {
        // Uses
        // const BYTE* OpInfo::fetch(const BYTE * instrPtr, OpArgsVal * args)
        OpInfo parser{};
        std::vector<OpCodeVariant> result{};
        const auto begin = reinterpret_cast<const BYTE*>(bodyBytes.data() + headerSize);
        const auto end = begin + codeSize;
        auto currentInstruction{ begin };
        ArgumentConverter<OpArgsVal> converter { result };
        while (currentInstruction < end)
        {
            OpArgsVal inlineArguments;
            currentInstruction = parser.fetch(
                currentInstruction,
                &inlineArguments);
            switch (parser.getOpcode())
            {
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
            case opcode_t:: ## canonicalName : \
            { \
                converter.AppendStream<OpCodeArgumentType :: ## inlineArgumentType >( \
                    std::byte { byte1 }, \
                    std::byte { byte2 }, \
                    inlineArguments); \
            } \
\
            break;
#include "DefineOpCodesGeneratorSpecializations.h"
#include <opcode.def>
#include "UnDefineOpCodesGeneratorSpecializations.h"
#undef OPDEF_REAL_INSTRUCTION
            }
        }

        return result;
    }

    std::vector<std::byte> MethodBody::Compile() const
    {
        std::vector<std::byte> result{};
        result.reserve(m_header.Size() + m_header.CodeSize());

        m_header.AppendToBytes(result);

        for (const auto& instruction : m_instructions)
        {
            instruction.m_code.AppendToVector(result);
            std::visit(
                [&result](const auto argument)
                {
                    using T = std::remove_cv_t<decltype(argument)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                    {
                        return;
                    }
                    else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineSwitch>)
                    {
                        AppendAsBytes(result, static_cast<uint32_t>(argument.size()));
                        for (const auto label : argument)
                        {
                            AppendAsBytes(result, label);
                        }
                    }
                    else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlinePhi>)
                    {
                        AppendAsBytes(result, static_cast<uint8_t>(argument.size()));
                        for (const auto var : argument)
                        {
                            AppendAsBytes(result, var);
                        }
                    }
                    else
                    {
                        AppendAsBytes(result, argument);
                    }
                },
                instruction.m_argument);
        }

        // Dangerous: need to add processing of exception clauses;

        return result;
    }

    void MethodBody::Insert(
        const std::vector<OpCodeVariant>::const_iterator position,
        const OpCodeVariant opcode)
    {
        // Dangerous: need to update branching instructions, exceptions handling, and maxstack
        const int64_t newCodeSize = int64_t { m_header.CodeSize() } + opcode.SizeWithArgument();
        if (!m_header.IsValidCodeSize(newCodeSize))
        {
            throw std::overflow_error("There is no room in the target method to insert the new instruction");
        }

        m_header.SetCodeSize(static_cast<AbsoluteOffset>(newCodeSize));
        m_instructions.insert(position, opcode);
    }
}

