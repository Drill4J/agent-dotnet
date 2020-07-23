#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <concepts>

#include "AdminDataStructures.h"

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

    // Determines whether the given functor can be
    // used as a source of classes tree.
    template <typename F>
    concept IsTreeProvider = requires (const F& f)
    {
        { f() } -> std::same_as<std::vector<AstEntity>>;
    };

    // Determines whether the given functor can be
    // used as a handler of assembly filter.
    template <typename F>
    concept IsPackagesPrefixesHandler = requires (F f, PackagesPrefixes prefixes)
    {
        { f(prefixes) } -> std::same_as<void>;
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
        { x.PackagesPrefixesHandler() } -> IsPackagesPrefixesHandler;
    };
}
