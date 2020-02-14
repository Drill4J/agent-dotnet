#pragma once

#include "OpCodes.h"
#include "InstructionStream.h"
#include "MethodHeader.h"

namespace Drill4dotNet
{
    // Object representation of method body bytes.
    // Allows converting back to bytes representation.
    // Reference: ECMA-335, Common Language Infrastructure,
    // part II.25.4 Common Intermediate Language physical layout
    // https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf
    class MethodBody
    {
    private:
        // The header storing information about other method structures.
        MethodHeader m_header;

        // The parsed instructions.
        std::vector<OpCodeVariant> m_instructions;

        // Parses a byte representation of instructions into a vector.
        static std::vector<OpCodeVariant> Decompile(
            const std::vector<std::byte>& bodyBytes,
            uint8_t headerSize,
            const AbsoluteOffset codeSize);


        // Specialized for .net's OpArgsVal to allow
        // getting instruction arguments from it.
        template <typename TOpArgsVal>
        class ArgumentConverter;
    public:
        // Creates the object representation of the method body.
        // @param bodyBytes : the bytes of method body.
        explicit MethodBody(const std::vector<std::byte>& bodyBytes);

        // Makes a binary representation of the method body.
        std::vector<std::byte> Compile() const;

        // Inserts the given instruction into the instructions list.
        // @param position : the point at which to insert,
        //    must be in range [begin(), end()]
        // @param opcode : the instruction to insert.
        void Insert(
            const std::vector<OpCodeVariant>::const_iterator position,
            const OpCodeVariant opcode);

        // Gets the beginning of the instructions list.
        std::vector<OpCodeVariant>::const_iterator begin() const noexcept
        {
            return m_instructions.cbegin();
        }

        // Gets the ending of the instructions list.
        std::vector<OpCodeVariant>::const_iterator end() const noexcept
        {
            return m_instructions.cend();
        }

        // Allows printing the instructions to standard streams.
        template <typename TChar>
        friend std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, const MethodBody& data)
        {
            for (const auto& x : data.m_instructions)
            {
                x.Visit([&target](const auto opcode)
                {
                    using T = std::remove_cv_t<decltype(opcode)>;
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
                        else
                        {
                            target << opcode.Argument();
                        }
                    }

                    target << L";" << std::endl;
                });
            }

            return target;
        }
    };
}

