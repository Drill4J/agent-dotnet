#pragma once

#include <sstream>
#include <iostream>

namespace Drill4dotNet
{
    template <typename TStream>
    class LogBuffer
    {
    private:
        TStream& m_output;
        std::wostringstream m_buffer{};
    public:
        explicit LogBuffer(TStream& output)
            : m_output(output)
        {
        }

        ~LogBuffer()
        {
            m_output << m_buffer.str() << std::endl;
        }

        LogBuffer(LogBuffer&&) = default;
        LogBuffer& operator=(LogBuffer&&) = default;
        LogBuffer(const LogBuffer&) = delete;
        LogBuffer& operator=(const LogBuffer&) = delete;

        template <typename T>
        LogBuffer& operator <<(T&& data)
        {
            m_buffer << data;
            return *this;
        }
    };

    inline LogBuffer<std::wostream> LogStdout()
    {
        return LogBuffer<std::wostream>(std::wcout);
    }
}
