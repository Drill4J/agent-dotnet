#pragma once

#include <sstream>
#include <iostream>

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
    };

    // Makes an output tool, which automatically adds
    // a line break at the end of series of << operators.
    inline LogBuffer<std::wostream> LogStdout()
    {
        return LogBuffer<std::wostream>(std::wcout);
    }
}
