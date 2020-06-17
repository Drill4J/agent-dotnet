#pragma once

#include "Connector.h"
#include <gmock/gmock.h>
#include <functional>

namespace Drill4dotNet
{
    class ConnectorMock
    {
    public:
        MOCK_METHOD(std::function<std::vector<AstEntity>()>&, TreeProvider, ());
        MOCK_METHOD(void, InitializeAgent, ());
        MOCK_METHOD(void, SendAgentMessage, (const std::string&, const std::string&, const std::string&));
        MOCK_METHOD(void, SendPluginMessage, (const std::string&, const std::string&));
        MOCK_METHOD(std::optional<ConnectorQueueItem>, GetNextMessage, ());
        MOCK_METHOD(void, WaitForNextMessage, ());
        MOCK_METHOD(void, WaitForNextMessage, (DWORD));

        ConnectorMock()
        {
        }

        ConnectorMock(const std::function<std::vector<AstEntity>()>&)
            : ConnectorMock()
        {
        }
    };

    static_assert(IsConnector<ConnectorMock>);
}

