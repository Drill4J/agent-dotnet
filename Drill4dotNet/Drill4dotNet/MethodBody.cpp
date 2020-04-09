#include "pch.h"
#include "MethodBody.h"

#include <unordered_map>

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
        // Jump extracted from the opcode argument.
        struct Jump
        {
            // The label associated with the jump.
            Label Label;

            // The offset of the jump, in bytes,
            // relative to the instruction immediately after
            // the branching instruction.
            LongJump::Offset Offset;
        };

        // The instructions stream to store parsed instructions.
        InstructionStream& Target;

        // The tool to emit new labels.
        LabelCreator& LabelCreator;

        // For each position in Target, has corresponding vector
        // of jumps from that position.
        std::vector<std::vector<Jump>> JumpOffsets;

    private:
        // Adds a new Jump with the given offset to JumpOffsets.
        // Returns a Jump to store in a branching instruction argument.
        // TJump : ShortJump or LongJump
        // @param jumpOffset : the offset in bytes.
        template <typename TJump>
        TJump CreateJump(const LongJump::Offset jumpOffset)
        {
            const Label label { LabelCreator.CreateLabel() };
            JumpOffsets.back().push_back( Jump { label, jumpOffset } );
            return TJump { label };
        }

    public:

        // Creates a new converter with the given values.
        ArgumentConverter(
            InstructionStream& target,
            Drill4dotNet::LabelCreator& labelCreator) noexcept
            : Target { target },
            LabelCreator { labelCreator }
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
            JumpOffsets.emplace_back();
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
                auto begin = static_cast<LongJump::Offset*>(rawArgument.switch_.targets);
                auto count = rawArgument.switch_.count;
                auto end = begin + count;

                std::vector<LongJump> jumpTable{};
                jumpTable.reserve(count);
                std::for_each(begin, end, [this, &jumpTable](LongJump::Offset offset)
                    {
                        jumpTable.push_back(CreateJump<LongJump>(offset));
                    });

                argument = jumpTable;
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::InlinePhi>)
            {
                auto begin = (uint16_t*)(rawArgument.phi.vars);
                auto end = begin + rawArgument.phi.count;
                argument = std::vector<uint16_t>(begin, end);
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::ShortInlineBrTarget>)
            {
                const LongJump::Offset offset { static_cast<ShortJump::Offset>(
                    rawArgument.i) };
                argument = CreateJump<TArgument>(offset);
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::InlineBrTarget>)
            {
                argument = CreateJump<TArgument>(rawArgument.i);
            }
            else if constexpr (
                std::is_same_v<TArgument, OpCodeArgumentType::InlineMethod>)
            {
                argument = OpCodeArgumentType::InlineMethod {
                    static_cast<OpCodeArgumentType::InlineMethod::TokenType>(rawArgument.i),
                    0,
                    false };
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
        // Uses
        // const BYTE* OpInfo::fetch(const BYTE * instrPtr, OpArgsVal * args)
        if (bodyBytes.size() < size_t { m_header.CodeSize() } + m_header.Size())
        {
            throw std::runtime_error("Unexpected end of method body bytes.");
        }

        OpInfo parser{};
        const auto begin = reinterpret_cast<const BYTE*>(bodyBytes.data() + m_header.Size());
        const auto end = begin + m_header.CodeSize();
        auto currentInstruction{ begin };
        ArgumentConverter<OpArgsVal> converter { m_stream, m_labelCreator };
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

            default:
                throw std::runtime_error("Unknown Intermediate Language instruction");
            }
        }

        if (m_header.HasExceptionsSections())
        {
            auto codeEnd = bodyBytes.cbegin() + m_header.Size() + m_header.CodeSize();
            auto sectionBeginning = codeEnd + AdvanceToBoundary<4>(codeEnd, bodyBytes);
            bool moreSections;
            do
            {
                ExceptionsSection section(
                    sectionBeginning,
                    bodyBytes.cend(),
                    m_stream,
                    m_labelCreator);
                m_exceptionSections.push_back(section);
                moreSections = section.HasMoreSections();
            } while (moreSections);
        }

        size_t instructionsCounter { 0 };
        for (const auto& jumpTable : converter.JumpOffsets)
        {
            for (const auto jump : jumpTable)
            {
                auto insertAt { ResolveJumpOffset(
                    m_stream,
                    GetNthInstruction(m_stream, instructionsCounter),
                    jump.Offset) };

                if (insertAt == m_stream.cend())
                {
                    throw std::runtime_error("Could not find an instruction by the given jump offset.");
                }

                m_stream.insert(insertAt, jump.Label);
            }

            ++instructionsCounter;
        }
    }

    std::vector<std::byte> MethodBody::Compile() const
    {
        std::vector<std::byte> result{};
        result.reserve(m_header.Size() + m_header.CodeSize());

        m_header.AppendToBytes(result);

        for (ConstStreamPosition current = m_stream.cbegin(); current != m_stream.cend(); ++current)
        {
            const StreamElement& variant{ *current };
            if (const OpCodeVariant* instruction = std::get_if<OpCodeVariant>(&variant)
                ; instruction != nullptr)
            {
                instruction->m_code.AppendToVector(result);
                std::visit(
                    [this, current, &result](const auto argument)
                    {
                        using T = std::remove_cv_t<decltype(argument)>;
                        if constexpr (std::is_same_v<T, std::monostate>)
                        {
                            return;
                        }
                        else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineSwitch>)
                        {
                            AppendAsBytes(result, static_cast<AbsoluteOffset>(argument.size()));
                            for (const auto jump : argument)
                            {
                                auto labelPosition = FindLabel(m_stream, jump.Label());
                                if (labelPosition == m_stream.cend())
                                {
                                    throw std::logic_error("Compilation failed: unresolved label.");
                                }

                                AppendAsBytes(
                                    result,
                                    CalculateJumpOffset(m_stream, current, labelPosition));
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
                        else if constexpr (std::is_same_v<T, OpCodeArgumentType::ShortInlineBrTarget>)
                        {
                            auto labelPosition = FindLabel(m_stream, argument.Label());
                            if (labelPosition == m_stream.cend())
                            {
                                throw std::logic_error("Compilation failed: unresolved label.");
                            }

                            const LongJump::Offset offset = CalculateJumpOffset(m_stream, current, labelPosition);
                            AppendAsBytes(
                                result,
                                static_cast<const ShortJump::Offset>(offset));
                        }
                        else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineBrTarget>)
                        {
                            auto labelPosition = FindLabel(m_stream, argument.Label());
                            if (labelPosition == m_stream.cend())
                            {
                                throw std::logic_error("Compilation failed: unresolved label.");
                            }

                            const LongJump::Offset offset = CalculateJumpOffset(m_stream, current, labelPosition);
                            AppendAsBytes(
                                result,
                                offset);
                        }
                        else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineMethod>)
                        {
                            AppendAsBytes(result, argument.MetadataToken);
                        }
                        else
                        {
                            AppendAsBytes(result, argument);
                        }
                    },
                    instruction->m_argument);
            }
        }

        if (m_header.HasExceptionsSections())
        {
            const ptrdiff_t padding { AdvanceToBoundary<4>(result.cend(), result) };
            for (size_t i = 0; i != padding; ++i)
            {
                result.push_back(std::byte { 0 });
            }

            for (const auto& section : m_exceptionSections)
            {
                section.AppendToBytes(result);
            }
        }

        return result;
    }

    void MethodBody::Insert(
        const ConstStreamPosition position,
        const OpCodeVariant opcode)
    {
        // Dangerous: maxstack
        const int64_t newCodeSize = int64_t { m_header.CodeSize() } + opcode.SizeWithArgument();
        if (!m_header.IsValidCodeSize(newCodeSize))
        {
            throw std::overflow_error("There is no room in the target method to insert the new instruction");
        }

        m_header.SetCodeSize(static_cast<AbsoluteOffset>(newCodeSize));
        m_stream.insert(position, opcode);
        TurnJumpsToLongIfNeeded();
    }

    // Indicates that the instruction is not a
    // short branching instruction.
    using InstructionCannotMadeLong = void;

    // Gets the long branching instruction corresponding to the
    // given short branching instruction.
    // TOpCode : instruction type.
    template <typename TOpCode>
    class ToLongBranchInstruction
    {
    public:
        // The long branching instruction, or
        // InstructionCannotMadeLong if TOpCode is not a short
        // branching instruction.
        using LongInstruction = InstructionCannotMadeLong;
    };

    // Declare specializations for all short branching instructions we know

