#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <optional>
#include <variant>
#include <type_traits>
#include <concepts>

#include "ByteUtils.h"

namespace Drill4dotNet
{
    // Represents a label, which can be put before an
    // instruction to mark a specific location in the
    // instructions stream.
    class Label
    {
    public:
        // Type of the label number.
        using Id = uint32_t;

    private:
        // Stores the label number.
        Id m_id;

        // Creates a new Label with the given id.
        // @param id : the id of the label.
        constexpr Label(Id id)
            : m_id { id }
        {
        }

        // The constructor is not meant to be used directly.
        // LabelCreator should be used instead to create new labels.
        friend class LabelCreator;
    public:

        // The id of the label. Ids of labels
        // inside one instructions stream are unique.
        // But labels from different instructions stream
        // may have the same id.
        constexpr Id GetId() const
        {
            return m_id;
        }
    };

    // Compares the given labels by Id.
    // Returns true if the ids are the same.
    // @param left : the first label to compare.
    // @param right : the second label to compare.
    constexpr bool operator==(const Label left, const Label right)
    {
        return left.GetId() == right.GetId();
    }

    // Compares the given labels by Id.
    // Returns true if the ids are the different.
    // @param left : the first label to compare.
    // @param right : the second label to compare.
    constexpr bool operator!=(const Label left, const Label right)
    {
        return !(left == right);
    }

