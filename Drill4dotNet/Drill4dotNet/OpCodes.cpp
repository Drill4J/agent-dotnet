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
        : m_code(OpCodeInstruction<OpCode_CEE_NOP>::Code),
        m_argument {}
    {
    }

    bool OpCodeVariant::HasArgument() const
    {
        return !std::holds_alternative<std::monostate>(m_argument);
    }

    uint32_t OpCodeVariant::SizeWithArgument() const
    {
        uint32_t instructionSize { m_code.Size() };
        return instructionSize + std::visit([](const auto& argument)
            {
                using T = std::decay_t<decltype(argument)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    return static_cast<uint32_t>(0);
                }
                else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlinePhi>)
                {
                    return static_cast<uint32_t>(argument.size() * sizeof(uint16_t) + sizeof(uint8_t));
                }
                else if constexpr (std::is_same_v<T, OpCodeArgumentType::InlineSwitch>)
                {
                    return static_cast<uint32_t>(argument.size() * sizeof(int32_t) + sizeof(int32_t));
                }
                else
                {
                    return static_cast<uint32_t>(sizeof(argument));
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

