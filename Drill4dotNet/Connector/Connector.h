#pragma once
#include <memory>
#include <string>
#include <optional>

namespace Drill4dotNet
{
    class IConnector
    {
    public:
        virtual ~IConnector() = default;
        virtual void InitializeAgent() = 0;
        virtual void SendMessage1(const std::string& content) = 0;
        // gets (and pops) the next message from the queue
        // @returns - next message, if available, std::nullopt otherwise
        virtual std::optional<std::string> GetNextMessage() = 0;
        // waits for availability of a new message in the given timeout
        // returns when the event signaled.
        virtual void WaitForNextMessage(DWORD timeout = INFINITE) = 0;
        static std::shared_ptr<IConnector> CreateInstance();
    };
}
