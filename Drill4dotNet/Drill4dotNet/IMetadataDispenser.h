#pragma once

#include <filesystem>
#include <optional>
#include <concepts>

#include "ComWrapperBase.h"
#include "IMetadataImport.h"
#include "IMetadataAssemblyImport.h"

namespace Drill4dotNet
{
    // Determines whether the given type is
    // an std::optional of some type.
    template <typename T>
    concept IsOptional = requires
    {
        typename T::value_type;
    }
    && std::is_same_v<T, std::optional<typename T::value_type>>;

    static_assert(IsOptional<std::optional<int>>);

    // Determines whether the given type can be used as
    // an implementation of IMetaDataDispenser.
    template <typename T>
    concept IsMetadataDispenser = requires(
        const T x,
        const std::filesystem::path& path,
        TrivialLogger logger)
    {
        // Returns an implementation of IMetaDataAssemblyImport
        // for the given assembly file. Throws on error.
        { x.OpenScopeMetaDataAssemblyImport(path, logger) } -> IsMetadataAssemblyImport;

        // Returns an implementation of IMetaDataAssemblyImport
        // for the given assembly file. Returns std::nullopt on error.
        { x.TryOpenScopeMetaDataAssemblyImport(path, logger) } -> IsOptional;
        { x.TryOpenScopeMetaDataAssemblyImport(path, logger).value() } -> IsMetadataAssemblyImport;


        // Returns an implementation of IMetaDataImport
        // for the given assembly file. Throws on error.
        { x.OpenScopeMetaDataImport(path, logger) } -> IMetadataImport;

        // Returns an implementation of IMetaDataImport
        // for the given assembly file. Returns std::nullopt on error.
        { x.TryOpenScopeMetaDataImport(path, logger) } -> IsOptional;
        { x.TryOpenScopeMetaDataImport(path, logger).value() } -> IMetadataImport;
    };
}