#define DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(longName) \
    template <> \
    class ToLongBranchInstruction< OpCode::CEE_ ## longName ## _S> \
    { \
    static_assert(std::is_same_v< \
        OpCode::CEE_ ## longName ## _S :: ArgumentType, \
        OpCodeArgumentType::ShortInlineBrTarget>); \
\
    static_assert(std::is_same_v< \
        OpCode::CEE_ ## longName ## :: ArgumentType, \
        OpCodeArgumentType::InlineBrTarget>); \
 \
   public: \
        using LongInstruction = OpCode::CEE_ ## longName; \
    };

    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BR)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BRFALSE)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BRTRUE)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BEQ)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BGE)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BGT)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BLE)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BLT)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BNE_UN)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BGE_UN)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BGT_UN)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BLE_UN)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(BLT_UN)
    DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION(LEAVE)

#undef DECLARE_TO_LONG_BRANCH_INSTRUCTION_SPECIALIZATION

    // Check there are no short branching instruction without ToLongBranchInstruction specialization

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
    static_assert( \
        !std::is_same_v< \
            OpCode :: ## canonicalName ## ::ArgumentType, \
            OpCodeArgumentType::ShortInlineBrTarget> \
        || !std::is_same_v< \
            ToLongBranchInstruction<OpCode :: ## canonicalName >::LongInstruction, \
            InstructionCannotMadeLong>);
