#include "pch.h"
#include "MethodBody.h"

#include <array>

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
    template <typename TArgument>
    auto ConvertArgument(const OpArgsVal argument)
    {
        if constexpr (std::is_same_v<TArgument, OpCodeArgumentType::InlineNone>)
        {
            return std::monostate{};
        }
        else if constexpr (
            std::is_same_v<TArgument, OpCodeArgumentType::ShortInlineR>
            || std::is_same_v<TArgument, OpCodeArgumentType::InlineR>)
        {
            return static_cast<TArgument>(argument.r);
        }
        else if constexpr (
            std::is_same_v<TArgument, OpCodeArgumentType::InlineI8>)
        {
            return static_cast<TArgument>(argument.i8);
        }
        else if constexpr (
            std::is_same_v<TArgument, OpCodeArgumentType::InlineSwitch>)
        {
            auto begin = (int32_t*)(argument.switch_.targets);
            auto end = begin + argument.switch_.count;
            return std::vector<int32_t>(begin, end);
        }
        else if constexpr (
            std::is_same_v<TArgument, OpCodeArgumentType::InlinePhi>)
        {
            auto begin = (uint16_t*)(argument.phi.vars);
            auto end = begin + argument.phi.count;
            return std::vector<uint16_t>(begin, end);
        }
        else
        {
            return static_cast<TArgument>(argument.i);
        }
    }

    MethodBody::MethodBody(const std::vector<std::byte>& bodyBytes)
    {
        if ((bodyBytes[0] & s_TinyHeaderFlagsMask) == s_TinyHeaderFlag)
        { // tiny header
            m_flags = static_cast<uint16_t>(bodyBytes[0] & s_TinyHeaderFlagsMask);
            m_headerSize = sizeof(std::byte);
            m_maxStack = std::nullopt;
            m_codeSize = static_cast<uint32_t>(bodyBytes[0] & s_TinyHeaderSizeMask) >> 2;
            m_localVariables = std::nullopt;
        }
        else
        { // fat header
            const IMAGE_COR_ILMETHOD_FAT& fatHeader
                = *reinterpret_cast<const IMAGE_COR_ILMETHOD_FAT*>(bodyBytes.data());
            m_flags = fatHeader.Flags;
            m_headerSize = static_cast<uint8_t>(sizeof(uint32_t) * fatHeader.Size);
            m_maxStack = static_cast<uint16_t>(fatHeader.MaxStack);
            m_codeSize = fatHeader.CodeSize;
            m_localVariables = fatHeader.LocalVarSigTok == 0
                ? std::optional<uint32_t>(std::nullopt) : fatHeader.LocalVarSigTok;
            m_fatHeaderRemainder.assign(
                bodyBytes.cbegin() + sizeof(fatHeader),
                bodyBytes.cbegin() + m_headerSize);
        }

        m_instructions = Decompile(
            bodyBytes,
            m_headerSize,
            m_codeSize);
    }

    std::vector<OpCodeVariant> MethodBody::Decompile(
        const std::vector<std::byte>& bodyBytes,
        uint8_t headerSize,
        const uint32_t codeSize)
    {
        // Uses
        // const BYTE* OpInfo::fetch(const BYTE * instrPtr, OpArgsVal * args)
        OpInfo parser{};
        std::vector<OpCodeVariant> result{};
        const auto begin = reinterpret_cast<const BYTE*>(bodyBytes.data() + headerSize);
        const auto end = begin + codeSize;
        auto currentInstruction{ begin };
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
                result.push_back( OpCodeVariant ( \
                    OpCodeVariant::InstructionCode { std::byte { byte1 }, std::byte {byte2} }, \
                    OpCodeVariant::VariantType(ConvertArgument<OpCodeArgumentType :: ## inlineArgumentType >(inlineArguments)))); \
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

    template <typename T>
    void AppendAsBytes(std::vector<std::byte>& target, const T& value)
    {
        for (const auto b : (std::array<std::byte, sizeof(T)>&)value)
        {
            target.push_back(b);
        }
    }

    std::vector<std::byte> MethodBody::Compile() const
    {
        std::vector<std::byte> result{};
        result.reserve(m_instructions.size() + m_headerSize); // each instruction takes at least 1 byte, so all this storage will be used

        if (m_headerSize == sizeof(std::byte)) // Dangerous, need to recalculate header type
        { // tiny header
            result.push_back(std::byte{ m_flags | m_codeSize << 2 }); // Dangerous, need to recalculate m_codeSize
        }
        else
        { // fat header
            IMAGE_COR_ILMETHOD_FAT header;
            header.Flags = m_flags;
            header.Size = m_headerSize / sizeof(uint32_t); // Dangerous, assert m_headerSize / sizeof(int32_t) == 0
            header.MaxStack = m_maxStack.value(); // Dangerous, may be std::nullopt
            header.LocalVarSigTok = m_localVariables.has_value() ? *m_localVariables : 0;
            header.CodeSize = m_codeSize; // Dangerous, need to recalculate m_codeSize

            AppendAsBytes(result, header);
            std::copy(
                m_fatHeaderRemainder.cbegin(),
                m_fatHeaderRemainder.cend(),
                std::back_inserter(result));
        }

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
        m_codeSize += opcode.SizeWithArgument();
        m_instructions.insert(position, opcode);
    }
}

