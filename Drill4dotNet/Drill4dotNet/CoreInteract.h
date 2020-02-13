#pragma once

#include "CorDataStructures.h"
#include <optional>
#include <vector>

namespace Drill4dotNet
{
    // Additional facility to cache information data structures
    class InfoHandler;

    // Interface to interact with CLR: to request information and to set profiler options
    class ICoreInteract
    {
    public:
        virtual ~ICoreInteract() = default;

        // Gets information about CLR the profiler is running in. Throws on errors.
        virtual RuntimeInformation GetRuntimeInformation() const = 0;

        // Gets information about CLR the profiler is running in. Returns std::nullopt in case of errors.
        virtual std::optional<RuntimeInformation> TryGetRuntimeInformation() const = 0;

        // Sets the mask of events the profiler wants to receive notifications. Throws on errors.
        virtual void SetEventMask(const uint32_t eventMask) const = 0;

        // Sets the mask of events the profiler wants to receive notifications. Returns false on errors.
        virtual bool TrySetEventMask(const uint32_t eventMask) const = 0;

        // Sets the Function Mapper callback. Throws on errors.
        virtual void SetFunctionIDMapper(FunctionIDMapper* pFunc) const = 0;

        // Sets the Function Mapper callback. Returns false on errors.
        virtual bool TrySetFunctionIDMapper(FunctionIDMapper* pFunc) const = 0;

        // Sets Enter/Leave function callbacks (version 2). Throws on errors.
        virtual void SetEnterLeaveFunctionHooks(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall) const = 0;

        // Sets Enter/Leave function callbacks (version 2). Returns false on errors.
        virtual bool TrySetEnterLeaveFunctionHooks(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall) const = 0;

        // Gets information about Application Domain by id. Throws on errors.
        virtual AppDomainInfo GetAppDomainInfo(const AppDomainID appDomainId) const = 0;

        // Gets information about Application Domain by id. Returns std::nullopt in case of errors.
        virtual std::optional<AppDomainInfo> TryGetAppDomainInfo(const AppDomainID appDomainId) const = 0;

        // Gets information about Assembly by id. Throws on errors.
        virtual AssemblyInfo GetAssemblyInfo(const AssemblyID assemblyId) const = 0;

        // Gets information about Assembly by id. Returns std::nullopt in case of errors.
        virtual std::optional<AssemblyInfo> TryGetAssemblyInfo(const AssemblyID assemblyId) const = 0;

        // Gets information about Module by id. Throws on errors.
        virtual ModuleInfo GetModuleInfo(const ModuleID moduleId) const = 0;

        // Gets information about Module by id. Returns std::nullopt in case of errors.
        virtual std::optional<ModuleInfo> TryGetModuleInfo(const ModuleID moduleId) const = 0;

        // Gets information about Class (type) by id. Throws on errors.
        virtual ClassInfo GetClassInfo(const ClassID classId) const = 0;

        // Gets information about Class (type) by id. Returns std::nullopt in case of errors.
        virtual std::optional<ClassInfo> TryGetClassInfo(const ClassID classId) const = 0;

        // Gets information about Functon (Method) by id. Throws on errors.
        virtual FunctionInfo GetFunctionInfo(const FunctionID functionId) const = 0;

        // Gets information about Functon (Method) by id. Returns std::nullopt in case of errors.
        virtual std::optional<FunctionInfo> TryGetFunctionInfo(const FunctionID functionId) const = 0;

        // Gets the Intermediate Language representation of the Function body. Throws on errors.
        // @returns a vector of bytes containing the function body in IL.
        // @param functionInfo : identifies the Function. @see GetFunctionInfo.
        virtual std::vector<std::byte> GetMethodIntermediateLanguageBody(const FunctionInfo& functionInfo) const = 0;

        // Gets the Intermediate Language representation of the Function body. Returns std::nullopt in case of errors.
        // @returns an optional vector of bytes containing the function body in IL.
        // @param functionInfo : identifies the Function. @see GetFunctionInfo.
        virtual std::optional<std::vector<std::byte>> TryGetMethodIntermediateLanguageBody(const FunctionInfo& functionInfo) const = 0;

        // Sets the given the Intermediate Language representation to the Function body. Throws on errors.
        // @param target : identifies the Function. @see GetFunctionInfo.
        // @param newILMethodHeader : a vector of bytes to replace the Function body.
        virtual void SetILFunctionBody(
            const FunctionInfo& target,
            const std::vector<std::byte>& newILMethodBody) const = 0;

        // Sets the given the Intermediate Language representation to the Function body. Returns false on errors.
        // @param target : identifies the Function. @see GetFunctionInfo.
        // @param newILMethodHeader : a vector of bytes to replace the Function body.
        virtual bool TrySetILFunctionBody(
            const FunctionInfo& target,
            const std::vector<std::byte>& newILMethodBody) const = 0;

        virtual void SetInfoCache(std::shared_ptr<InfoHandler>& cache) = 0;
        virtual std::shared_ptr<InfoHandler> GetInfoCache() = 0;
    };

    // Factory-like function to create an instance of `ICoreInteract` interface. Throws on errors.
    // Concrete implementations should be defined and instantiated in a separate cpp file.
    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> CreateCorProfilerInfo(TQueryInterface query, const TLogger logger);

    // Factory-like function to create an instance of `ICoreInteract` interface. Returns unique_ptr(nullptr_t) on errors.
    // Concrete implementations should be defined and instantiated in a separate cpp file.
    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> TryCreateCorProfilerInfo(TQueryInterface query, const TLogger logger);
}
