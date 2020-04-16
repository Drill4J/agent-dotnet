#pragma once

#include "CorDataStructures.h"
#include "IMetadataImport.h"
#include "ComWrapperBase.h"
#include <optional>
#include <vector>
#include <concepts>

namespace Drill4dotNet
{
    // Determines whether the given type can be used as
    // a tool to retrieve data from .net Profiling API.
    template <typename T>
    concept TCorProfilerInfo = requires (const T x)
    {
        // Gets information about CLR the profiler is running in. Throws on errors.
        { x.GetRuntimeInformation() } -> std::same_as<RuntimeInformation>;

        // Gets information about CLR the profiler is running in. Returns std::nullopt in case of errors.
        { x.TryGetRuntimeInformation() } -> std::same_as<std::optional<RuntimeInformation>>;

        // Sets the mask of events the profiler wants to receive notifications. Throws on errors.
        { x.SetEventMask(std::declval<const uint32_t>()) } -> std::same_as<void>;

        // Sets the mask of events the profiler wants to receive notifications. Returns false on errors.
        { x.TrySetEventMask(std::declval<const uint32_t>()) } -> std::same_as<bool>;

        // Sets the Function Mapper callback. Throws on errors.
        { x.SetFunctionIDMapper(std::declval<const FunctionIDMapper* const>()) } -> std::same_as<void>;

        // Sets the Function Mapper callback. Returns false on errors.
        { x.TrySetFunctionIDMapper(std::declval<const FunctionIDMapper* const>()) } -> std::same_as<bool>;

        // Sets Enter/Leave function callbacks (version 2). Throws on errors.
        { x.SetEnterLeaveFunctionHooks(
            std::declval<const FunctionEnter2* const>(),
            std::declval<const FunctionLeave2* const>(),
            std::declval<const FunctionTailcall2* const>())} -> std::same_as<void>;

        // Sets Enter/Leave function callbacks (version 2). Returns false on errors.
        { x.TrySetEnterLeaveFunctionHooks(
            std::declval<const FunctionEnter2* const>(),
            std::declval<const FunctionLeave2* const>(),
            std::declval<const FunctionTailcall2* const>())} -> std::same_as<bool>;

        // Gets information about Application Domain by id. Throws on errors.
        { x.GetAppDomainInfo(std::declval<const AppDomainID>()) } -> std::same_as<AppDomainInfo>;

        // Gets information about Application Domain by id. Returns std::nullopt in case of errors.
        { x.TryGetAppDomainInfo(std::declval<const AppDomainID>()) } -> std::same_as<std::optional<AppDomainInfo>>;

        // Gets information about Assembly by id. Throws on errors.
        { x.GetAssemblyInfo(std::declval<const AssemblyID>()) } -> std::same_as<AssemblyInfo>;

        // Gets information about Assembly by id. Returns std::nullopt in case of errors.
        { x.TryGetAssemblyInfo(std::declval<const AssemblyID>()) } -> std::same_as<std::optional<AssemblyInfo>>;

        // Gets information about Module by id. Throws on errors.
        { x.GetModuleInfo(std::declval<const ModuleID>()) } -> std::same_as<ModuleInfo>;

        // Gets information about Module by id. Returns std::nullopt in case of errors.
        { x.TryGetModuleInfo(std::declval<const ModuleID>()) } -> std::same_as<std::optional<ModuleInfo>>;

        // Gets information about Class (type) by id. Throws on errors.
        { x.GetClassInfo(std::declval<const ClassID>()) } -> std::same_as<ClassInfoWithoutName>;

        // Gets information about Class (type) by id. Returns std::nullopt in case of errors.
        { x.TryGetClassInfo(std::declval<const ClassID>()) } -> std::same_as<std::optional<ClassInfoWithoutName>>;

        // Gets information about Function (Method) by id. Throws on errors.
        { x.GetFunctionInfo(std::declval<const FunctionID>()) } -> std::same_as<FunctionInfoWithoutName>;

        // Gets information about Function (Method) by id. Returns std::nullopt in case of errors.
        { x.TryGetFunctionInfo(std::declval<const FunctionID>()) } -> std::same_as<std::optional<FunctionInfoWithoutName>>;

        // Gets an IMetadataImport object for the
        // given module. Throws on errors.
        { x.GetModuleMetadata(
            std::declval<const ModuleID>(),
            std::declval<const TrivialLogger>()) } -> IMetadataImport;

        // Gets an IMetadataImport object for the
        // given module. Returns std::nullopt on errors.
        { x
            .TryGetModuleMetadata(
                std::declval<const ModuleID>(),
                std::declval<const TrivialLogger>())
            .value() } -> IMetadataImport;

        // Gets the Intermediate Language representation of the Function body. Throws on errors.
        // Returns a vector of bytes containing the function body in IL.
        { x.GetMethodIntermediateLanguageBody(std::declval<const FunctionInfo&>()) } -> std::same_as<std::vector<std::byte>>;

        // Gets the Intermediate Language representation of the Function body. Returns std::nullopt in case of errors.
        // Returns an optional vector of bytes containing the function body in IL.
        { x.TryGetMethodIntermediateLanguageBody(std::declval<const FunctionInfo&>()) }
            -> std::same_as<std::optional<std::vector<std::byte>>>;

        // Sets the given the Intermediate Language representation to the Function body. Throws on errors.
        { x.SetILFunctionBody(
            std::declval<const FunctionInfo&>(),
            std::declval<const std::vector<std::byte>&>()) } -> std::same_as<void>;

        // Sets the given the Intermediate Language representation to the Function body. Returns false on errors.
        { x.TrySetILFunctionBody(
            std::declval<const FunctionInfo&>(),
            std::declval<const std::vector<std::byte>&>()) } -> std::same_as<bool>;
    };
}
