#include "pch.h"
#include "OpCodes.h"

namespace Drill4dotNet
{
    OpCodeVariant::OpCodeVariant(
        InstructionCode code,
        VariantType argument)
        : m_code(code),
        m_argument { argument }
    {
    }

    OpCodeVariant::OpCodeVariant()
        : m_code(OpCodeInstruction<OpCode::CEE_NOP>::Code),
        m_argument {}
    {
    }

    bool OpCodeVariant::HasArgument() const
    {
        return !std::holds_alternative<std::monostate>(m_argument);
    }

    AbsoluteOffset OpCodeVariant::SizeWithArgument() const
    {
        AbsoluteOffset instructionSize { m_code.Size() };
        return instructionSize + std::visit([](const auto& argument)
            {
                using T = std::decay_t<decltype(argument)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    return static_cast<AbsoluteOffset>(0);
                }
                else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlinePhi>)
                {
                    return static_cast<AbsoluteOffset>(argument.size() * sizeof(uint16_t) + sizeof(uint8_t));
                }
                else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineSwitch>)
                {
                    return static_cast<AbsoluteOffset>(argument.size() * sizeof(LongJump::Offset) + sizeof(uint32_t));
                }
                else if constexpr (std::is_same_v<T, OpCodeArgumentType::ShortInlineBrTarget>
                    || std::is_same_v<T, OpCodeArgumentType::InlineBrTarget>)
                {
                    return static_cast<AbsoluteOffset>(sizeof(T::Offset));
                }
                else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineMethod>)
                {
                    return static_cast<AbsoluteOffset>(sizeof(OpCodeArgumentType::InlineMethod::TokenType));
                }
                else
                {
                    return static_cast<AbsoluteOffset>(sizeof(argument));
                }
            },
            m_argument);
    }

    void OpCodeVariant::InstructionCode::AppendToVector(std::vector<std::byte>& target) const
    {
        if (!IsOneByte())
        {
            target.push_back(FirstByte);
        }

        target.push_back(SecondByte);
    }


}

