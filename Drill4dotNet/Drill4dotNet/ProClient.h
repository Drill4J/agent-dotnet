#pragma once

#include <string>
#include <iostream>

namespace Drill4dotNet
{
    class ProClient
    {
    public:
        ProClient();
        ~ProClient();
        std::wostream& Log();
        std::wistream& Key();
        std::wostream& operator << (const std::wstring& s);
        std::wostream& operator << (std::wstring&& s);
        std::wostream& operator << (std::int16_t i);
        std::wostream& operator << (std::int32_t l);
    protected:
        std::wostream& m_ostream;
        std::wistream& m_istream;
    };
}
