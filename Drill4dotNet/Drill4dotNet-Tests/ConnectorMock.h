#pragma once

#include "Connector.h"
#include <gmock/gmock.h>

namespace Drill4dotNet
{
    class ConnectorMock : public IConnector
    {
    public:
        MOCK_METHOD(void, InitializeAgent, ());
        MOCK_METHOD(void, SendMessage1, (const std::string&));
        MOCK_METHOD(std::optional<std::string>, GetNextMessage, ());
        MOCK_METHOD(void, WaitForNextMessage, (DWORD));
    };
}
