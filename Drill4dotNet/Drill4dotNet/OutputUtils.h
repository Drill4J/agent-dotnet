#pragma once

#include <ostream>
#include <ios>
#include <iomanip>
#include <optional>

namespace Drill4dotNet
{
    // RAII wrapper to save and restore settings
    // of a stream.
    // Usage:
    // void f()
    // {
    //     StreamFlagsSaveRestore storedFlags { std::cout }; // settings stored
    //     std::cout << std::setw(8) << std::setfill('0') << 42; // stream state changed
    //     // here, storedFlags will restore the settings
    // }
    // 
    // // Will type 00000042 42
    // void main()
    // {
    //     f();
    //     std::cout << " " << 42;
    // }
    template <typename T>
    class StreamFlagsSaveRestore
    {
    private:
        T& m_stream;
        std::ios_base::fmtflags m_flags;

    public:
        // Stores the format setting of the given stream
        // into the instance being created.
        // stream : the underlying object should be live
        //     all the time the new instance will live.
        StreamFlagsSaveRestore(T& stream)
            : m_stream(stream), m_flags(stream.flags())
        {
        }

        // Restores the target stream status
        ~StreamFlagsSaveRestore()
        {
            m_stream.flags(m_flags);
        }

        // Copy operations are forbidden because they do
        // not make any sense.
        StreamFlagsSaveRestore(const StreamFlagsSaveRestore&) = delete;
        StreamFlagsSaveRestore& operator=(const StreamFlagsSaveRestore&) = delete;
    };

    // Tool to put a number in the hex form to a stream.
    // Pads the value with zeros on the left.
    // Does not affect the output settings of other values.
    // Parameters:
    // width : Number of characters to output, optional
    // Usage:
    // void main()
    // {
    //     // Types: 0x10 == 16
    //     std::cout << "0x" << HexOutput(16) << " == " << 16;
    //
    //     // Types: 0x0010
    //     std::cout << "0x" << HexOutput<int, 4>(16);
    // }
    template <typename T, size_t width = 2 * sizeof(T)>
    class HexOutput
    {
    private:
        const T& m_value;
    public:
        // Puts a number in the fixed-width hex form to a stream.
        // Pads the value with zeros on the left, Does not affect
        // the output settings of other values. Usage:
        // void main()
        // {
        //     // Types: 0x10 == 16
        //     std::cout << "0x" << HexOutput(16) << " == " << 16;
        //
        //     // Types: 0x0010
        //     std::cout << "0x" << HexOutput<int, 4>(16);
        // }
        explicit HexOutput(const T& value) noexcept
            : m_value(value)
        {
        }

        // Temporarily changes the format settings of the target stream,
        // to output the value in the desired format.
        template <typename TChar>
        friend std::basic_ostream<TChar>& operator<<(std::basic_ostream<TChar>& target, const HexOutput& value)
        {
            StreamFlagsSaveRestore restoreStreamOnExit { target };

            target
                << std::hex
                << std::uppercase
                << std::setw(width)
                << std::setfill(L'0')
                << value.m_value;

            return target;
        }
    };

    // Provides support for putting an empty optional<T>
    // to an output stream.
    // Writes "std::nullopt".
    template <typename TChar>
    std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, std::nullopt_t)
    {
        // DO NOT "optimize" by removing this function and putting the string to the next operator.
        // This will cause inconsistency between different usage scenarios.

        target << L"std::nullopt";
        return target;
    }

    // Provides support for putting an optional<T>
    // to an output stream.
    // Writes the contents of the givan optional.
    // If the given optional is empty, types a placeholder.
    template <typename TChar, typename T>
    std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, const std::optional<T>& value)
    {
        if (value.has_value())
        {
            target << *value;
        }
        else
        {
            target << std::nullopt;
        }

        return target;
    }
}
