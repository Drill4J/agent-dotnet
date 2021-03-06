#pragma once

#include <string>
#include <sstream>
#include "LogBuffer.h"
#include "InfoHandler.h"
#include "Connector.h"

namespace Drill4dotNet
{
    template <IsConnector TConnector>
    class ProClient
    {
    private:
        std::wostream& m_ostream;
        std::wistream& m_istream;
        InfoHandler m_infoHandler;
        TConnector m_connector {
            std::function<std::vector<AstEntity>()>{},
            std::function<void(const PackagesPrefixes&)>{} };

    public:
        ProClient()
            : m_ostream(std::wcout),
            m_istream(std::wcin),
            m_infoHandler(m_ostream)
        {
        }

        LogBuffer<std::wostream> Log() const
        {
            return LogBuffer<std::wostream>(m_ostream);
        }

        std::wistream& Key()
        {
            return m_istream;
        }

        InfoHandler& GetInfoHandler()
        {
            return m_infoHandler;
        }

        TConnector& GetConnector()
        {
            return m_connector;
        }
    };
}
