#pragma once

#include <sstream>
#include <iostream>
#include <concepts>

namespace Drill4dotNet
{
    // Wrapper for another stream. Adds a line break each time
    // when a set of << operators is over. All instances are
    // temporary and forbidden to be stored in variables.
    // Example:
    // auto f() { return LogBuffer(std::cout); }
    //
    // void main()
    // {
    //     f() << "Hello, " << "world";
    //     // Line break inserted to the output;
    //     f() << 1 << 2 << 3;
    //     // Another line break inserted to the output;
    // }
    template <typename TStream>
    class LogBuffer
    {
    private:
        // It is safe to have a reference here.
        // This class is neither copyable not moveable.
        // An instance cannot live longer than the expression,
        // which created it, is being executed - because
        // temporary objects are not deleted in the middle of
        // an expression.
        TStream& m_output;
        std::wostringstream m_buffer{};
    public:
        // Creates a new instance.
        // output : the target stream.
        explicit LogBuffer(TStream& output)
            : m_output(output)
        {
        }

        ~LogBuffer()
        {
            m_output << m_buffer.str() << std::endl;
        }

        // This class should not be neither copyable nor moveable.
        // Otherwise, it is possible to keer the instance after
        // the expression, which created it, is finished.
        // In this case it is possible that the m_output reference
        // to become stale.
        LogBuffer(const LogBuffer&) = delete;
        LogBuffer& operator=(const LogBuffer&) = delete;

        template <typename T>
        LogBuffer& operator <<(T&& data)
        {
            m_buffer << data;
            return *this;
        }

        // Adds support for std::hex, std::boolalpha, std::endl, etc
        LogBuffer& operator <<(const std::add_pointer_t<TStream&(TStream&)> manipulator)
        {
            m_buffer << manipulator;
            return *this;
        }
    };

    // Makes an output tool, which automatically adds
    // a line break at the end of series of << operators.
    inline LogBuffer<std::wostream> LogStdout()
    {
        return LogBuffer<std::wostream>(std::wcout);
    }

    // Checks the given type is suitable for usage as
    // a logger in classes derived from ComWrapperBase.
    // The type must have methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename Logger>
    concept IsLogger = requires (const Logger & logger)
    {
        { logger.IsLogEnabled() } -> std::same_as<bool>;
        { logger.Log() };
    };

    // Simple logger satisfying the Logger requirements.
    // Will discard all log entries.
    class TrivialLogger
    {
    private:
        class LogWriter
        {
        public:
            template <typename T>
            constexpr LogWriter& operator<<(T&&) noexcept
            {
                return *this;
            }

            // Adds support for std::hex, std::boolalpha, std::endl, etc
            constexpr LogWriter& operator <<(const std::add_pointer_t<std::basic_ostream<wchar_t>&(std::basic_ostream<wchar_t>&)> manipulator)
            {
                return *this;
            }
        };
    public:
        // Creates a new instance.
        constexpr TrivialLogger() noexcept
        {
        }

        // Determines when log is enabled.
        // This implementation will always return false.
        constexpr bool IsLogEnabled() const noexcept
        {
            return false;
        }

        // Returns the object which can accept log entries.
        // The returned object will ignore all log entries.
        constexpr auto Log() const noexcept
        {
            return LogWriter{};
        }
    };

    static_assert(IsLogger<TrivialLogger>);

    // Log interface for console.
    class ConsoleLogger
    {
    public:
        ConsoleLogger() noexcept
        {
        }

        constexpr bool IsLogEnabled() const noexcept
        {
            return true;
        }

        LogBuffer<std::wostream> Log() const
        {
            return LogBuffer<std::wostream>(std::wcout);
        }
    };

    static_assert(IsLogger<ConsoleLogger>);
}
