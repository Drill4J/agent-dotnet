#pragma once

#include <ostream>
#include <ios>
#include <iomanip>
#include <optional>
#include <cstddef>
#include <type_traits>
#include <iterator>
#include <utility>
#include <string>
#include <concepts>

namespace Drill4dotNet
{
    // Determines whether values of the givan type
    // can be sent to standard streams via << operator.
    template <typename T>
    concept HasOutputOperator = requires(
        const T& value,
        std::basic_ostream<char>& target)
    {
        { target << value } -> std::same_as<decltype(target)>;
    } || requires(
        const T& value,
        std::basic_ostream<wchar_t>& target)
    {
        { target << value } -> std::same_as<decltype(target)>;
    };

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
    template <HasOutputOperator T, size_t width = 2 * sizeof(T)>
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
                << std::setfill(TChar { '0' })
                << value.m_value;

            return target;
        }
    };

    // Writes the given object to a stream,
    // surrounding it with brackets.
    template <
        HasOutputOperator T,
        auto OpenBracket,
        decltype(OpenBracket) CloseBracket>
    class InBrackets
    {
    private:
        const T& m_value;
    public:
        // Creates a object which will output the
        // given value, surrounding it with brackets
        explicit InBrackets(const T& value)
            : m_value { value }
        {
        }

        // Outputs brackets and the value to the given stream.
        template <typename TChar>
        friend std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, const InBrackets& data)
        {
            target << OpenBracket << data.m_value << CloseBracket;
            return target;
        }
    };

    // Creates an object, which will output the given value,
    // surrounding it with round brackets.
    template <HasOutputOperator T>
    InBrackets<T, '(', ')'> InRoundBrackets(const T& value)
    {
        return InBrackets<T, '(', ')'>(value);
    }

    // Creates an object, which will output the given value,
    // surrounding it with square brackets.
    template <HasOutputOperator T>
    InBrackets<T, '[', ']'> InSquareBrackets(const T& value)
    {
        return InBrackets<T, '[', ']'>(value);
    }

    // Creates an object, which will output the given value,
    // surrounding it with curly brackets.
    template <HasOutputOperator T>
    InBrackets<T, '{', '}'> InCurlyBrackets(const T& value)
    {
        return InBrackets<T, '{', '}'>(value);
    }

    // Creates an object, which will output the given value,
    // surrounding it with spaces.
    template <HasOutputOperator T>
    InBrackets<T, ' ', ' '> InSpaces(const T& value)
    {
        return InBrackets<T, ' ', ' '>(value);
    }

    // Creates an object, which will output the given value,
    // surrounding it with apostrophes (aka single quotes).
    template <HasOutputOperator T>
    InBrackets<T, '\'', '\''> InApostrophes(const T& value)
    {
        return InBrackets<T, '\'', '\''>(value);
    }
    // Allows to put content of containers to streams,
    // adding a given delmiter between items.
    template <typename TContainer, HasOutputOperator TDelimiter>
    class Delimitered
    {
    private:
        const TContainer& m_container;
        const TDelimiter& m_delimiter;
    public:
        // Creates an object which will output
        // each item of the given container, and
        // the specified delimiter between items.
        Delimitered(const TContainer& container, const TDelimiter& delimiter)
            : m_container(container),
            m_delimiter(delimiter)
        {
            static_assert(!std::is_void_v<decltype(std::cbegin(container))>);
            static_assert(!std::is_void_v<decltype(std::cend(container))>);
        }

        // Outputs items and delimiters between them.
        template <typename TChar>
        friend std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, const Delimitered& data)
        {
            bool first { true };
            for (const auto& x : data.m_container)
            {
                if (!first)
                {
                    target << data.m_delimiter;
                }

                target << x;
                first = false;
            }

            return target;
        }
    };


    // Allows to send std::byte to streams.
    // Outputs hexadecimal digits.
    template <typename TChar>
    std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, std::byte value)
    {
        return target << HexOutput<int, 2>(int(value));
    }

    // Determines whether the given type is a container
    // with std::byte values.
    template <typename T>
    concept IsContainerOfBytes = requires(const T& x)
    {
        { *std::begin(x) } -> std::convertible_to<const std::byte&>;
        { *std::end(x) } -> std::convertible_to<const std::byte&>;
    };

    static_assert(IsContainerOfBytes<std::vector<std::byte>>);
    static_assert(IsContainerOfBytes<std::byte[42]>);

    // Allows to send containers with std::byte to output streams,
    // such as std::vector<std::byte>, std::array<std::byte>, or std::byte[]
    // Outputs hexadecimal digits.
    // Usage:
    // void main
    // {
    //     std::vector<std::byte> a { 16, 32, 48, 64 };
    //     std::array<std::byte> b { 64, 48, 32, 16 };
    //     std::cout << a << b; // Will type 1020304040302010
    // }
    template <
        typename TChar,
        IsContainerOfBytes TContainer>
    std::basic_ostream<TChar>& operator <<(std::basic_ostream<TChar>& target, TContainer&& byteRange)
    {
        for (const std::byte b : byteRange)
        {
            target << b;
        }

        return target;
    }

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
    template <typename TChar, HasOutputOperator T>
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

    // If the given string has null as the last
    // character, removes it.
    template<typename TChar>
    void TrimTrailingNull(std::basic_string<TChar>& target)
    {
        size_t size{ target.size() };
        if (size == 0)
        {
            return;
        }

        size_t newSize = size - 1;
        if (target[newSize] == TChar{})
        {
            target.resize(newSize);
        }
    }

    // If the given string has null characters at the end,
    // removes them.
    template<typename TChar>
    std::basic_string<TChar>& TrimTrailingNulls(std::basic_string<TChar>& target)
    {
        using MyType = std::basic_string<TChar>;
        if (target.size() > 0)
        {
            const auto newSize = target.find_last_not_of(L'\0');
            if (newSize != MyType::npos)
            {
                target.resize(newSize+1);
            }
        }
        return target;
    }

    template<typename TChar>
    int CompareIgnoreCase(
        std::basic_string<TChar> left,
        std::basic_string<TChar> right,
        size_t partToCompare)
    {
        for (ptrdiff_t i { 0 }; i != partToCompare; ++i)
        {
            if (left.size() == i)
            {
                if (right.size() == i)
                {
                    return true;
                }
                else
                {
                    return -1;
                }
            }
            else if (right.size() == i)
            {
                return +1;
            }
            else
            {
                TChar leftLower { std::tolower(left[i], std::locale::classic()) };
                TChar rightLower { std::tolower(right[i], std::locale::classic()) };
                if (leftLower != rightLower)
                {
                    return leftLower - rightLower;
                }
            }
        }

        return 0;
    }

    template<typename TChar>
    bool StartsWithIgnoreCase(std::basic_string<TChar> stringToTest, std::basic_string<TChar> prefix)
    {
        return CompareIgnoreCase(
            stringToTest,
            prefix,
            prefix.size()) == 0;
    }

    // Converts a UTF-8 string to a std::wstring.
    // Throws std::runtime_error in case of an error.
    std::wstring DecodeUtf8(const std::string& string);

    // Converts the given string to UTF-8.
    // Throws std::runtime_error in case of an error.
    std::string EncodeUtf8(const std::wstring& source);
}