    // Support for writing labels to standard output streams.
    // @param target : the stream to output data to.
    // @param value : the label to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator <<(
        std::basic_ostream<TChar>& target,
        Label value)
    {
        target << L"Label_" << value.GetId();
        return target;
    }

    // Represents a jump from some instruction in the
    // instructions stream.
    // .NET stores the jump as an offset, in bytes,
    // between instruction immediately following the branching
    // (source), and the jump target instruction. This way, 0
    // will mean the instruction immediately following the
    // branching instruction.
    // TOffset : type of byte offset between source and target
    //     instructions. This type determines how far jump the
    //     inline argument of the source instruction can store.
    template <typename TOffset>
    class Jump
    {
    private:
        // The label before the target instruction.
        Label m_label;

    public:
        // Type of byte offset between source and target
        // instructions. This type determines how far jump the
        // inline argument of the source instruction can store.
        using Offset = TOffset;

        // Gets the value indicating whether the
        // given offset can be stored in the inline
        // argument.
        template <typename TSourceOffset>
        static constexpr bool CanSafelyStoreOffset(const TSourceOffset offset) noexcept
        {
            return !Overflows<TOffset>(offset);
        }

        // Creates a new instance.
        // @param label : the label before the target instruction.
        constexpr Jump(const Label label)
            : m_label(label)
        {
        }

        // Gets the label before the target instruction.
        constexpr Label Label() const
        {
            return m_label;
        }
    };

    // Compares the given jumps by their targets.
    // Returns true if the jumps have the same targets.
    // @param left : the first jump to compare.
    // @param right : the second jump to compare.
    template <typename TOffset>
    constexpr bool operator==(const Jump<TOffset> left, const Jump<TOffset> right)
    {
        return left.Label() == right.Label();
    }

    // Compares the given jumps by their targets.
    // Returns true if the jumps have different targets.
    // @param left : the first jump to compare.
    // @param right : the second jump to compare.
    template <typename TOffset>
    constexpr bool operator!=(const Jump<TOffset> left, const Jump<TOffset> right)
    {
        return !(left == right);
    }

    // Support for writing jump targets to standard output streams.
    // @param target : the stream to output data to.
    // @param value : the jump to output.
    template <typename TOffset, typename TChar>
    std::basic_ostream<TChar>& operator <<(
        std::basic_ostream<TChar>& target,
        Jump<TOffset> value)
    {
        target << value.Label();
        return target;
    }

    // Type of jumps which is used by the short form of branching instructions.
    using ShortJump = Jump<int8_t>;

    // Type of jumps which is used by the long form of branching instructions.
    using LongJump = Jump<int32_t>;

    // Unit to measure sizes of instructions,
    // sizes of instructions streams, and
    // distances from instructions stream start to
    // a specific instruction.
    using AbsoluteOffset = uint32_t;

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
        using InlineBrTarget = LongJump;
        using InlineI8 = int64_t;

        class InlineMethod
        {
        public:
            using TokenType = uint32_t;

            TokenType MetadataToken;
            uint32_t ParametersCount;
            bool HasReturnValue;
        };

        using InlineField = uint32_t;
        using InlineType = uint32_t;
        using InlineString = uint32_t;
        using InlineSig = uint32_t;
        using InlineRVA = uint32_t;
        using InlineTok = uint32_t;
        using InlineSwitch = std::vector<LongJump>;
        using InlinePhi = std::vector<uint16_t>;
        using ShortInlineVar = uint8_t;
        using ShortInlineI = int8_t;
        using ShortInlineR = float;
        using ShortInlineBrTarget = ShortJump;
    };

    constexpr bool operator==(
        const OpCodeArgumentType::InlineMethod left,
        const OpCodeArgumentType::InlineMethod right)
    {
        return left.MetadataToken == right.MetadataToken
            && left.ParametersCount == right.ParametersCount
            && left.HasReturnValue == right.HasReturnValue;
    }

    constexpr bool operator!=(
        const OpCodeArgumentType::InlineMethod left,
        const OpCodeArgumentType::InlineMethod right)
    {
        return !(left == right);
    }

    // How the instruction affect flow control.
    enum class OpCodeFlowBehavior
    {
        // Usual instruction. After the instruction is executed,
        // transfers the control to the next instruction.
        Next,

        // Debugger instruction. After the instruction is executed,
        // and the debugger allowed to continue,
        // transfers the control to the next instruction.
        Break,

        // Method or constructor call. Transfers the control
        // to the target method or constructor, and continues
        // to the next instruction after the called method or
        // constructor is exited.
        Call,

        // Transfers the control to the method calling the
        // current method.
        Return,

        // Unconditionally transfers the control to the
        // given target instruction.
        Branch,

        // If the condition is met, transfers to control to
        // the given target instruction. Otherwise, transfers
        // the control to the next instruction.
        ConditionalBranch,

        // Transfes to control to the nearest catch or finally.
        Throw,

        // Value used for prefixes which does not have any
        // control flow effect.
        Meta
    };

    // Common base class for all opcodes.
    // Intentionally left empty.
    // Should only be used for type compatibility checking.
    class OpCodeTag
    {
    };

    // Determines whether the given class represents an opcode.
    template <typename T>
    concept IsOpCode = std::derived_from<T, OpCodeTag>;

    // Contains a set of CEE_* classes, each representing a
    // specific Intermediate Language instruction.
    class OpCode
    {
    private:

        // Base class with interface methods for all opcodes.
        // Uses CRTP ( https://www.fluentcpp.com/2017/05/12/curiously-recurring-template-pattern/ )
        // to provide methods and values specific to the opcode.
        // Parameters:
        // TOpCode - the opcode to produce values for.
        // TArgument - the type of the OpCode inline argument.
        // flowBehavior - the value describing control flow
        //     behavior of the instruction.
        template <
            typename TOpCode,
            typename TArgument,
            OpCodeFlowBehavior flowBehavior>
        class OpCodeBase : public OpCodeTag
        {
        public:
            OpCodeBase() noexcept
            {
            }

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

            // The value describing control flow
            // behavior of the instruction.
            constexpr inline static OpCodeFlowBehavior FlowBehavior { flowBehavior };
        };

        // Provides methods for inline argument handling.
        // Parameters:
        // TArgument : the type of the inline argument.
        template <typename TArgument>
        class OpCodeArgument
        {
        private:
            TArgument m_argument;

        public:
            // Creates a new opcode value with the
            // given argument.
            // @param argument : the value to use.
            OpCodeArgument(const TArgument argument) noexcept
                : m_argument{ argument }
            {
            }

            // Gets the inline argument.
            TArgument Argument() const noexcept
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
        template <>
        class OpCodeArgument<OpCodeArgumentType::InlineNone>
        {
        public:
            OpCodeArgument() noexcept
            {
            }
        };

        // opcode.def will use these variables to define
        // push stack behavior.
        // 0 will mean nothing pushed to the method stack.
        // 1 will mean 1 item is pushed to the method stack.
        // Values can be combined, for example PushRef+PushI4
        // will give 2, which means 2 items are pushed.
        // nullptr will mean that the instruction pushes
        // varying amount of items, determined by the usage context,
        // for example the signature of the function being called.
        // nullptr cannot be combined with other values.
        // The value from opcode.def is then used with
        // the StackPush base class. Different type for VarPush
        // allows specialization for the VarPush case.
        inline static constexpr int Push0{ 0 };
        inline static constexpr int Push1{ 1 };
        inline static constexpr int PushI{ 1 };
        inline static constexpr int PushI4{ 1 };
        inline static constexpr int PushR4{ 1 };
        inline static constexpr int PushI8{ 1 };
        inline static constexpr int PushR8{ 1 };
        inline static constexpr int PushRef{ 1 };
        inline static constexpr nullptr_t VarPush{ nullptr };

        // As a base class, provides static values describing push
        // stack behavior to the CEE_* classes.
        // If the instruction always pushes the same amount of items,
        // it will have static member constants IsStackPushBehaviorKnown,
        // equal to true, and ItemsPushedToStack, equal to the amount
        // of items. If the amount of items the instruction pushes
        // is different for each usage, only static member constant
        // IsStackPushBehaviorKnown, equal to false, is provided.
        // stackPush : Combination of Push0, Push1, etc variables 
        //     from opcode.def, or VarPush.
        // T : type of stackPush.
        template <typename T, T stackPush>
        class StackPush;

        // Specialization of StackPush for the VarPush case.
        // In this case, T = nullptr_t, stackPush = VarPush.
        // Only IsStackPushBehaviorKnown, equal to false, is provided.
        template <>
        class StackPush <nullptr_t, VarPush>
        {
        public:
            // Value indicating the amount of items the instruction
            // pushes onto the stack is different in each usage.
            // The amount is then determined by the usage context,
            // for example the signature of the function being called.
            inline static constexpr bool IsStackPushBehaviorKnown { false };
        };

        // Specialization of StackPush for case when
        // the count of items for specific instruction
        // is always the same and known.
        // In this case, T = int, and stackPush is a combination of
        // Push0, Push1, etc variables from opcode.def.
        // In this case, provides static member constants
        // IsStackPushBehaviorKnown, equal to true, and
        // ItemsPushedToStack, equal to the amount of items.
        template <int stackPush>
        class StackPush<int, stackPush>
        {
        public:
            // The value indicating that the instruction
            // always pushes the same amount of items onto the stack.
            inline static constexpr bool IsStackPushBehaviorKnown { true };

            // The amount of parameters the instruction
            // pushes onto the stack.
            inline static constexpr int ItemsPushedToStack { stackPush };
        };

        // opcode.def will use these variables to define
        // pop stack behavior.
        // 0 will mean nothing popped from the method stack.
        // 1 will mean 1 item is popped from the method stack.
        // Values can be combined, for example PopRef+PopI4
        // will give 2, which means 2 items are popped.
        // nullptr will mean that the instruction pops
        // varying amount of items, determined by the usage context,
        // for example the signature of the function being called.
        // nullptr cannot be combined with other values.
        // The value from opcode.def is then used with
        // the StackPop base class. Different type for VarPop
        // allows specialization for the VarPop case.
        inline static constexpr int Pop0{ 0 };
        inline static constexpr int Pop1{ 1 };
        inline static constexpr int PopI{ 1 };
        inline static constexpr int PopI4{ 1 };
        inline static constexpr int PopR4{ 1 };
        inline static constexpr int PopI8{ 1 };
        inline static constexpr int PopR8{ 1 };
        inline static constexpr int PopRef{ 1 };
        inline static constexpr nullptr_t VarPop{ nullptr };

        // As a base class, provides static values describing pop
        // stack behavior to the CEE_* classes.
        // If the instruction always pops the same amount of items,
        // it will have static member constants IsStackPopBehaviorKnown,
        // equal to true, and ItemsPoppedFromStack, equal to the amount
        // of items. If the amount of items the instruction pops is
        // different for each usage, only static member constant
        // IsStackPopBehaviorKnown, equal to false, is provided.
        // stackPop : Combination of Pop0, Pop1, etc variables 
        //     from opcode.def, or VarPop.
        // T : type of stackPop.
        template <typename T, T stackPop>
        class StackPop;

        // Specialization of StackPop for the VarPop case.
        // In this case, T = nullptr_t, stackPop = VarPop.
        // Only IsStackPopBehaviorKnown, equal to false, is provided.
        template <>
        class StackPop <nullptr_t, VarPop>
        {
        public:
            // Value indicating the amount of items the instruction
            // pops from the stack is different in each usage.
            // The amount is then determined by the usage context,
            // for example the signature of the function being called.
            inline static constexpr bool IsStackPopBehaviorKnown { false };
        };

        // Specialization of StackPop for case when
        // the count of items for specific instruction
        // is always the same and known.
        // In this case, T = int, and stackPop is a combination of
        // Pop0, Pop1, etc variables from opcode.def.
        // In this case, provides static member constants
        // IsStackPopBehaviorKnown, equal to true, and
        // ItemsPoppedFromStack, equal to the amount of items.
        template <int stackPop>
        class StackPop<int, stackPop>
        {
        public:
            // The value indicating that the instruction
            // always pops the same amount of items from the stack.
            inline static constexpr bool IsStackPopBehaviorKnown { true };

            // The amount of parameters the instruction
            // pops from the stack.
            inline static constexpr int ItemsPoppedFromStack { stackPop };
        };

        // Define values with exactly the same names as
        // opcode.def uses to describe control flow behavior.
        inline static constexpr OpCodeFlowBehavior NEXT { OpCodeFlowBehavior::Next };
        inline static constexpr OpCodeFlowBehavior BREAK { OpCodeFlowBehavior::Break };
        inline static constexpr OpCodeFlowBehavior CALL { OpCodeFlowBehavior::Call };
        inline static constexpr OpCodeFlowBehavior RETURN { OpCodeFlowBehavior::Return };
        inline static constexpr OpCodeFlowBehavior BRANCH { OpCodeFlowBehavior::Branch };
        inline static constexpr OpCodeFlowBehavior COND_BRANCH { OpCodeFlowBehavior::ConditionalBranch };
        inline static constexpr OpCodeFlowBehavior THROW { OpCodeFlowBehavior::Throw };
        inline static constexpr OpCodeFlowBehavior META { OpCodeFlowBehavior::Meta };

    public:

        // Using the OPDEF_REAL_INSTRUCTION macro, the next invocation
        // of <opcode.def> will generate a set of OpCode::CEE_* classes, each
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
    class canonicalName : \
        public OpCodeBase<\
            canonicalName , \
            OpCodeArgumentType:: ## inlineArgumentType , \
            controlBehavior >, \
        public OpCodeArgument<OpCodeArgumentType:: ## inlineArgumentType >, \
        public StackPush<std::decay_t<decltype(stackPush)>, stackPush>, \
        public StackPop<std::decay_t<decltype(stackPop)>, stackPop> \
    { \
    public: \
        inline static constexpr std::wstring_view CanonicalName { L ## stringName }; \
        using OpCodeArgument::OpCodeArgument; \
    };
#include "DefineOpCodesGeneratorSpecializations.h"
#include <opcode.def>
#include "UnDefineOpCodesGeneratorSpecializations.h"
#undef OPDEF_REAL_INSTRUCTION

    }; // end of OpCode class

    // Provides compile-time operations on a set of types.
    template <typename ...>
    class ParameterPack;

    // Allows to merge two sets of types.
    template <typename TPack1, typename TPack2>
    class Tail;

    // Merging two sets of types: primary specialization.
    template <typename ... TFirst, typename ... TSecond>
    class Tail<ParameterPack<TFirst ...>, ParameterPack<TSecond ...>>
    {
    public:
        using Type = typename ParameterPack<TFirst ..., TSecond ...>;
    };

    // Compile-time operations for 1 type.
    template <typename T>
    class ParameterPack<T>
    {
    public:
        // Applies the type to the given template.
        template <template <typename T> class TTemplate>
        using Apply = TTemplate<T>;

        // Provides a set of types with removed duplicates.
        using RemoveDuplicates = typename ParameterPack<T>;
    };

    // Compile-time operations for 2 or more types.
    template <typename T, typename T2, typename ... Ts>
    class ParameterPack<T, T2, Ts...>
    {
    public:
        // Applies the given types to given template.
        template <template <typename T, typename T2, typename ... Ts> class TTemplate>
        using Apply = TTemplate<T, T2, Ts ...>;

        // Provides a set of types with duplicates removed.
        using RemoveDuplicates = typename std::conditional_t <
            std::is_same_v<T, T2> || ( std::is_same_v<T, Ts> || ... ),
            typename ParameterPack<T2, Ts ...>::RemoveDuplicates,
            typename Tail<ParameterPack<T>, typename ParameterPack<T2, Ts ...>::RemoveDuplicates>::Type>;
    };

    // Stores one of possible Intermediate Language instructions.
    class OpCodeVariant
    {
    private:
        friend class MethodBody;

        // Variant which can store any possible inline argument type,
        // std::monostate is for no inline argument case.
        using VariantType = ParameterPack<
            std::monostate,
            OpCodeArgumentType::InlineBrTarget,
            OpCodeArgumentType::InlineField,
            OpCodeArgumentType::InlineI,
            OpCodeArgumentType::InlineI8,
            OpCodeArgumentType::InlineMethod,
            OpCodeArgumentType::InlinePhi,
            OpCodeArgumentType::InlineR,
            OpCodeArgumentType::InlineRVA,
            OpCodeArgumentType::InlineSig,
            OpCodeArgumentType::InlineString,
            OpCodeArgumentType::InlineSwitch,
            OpCodeArgumentType::InlineTok,
            OpCodeArgumentType::InlineType,
            OpCodeArgumentType::InlineVar,
            OpCodeArgumentType::ShortInlineBrTarget,
            OpCodeArgumentType::ShortInlineI,
            OpCodeArgumentType::ShortInlineR,
            OpCodeArgumentType::ShortInlineVar
        >::RemoveDuplicates::Apply<std::variant>;

        // Represents instruction code. Has maximum 2 bytes,
        // but some codes can be in 1-byte form.
        class InstructionCode
        {
        private:
            inline static const std::byte s_OneByteMarker = std::byte(0xFF);

        public:
            // The first byte. Can be a one-byte form marker.
            std::byte FirstByte;

            // The second byte.
            std::byte SecondByte;

            // Gets the value indicating whether this code
            // can be in 1-byte form.
            constexpr bool IsOneByte() const
            {
                return FirstByte == s_OneByteMarker;
            }

            // Gets the size of binary representation, in bytes.
            constexpr AbsoluteOffset Size() const
            {
                return IsOneByte() ? 1 : 2;
            }

            // Gets the combined uint16_t value to use in switch-case statements.
            constexpr uint16_t AsKey() const
            {
                return static_cast<uint16_t>(FirstByte) << 8 | static_cast<uint16_t>(SecondByte);
            }

            // Append the binary representation to the given vector.
            // @param target : the vector to append to.
            void AppendToVector(std::vector<std::byte>& target) const;

            // Compares two codes for equality.
            constexpr bool operator==(const InstructionCode other) const
            {
                return FirstByte == other.FirstByte && SecondByte == other.SecondByte;
            }

            // Compares two codes for inequality.
            constexpr bool operator!=(const InstructionCode other) const
            {
                return !(*this == other);
            }
        };

        // Allows to get an instruction code for a
        // specific OpCode type.
        // The next call to <opcode.def> will specialize
        // for each possible instruction.
        template <typename TOpCode>
        struct OpCodeInstruction;

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
template <> \
class OpCodeInstruction<OpCode :: ## canonicalName> \
{ \
public: \
    static constexpr InstructionCode Code { std::byte { byte1 }, std::byte { byte2 } }; \
};

#include "DefineOpCodesGeneratorSpecializations.h"
#include <opcode.def>
#include "UnDefineOpCodesGeneratorSpecializations.h"
#undef OPDEF_REAL_INSTRUCTION

        // Stores the instruction code.
        InstructionCode m_code;

        // Stores the inline argument.
        VariantType m_argument;

        // Intended to use from MethodBody.
        OpCodeVariant(
            InstructionCode code,
            VariantType argument);

        // Gets the opcode of the given type,
        // with the argument stored in this variant.
        // Does not do any checks, so made private.
        template <typename TOpCode>
        TOpCode Get() const
        {
            if constexpr (TOpCode::HasArgument())
            {
                return TOpCode(std::get<TOpCode::ArgumentType>(m_argument));
            }
            else
            {
                return TOpCode{};
            }
        }

    public:
        // Constructs a variant holding OpCode::CEE_NOP instruction,
        // which means no operation.
        OpCodeVariant();

        // Constructs the variant holding the given opcode.
        // @param opCode : instruction to store.
        template <IsOpCode TOpCode>
        OpCodeVariant(const TOpCode opCode)
            : m_code (OpCodeInstruction<TOpCode>::Code),
            m_argument{}
        {
            if constexpr (TOpCode::HasArgument())
            {
                m_argument = opCode.Argument();
            }
        }

        // Puts the given opcode into this variant.
        // @param opCode : the instruction to store.
        template <IsOpCode TOpCode>
        OpCodeVariant& operator=(const TOpCode opCode)
        {
            m_code = OpCodeInstruction<TOpCode>::Code;
            if constexpr (TOpCode::HasArgument())
            {
                m_argument = opCode.Argument();
            }
            else
            {
                m_argument = std::monostate{};
            }

            return *this;
        }

        // Gets the value indicating whether the
        // variant contains an opcode of the given type.
        // TOpCode : the opcode type to check for.
        template <IsOpCode TOpCode>
        bool HoldsAlternative() const
        {
            return m_code == OpCodeInstruction<TOpCode>::Code;
        }

        // If the variant contains an opcode of the given type,
        // gets it. std::nullopt otherwise.
        // TOpCode : the opcode type to check for.
        template <IsOpCode TOpCode>
        std::optional<TOpCode> GetIf() const
        {
            if (HoldsAlternative<TOpCode>())
            {
                return Get<TOpCode>();
            }
            else
            {
                return std::nullopt;
            }
        }

        // Gets the value indicating whether the opcode
        // this variant contains has an inline argument.
        bool HasArgument() const;

        // Gets the size of binary representation of the
        // opcode this variant contains, including the instruction
        // code and the inline argument (if any).
        AbsoluteOffset SizeWithArgument() const;

        // Calls the visitor with the instruction this variant contains.
        // TVisitor must provide overload for each possible OpCode_* class.
        // This is usually done with a template lambda.
        // @param : must have method accepting the OpCode_* instance.
        template <typename TVisitor>
        void Visit(TVisitor&& visitor) const
        {
            switch (m_code.AsKey())
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
            case InstructionCode { std::byte { byte1 }, std::byte { byte2 } }.AsKey() : \
            { \
                visitor(Get< OpCode :: ## canonicalName >()); \
            } \
\
            break;

#include "DefineOpCodesGeneratorSpecializations.h"
#include <opcode.def>
#include "UnDefineOpCodesGeneratorSpecializations.h"
#undef OPDEF_REAL_INSTRUCTION
            }
        }

        template <typename TChar>
        friend std::basic_ostream<TChar>& operator <<(
            std::basic_ostream<TChar>& target,
            const OpCodeVariant& variant)
        {
            variant.Visit([&target](const auto &opcode)
            {
                using T = std::decay_t<decltype(opcode)>;
                target << T::Name();

                if constexpr (T::HasArgument())
                {
                    target << L" ";
                    if constexpr (std::is_same_v<T::ArgumentType, OpCodeArgumentType::InlineSwitch>
                        || std::is_same_v<T::ArgumentType, OpCodeArgumentType::InlinePhi>)
                    {
                        const auto& argument = opcode.Argument();
                        target << InSquareBrackets(argument.size()) << InCurlyBrackets(Delimitered(argument, L", "));
                    }
                    else if constexpr (std::is_same_v<T::ArgumentType, OpCodeArgumentType::InlineMethod>)
                    {
                        target << opcode.Argument().MetadataToken;
                    }
                    else
                    {
                        target << opcode.Argument();
                    }
                }

                target << L";";
            });

            return target;
        }

        // Compares with another OpCodeVariant.
        // Returns true if the other OpCodeVariant stores the
        // same instruction with the same inline arguments.
        // @param other : the OpCodeVariant to compare with.
        bool operator==(const OpCodeVariant& other) const noexcept
        {
            return m_code == other.m_code && m_argument == other.m_argument;
        }

        // Compares with another OpCodeVariant.
        // Returns true if the other OpCodeVariant stores the
        // a different instruction, or the same instruction with
        // different inline arguments.
        // @param other : the OpCodeVariant to compare with.
        bool operator!=(const OpCodeVariant& other) const noexcept
        {
            return !(*this == other);
        }
    };
}

