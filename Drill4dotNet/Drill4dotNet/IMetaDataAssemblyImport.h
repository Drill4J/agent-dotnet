#pragma once

#include <concepts>
#include <optional>

#include "CorDataStructures.h"

namespace Drill4dotNet
{
    // Determines whether the given type can be used as
    // an implementation of IMetadataAssemblyImport.
    template <typename T>
    concept IsMetadataAssemblyImport = requires(const T x, const mdAssembly assembly)
    {
        // Gets the assembly from scope. Throws on error.
        { x.GetAssemblyFromScope() } -> std::same_as<mdAssembly>;

        // Gets the assembly from scope. Returns std::nullopt on error.
        { x.TryGetAssemblyFromScope() } -> std::same_as<std::optional<mdAssembly>>;

        // Gets the information on the given assembly. Throws on error.
        { x.GetAssemblyProps(assembly) } -> std::same_as<AssemblyProps>;

        // Gets the information on the given assembly. Returns std::nullopt on error.
        { x.TryGetAssemblyProps(assembly) } -> std::same_as<std::optional<AssemblyProps>>;
    };
}
