#pragma once

#include <string>
#include <sstream>
#include "LogBuffer.h"
#include "InfoHandler.h"
#include "Connector.h"

namespace Drill4dotNet
{
    template <
        IsConnector TConnector,
        IsLogger Logger>
    class ProClient
    {
    private:
        InfoHandler<Logger> m_infoHandler { Logger {} };
        TConnector m_connector {
            std::function<std::vector<AstEntity>()>{},
            std::function<void(const PackagesPrefixes&)>{} };

    public:
        ProClient() = default;

        InfoHandler<Logger>& GetInfoHandler()
        {
            return m_infoHandler;
        }

        TConnector& GetConnector()
        {
            return m_connector;
        }
    };
}
