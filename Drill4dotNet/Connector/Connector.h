#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <concepts>

namespace Drill4dotNet
{
    // Represents a message from Drill admin.
    class ConnectorQueueItem
    {
    public:
        // Type of the message.
        std::wstring Destination;

        // The message payload.
        std::wstring Message;
    };

    // Data about a method sent to Drill admin.
    class AstMethod
    {
    public:
        // Name of the method.
        std::wstring name;

        // Lists the parameters.
        std::vector<std::wstring> params;

        // The return type.
        std::wstring returnType;

        // The count of probes added to the method.
        uint32_t count;

        // The identifiers of the probes added to the method.
        std::vector<uint32_t> probes;
    };

    // Data about a class sent to Drill admin.
    class AstEntity
    {
    public:
        // The assembly where the class is located.
        std::wstring path;

        // The name of the class, with namespace.
        std::wstring name;

        // Information about methods in the class.
        std::vector<AstMethod> methods;
    };

    // Determines whether the given functor can be
    // used as a source of classes tree.
    template <typename F>
    concept IsTreeProvider = requires (const F& f)
    {
        { f() } -> std::same_as<std::vector<AstEntity>>;
    };

    // Determines whether the given type can be used for
    // communication between Drill admin and the profiler.
    template <typename T>
    concept IsConnector = requires (T x)
    {
        { x.InitializeAgent() } -> std::same_as<void>;
        { x.SendAgentMessage(
            std::declval<const std::string&>(),
            std::declval<const std::string&>(),
            std::declval<const std::string&>()) } -> std::same_as<void>;
        { x.SendPluginMessage(
            std::declval<const std::string&>(),
            std::declval<const std::string&>()) } -> std::same_as<void>;

        // gets (and pops) the next message from the queue
        // @returns - next message, if available, std::nullopt otherwise
        { x.GetNextMessage() } -> std::same_as<std::optional<ConnectorQueueItem>>;

        // waits for availability of a new message in the given timeout
        // returns when the event signaled.
        { x.WaitForNextMessage() } -> std::same_as<void>;
        { x.WaitForNextMessage(std::declval<const DWORD>()) } -> std::same_as<void>;

        { x.TreeProvider() } -> IsTreeProvider;
    };
}
