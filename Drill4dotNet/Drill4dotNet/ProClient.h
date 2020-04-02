#pragma once

#include <string>
#include <sstream>
#include "LogBuffer.h"
#include "InfoHandler.h"
#include "Connector.h"

namespace Drill4dotNet
{
    class ProClient
    {
    public:
        ProClient(const std::shared_ptr<IConnector>& connector);
        ~ProClient();
        LogBuffer<std::wostream> Log() const;
        std::wistream& Key();

        InfoHandler& GetInfoHandler()
        {
            return m_infoHandler;
        }

        IConnector& GetConnector()
        {
            return *m_connector;
        }

    protected:
        std::wostream& m_ostream;
        std::wistream& m_istream;
        InfoHandler m_infoHandler;
        std::shared_ptr<IConnector> m_connector;
    };
}
