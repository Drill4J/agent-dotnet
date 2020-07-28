#pragma once

#include "Connector.h"
#include <gmock/gmock.h>
#include <functional>

namespace Drill4dotNet
{
    class ConnectorMock
    {
    private:
        std::function<std::vector<AstEntity>()> m_tree{};
        std::function<void(const PackagesPrefixes&)> m_packagesPrefixesHandler{};

    public:
        std::function<std::vector<AstEntity>()>& TreeProvider()
        {
            return m_tree;
        }

        std::function<void(const PackagesPrefixes&)>& PackagesPrefixesHandler()
        {
            return m_packagesPrefixesHandler;
        }

        MOCK_METHOD(void, InitializeAgent, ());
        MOCK_METHOD(void, SendAgentMessage, (const std::string&, const std::string&, const std::string&));
        MOCK_METHOD(void, SendPluginMessage, (const std::string&, const std::string&));

        ConnectorMock()
        {
        }

        ConnectorMock(
            const std::function<std::vector<AstEntity>()>&,
            const std::function<void(const PackagesPrefixes&)>&)
            : ConnectorMock()
        {
        }
    };

    static_assert(IsConnector<ConnectorMock>);
}

