#pragma once

#include <memory>
#include <string>
#include <optional>
#include <concepts>

namespace Drill4dotNet
{
    // Determines whether the given type can be used for
    // communication between Drill admin and the profiler.
    template <typename T>
    concept IsConnector = requires (T x)
    {
        { x.InitializeAgent() } -> std::same_as<void>;
        { x.SendMessage1(std::declval<const std::string&>()) } -> std::same_as<void>;

        // gets (and pops) the next message from the queue
        // @returns - next message, if available, std::nullopt otherwise
        { x.GetNextMessage() } -> std::same_as<std::optional<std::string>>;

        // waits for availability of a new message in the given timeout
        // returns when the event signaled.
        { x.WaitForNextMessage() } -> std::same_as<void>;
        { x.WaitForNextMessage(std::declval<const DWORD>()) } -> std::same_as<void>;
    };
}
