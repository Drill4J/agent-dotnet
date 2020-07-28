#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <concepts>

#include "AdminDataStructures.h"

namespace Drill4dotNet
{
    using ConnectorReceiveCallback = std::add_pointer_t<void(const char*, const char*)>;

    int64_t GetCurrentTimeMillis();

    // Determines whether the given type can be used for
    // communication between Drill admin and the profiler.
    template <typename T>
    concept IsConnector =
        std::constructible_from<ConnectorReceiveCallback>
        && requires (T x)
    {
        { x.InitializeAgent() } -> std::same_as<void>;
        { x.SendAgentMessage(
            std::declval<const std::string&>(),
            std::declval<const std::string&>(),
            std::declval<const std::string&>()) } -> std::same_as<void>;
        { x.SendPluginMessage(
            std::declval<const std::string&>(),
            std::declval<const std::string&>()) } -> std::same_as<void>;
    };
}
