#pragma once

#include <string>
#include <sstream>
#include "LogBuffer.h"

namespace Drill4dotNet
{
    class ProClient
    {
    public:
        ProClient();
        ~ProClient();
        LogBuffer<std::wostream> Log();
        std::wistream& Key();

    protected:
        std::wostream& m_ostream;
        std::wistream& m_istream;
    };
}