#include "DefineOpCodesGeneratorSpecializations.h"
#include <opcode.def>
#include "UnDefineOpCodesGeneratorSpecializations.h"
#undef OPDEF_REAL_INSTRUCTION

    bool MethodBody::ConvertJumpInstructionToLongIfNeeded(const StreamPosition instructionPosition)
    {
        bool result{ false };

        if (const OpCodeVariant* const instruction = std::get_if<OpCodeVariant>(&*instructionPosition)
            ; instruction != nullptr)
        {
            instruction->Visit(
                [this, instructionPosition, instruction, &result](const auto& opcode)
                {
                    using TLong = typename ToLongBranchInstruction<std::decay_t<decltype(opcode)>>::LongInstruction;
                    if constexpr (std::is_same_v<TLong, InstructionCannotMadeLong>)
                    {
                        return;
                    }
                    else
                    {
                        const Label label = opcode.Argument().Label();
                        const ConstStreamPosition target = FindLabel(m_stream, label);
                        if (target == m_stream.cend())
                        {
                            return;
                        }

                        LongJump::Offset distance = CalculateJumpOffset(m_stream, instructionPosition, target);
                        if (ShortJump::CanSafelyStoreOffset(distance))
                        {
                            return;
                        }

                        OpCodeVariant newInstruction{ TLong(LongJump(label)) };
                        const int64_t newCodeSize = int64_t{ m_header.CodeSize() }
                            + newInstruction.SizeWithArgument()
                            - instruction->SizeWithArgument();

                        if (!m_header.IsValidCodeSize(newCodeSize))
                        {
                            throw std::overflow_error("There is no room in the target method to convert an existing instruction to long variant");
                        }

                        m_header.SetCodeSize(static_cast<AbsoluteOffset>(newCodeSize));
                        *instructionPosition = newInstruction;

                        result = true;
                    }
                });
        }

        return result;
    }

    void MethodBody::TurnJumpsToLongIfNeeded()
    {
        bool jumpsUpdated;
        do
        {
            jumpsUpdated = false;

            for (auto current = m_stream.begin(); current != m_stream.end(); ++current)
            {
                jumpsUpdated |= ConvertJumpInstructionToLongIfNeeded(current);
            }
        }
        while (jumpsUpdated);
    }

    Label MethodBody::CreateLabel()
    {
        return m_labelCreator.CreateLabel();
    }

    void MethodBody::MarkLabel(
        const ConstStreamPosition target,
        const Label label)
    {
        const ConstStreamPosition existingLabelPosition { FindLabel(m_stream, label) };
        if (existingLabelPosition != m_stream.cend())
        {
            throw std::logic_error("This label has already been marked");
        }

        m_stream.insert(target, label);
        TurnJumpsToLongIfNeeded();
    }

    int MethodBody::CalculateMaxStack() const
    {
        ConstStreamPosition current{ SkipLabels(m_stream.cbegin(), m_stream.cend()) };
        std::unordered_map<ptrdiff_t, int> stackCounts{ { current - m_stream.cbegin(), 0 } };
        while (current != m_stream.cend())
        {
            const OpCodeVariant& instruction{ std::get<OpCodeVariant>(*current) };
            instruction.Visit(
                [this, &stackCounts, &current](const auto& opcode)
            {
                using T = std::decay_t<decltype(opcode)>;
                int currentStackCount = stackCounts[current - m_stream.cbegin()];
                if constexpr (T::IsStackPopBehaviorKnown)
                {
                    currentStackCount -= T::ItemsPoppedFromStack;
                }
                else if constexpr (std::is_same_v<T, OpCode::CEE_RET>)
                {
                    if (hasReturnValue)
                    {
                        --currentStackCount;
                    }

                    if (currentStackCount != 0)
                    {
                        throw std::runtime_error("Invalid Intermediate Language: evaluation stack is not empty after return");
                    }
                }
                else if constexpr (std::is_same_v<T::ArgumentType, OpCodeArgumentType::InlineMethod>)
                {
                    currentStackCount -= opcode.Argument().ParametersCount;
                }

                if constexpr (T::IsStackPushBehaviorKnown)
                {
                    currentStackCount += T::ItemsPushedToStack;
                }
                else
                {
                    if (opcode.Argument().HasReturnValue)
                    {
                        ++currentStackCount;
                    }
                }

                static_assert(false, L"Move to GetInstructionFlowDestinations and use");
                if constexpr (std::is_same_v<T::ArgumentType, OpCodeArgumentType::InlineSwitch>)
                {

                }
                else if constexpr (std::is_same_v<T::ArgumentType, OpCodeArgumentType::ShortInlineBrTarget>
                    || std::is_same_v<T::ArgumentType, OpCodeArgumentType::InlineBrTarget>)
                {
                    static_assert(false, L"Take FLOW into account");
                    current = SkipLabels(
                        FindLabel(stream, opcode.Argument().Label()),
                        stream.cend());
                }
                else
                {
                    current = FindNextInstruction(current, stream.cend());
                }

                stackCounts[current - stream.cbegin()] = currentStackCount;
            });
        }

        return std::max_element(
            stackCounts.cbegin(),
            stackCounts.cend(),
            [](
                const std::pair<const ptrdiff_t, int>& x,
                const std::pair<const ptrdiff_t, int>& y)
        {
            return x.second < y.second;
        })->second;
    }

    void MethodBody::UpdateMaxStack()
    {
        static_assert(false, L"Must implement");
    }


    std::variant<
        std::array<ConstStreamPosition, 0>,
        std::array<ConstStreamPosition, 1>,
        std::array<ConstStreamPosition, 2>,
        std::vector<ConstStreamPosition>>
    MethodBody::GetInstructionFlowDestinations(const ConstStreamPosition position) const
    {
        static_assert(false, L"Must implement");
        return {};
    }
}

