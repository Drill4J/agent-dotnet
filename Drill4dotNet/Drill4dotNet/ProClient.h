#pragma once

#include <string>
#include <sstream>
#include "LogBuffer.h"
#include "InfoHandler.h"

namespace Drill4dotNet
{
    class ProClient
    {
    public:
        ProClient();
        ~ProClient();
        LogBuffer<std::wostream> Log() const;
        std::wistream& Key();

        std::shared_ptr<InfoHandler> GetInfoHandler()
        {
            return m_infoHandler;
        }

    protected:
        std::wostream& m_ostream;
        std::wistream& m_istream;
        std::shared_ptr<InfoHandler> m_infoHandler;
    };
}
