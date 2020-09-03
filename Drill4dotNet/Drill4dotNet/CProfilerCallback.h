#pragma once

#include <optional>
#include <filesystem>
#include <type_traits>
#include <functional>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>

#include "LogBuffer.h"
#include "ICorProfilerInfo.h"
#include "CProfilerCallbackBase.h"
#include "ComWrapperBase.h"
#include "MethodBody.h"
#include "IMetadataImport.h"
#include "IMetaDataAssemblyImport.h"
#include "IMetadataDispenser.h"
#include "Connector.h"
#include "ProClient.h"
#include "Signature.h"
#include "HttpPost.h"
#include "SessionControl.h"

namespace Drill4dotNet
{
    // Adds function name to the given function info.
    // Throws _com_error in case of an error.
    template <IMetadataImport TMetadataImport>
    FunctionInfo GetFunctionInfo(
        const TMetadataImport& metadataImport,
        const FunctionInfoWithoutName& functionInfo)
    {
        FunctionInfo result;
        result.moduleId = functionInfo.moduleId;
        result.classId = functionInfo.classId;
        result.token = functionInfo.token;
        MethodProps methodProps { metadataImport.GetMethodProps(functionInfo.token) };
        result.name = FunctionName {
            methodProps.Name,
            metadataImport.GetTypeDefProps(methodProps.EnclosingClass).Name };
        return result;
    }

    // Adds function name to the given function info.
    // Returns an empty optional in case of an error.
    template <IMetadataImport TMetadataImport>
    std::optional<FunctionInfo> TryGetFunctionInfo(
        const std::optional<TMetadataImport>& metadataImport,
        const std::optional<FunctionInfoWithoutName>& functionInfo)
    {
        if (!metadataImport.has_value() || !functionInfo.has_value())
        {
            return std::nullopt;
        }

        if (std::optional<MethodProps> methodProps { metadataImport->TryGetMethodProps(functionInfo->token) }
            ; methodProps.has_value())
        {
            if (std::optional<TypeDefProps> classProps{ metadataImport->TryGetTypeDefProps(methodProps->EnclosingClass) }
                ; classProps.has_value())
            {
                FunctionInfo result;
                result.moduleId = functionInfo->moduleId;
                result.classId = functionInfo->classId;
                result.token = functionInfo->token;
                result.name = FunctionName { std::move(methodProps->Name), std::move(classProps->Name) };
                return result;
            }
        }

        return std::nullopt;
    }

    // Adds the class name to the given class info.
    // @param classInfo : the object carrying tokens of the class.
    // @returns the class name and the tokens from classInfo, if obtained, std::nullopt otherwise.
    template <IMetadataImport TMetadataImport>
    std::optional<ClassInfo> TryGetClassInfo(
        const std::optional<TMetadataImport>& metadataImport,
        const std::optional<ClassInfoWithoutName>& classInfo)
    {
        if (!metadataImport.has_value() || !classInfo.has_value())
        {
            return std::nullopt;
        }

        if (std::optional<TypeDefProps> props { metadataImport->TryGetTypeDefProps(classInfo->typeDefToken) }
            ; props.has_value())
        {
            ClassInfo result;
            result.moduleId = classInfo->moduleId;
            result.typeDefToken = classInfo->typeDefToken;
            result.corTypeAttributes = classInfo->corTypeAttributes;
            result.name = std::move(props->Name);
            return result;
        }

        return std::nullopt;
    }

    // Adds the class name to the given class info.
    // @param classInfo : the object carrying tokens of the class.
    // @returns the class name and the tokens from classInfo.
    // Throws _com_error in case of an error.
    template <IMetadataImport TMetadataImport>
    ClassInfo GetClassInfo(
        const TMetadataImport& metadataImport,
        const ClassInfoWithoutName& classInfo)
    {
        ClassInfo result;
        result.moduleId = classInfo.moduleId;
        result.typeDefToken = classInfo.typeDefToken;
        result.corTypeAttributes = classInfo.corTypeAttributes;
        result.name = metadataImport.GetTypeDefProps(classInfo.typeDefToken).Name;
        return result;
    }

    template <
        IsConnector TConnector,
        TCorProfilerInfo CorProfilerInfo,
        IsMetadataDispenser MetaDataDispenser,
        IsMetadataAssemblyImport MetaDataAssemblyImport,
        IMetadataImport MetaDataImport,
        IsLogger Logger>
        class CProfilerCallback :
        public CProfilerCallbackBase
    {
    protected:
        Logger m_logger;

    private:
        inline static constexpr DWORD EVENTS_WE_MONITOR =
            COR_PRF_MONITOR_CLASS_LOADS |
            COR_PRF_MONITOR_MODULE_LOADS |
            COR_PRF_MONITOR_ASSEMBLY_LOADS |
            COR_PRF_MONITOR_APPDOMAIN_LOADS |
            COR_PRF_MONITOR_JIT_COMPILATION |
            COR_PRF_MONITOR_ENTERLEAVE;

        std::optional<
            ProClient<
            std::function<std::vector<AstEntity>()>,
            std::function<void(const PackagesPrefixes&)>,
            std::function<std::vector<ExecClassData>()>,
            TConnector,
            Logger>
        > m_pImplClient;
        std::optional<CorProfilerInfo> m_corProfilerInfo;
        std::optional<std::vector<std::wstring>> m_packagesPrefixes;
        std::optional<std::thread> m_adminInteractionThread;
        std::optional<std::vector<ExecClassData>> m_coverageData;
        std::optional<std::vector<AstEntity>> m_tree;
        std::optional<SessionControl<HttpPost>> m_sessionControl;
        std::function<std::vector<AstEntity>()> m_treeProvider{};
        std::function<void(const PackagesPrefixes&)> m_packagesPrefixesHandler{};
        std::function<std::vector<ExecClassData>()> m_coverageDataSource{};

        inline static CProfilerCallback* g_cb = nullptr;

        static void __stdcall fn_functionEnter(
            FunctionID funcId,
            UINT_PTR clientData,
            COR_PRF_FRAME_INFO func,
            COR_PRF_FUNCTION_ARGUMENT_INFO* argumentInfo
        )
        {
            if (!g_cb) return;

            if (std::optional<FunctionInfo> functionInfo = g_cb->GetInfoHandler().TryGetFunctionInfo(funcId);
                functionInfo.has_value())
            {
                g_cb->m_logger.Log() << L"Enter function: " << functionInfo->fullName();

                if (functionInfo->fullName() == L"HelloWorld.Program.Main")
                { 
                    std::wcout << "this";
                }

                if (const auto typeLocation{ std::find_if(
                    g_cb->m_tree->cbegin(),
                    g_cb->m_tree->cend(),
                    [&functionInfo](const auto& x) { return x.name == functionInfo->name.className; }) }
                ; typeLocation != g_cb->m_tree->cend())
                {
                    const ptrdiff_t typeIndex { typeLocation - g_cb->m_tree->cbegin() };
                    if (const auto methodLocation { std::find_if(
                        typeLocation->methods.cbegin(),
                        typeLocation->methods.cend(),
                        [&functionInfo](const auto x)
                        { return x.name == functionInfo->name.ownName; }
                        )}
                    ; methodLocation != typeLocation->methods.cend())
                    {
                        const ptrdiff_t methodIndex { methodLocation - typeLocation->methods.cbegin() };
                        // if (typeIndex < g_cb->m_coverageData->size()
                        //     && methodIndex < (*(g_cb->m_coverageData))[typeIndex].probes.size())
                        (*(g_cb->m_coverageData))[typeIndex].probes[methodIndex] = true;
                       // (*(g_cb->m_coverageData))[0].probes[0] = true;
                    }
                }
            }
            else
            {
                g_cb->m_logger.Log() << L"Enter function: " << funcId;
            }
            g_cb->GetInfoHandler().FunctionCalled(funcId);
        }

        static void __stdcall fn_functionLeave(
            FunctionID funcId,
            UINT_PTR clientData,
            COR_PRF_FRAME_INFO func,
            COR_PRF_FUNCTION_ARGUMENT_RANGE* retvalRange
        )
        {
            if (!g_cb) return;

            if (std::optional<FunctionInfo> functionInfo = g_cb->GetInfoHandler().TryGetFunctionInfo(funcId);
                functionInfo.has_value())
            {
                g_cb->m_logger.Log() << L"Leave function: " << functionInfo->fullName();
            }
            else
            {
                g_cb->m_logger.Log() << L"Leave function: " << funcId;
            }
        }

        static void __stdcall fn_functionTailcall(
            FunctionID funcId,
            UINT_PTR clientData,
            COR_PRF_FRAME_INFO func
        )
        {
            if (!g_cb) return;

            if (std::optional<FunctionInfo> functionInfo = g_cb->GetInfoHandler().TryGetFunctionInfo(funcId);
                functionInfo.has_value())
            {
                g_cb->m_logger.Log() << L"Tailcall at function: " << functionInfo->fullName();
            }
            else
            {
                g_cb->m_logger.Log() << L"Tailcall at function: " << funcId;
            }
        }

        static UINT_PTR __stdcall fn_FunctionIDMapper(
            FunctionID funcId,
            BOOL* pbHookFunction)
        {
            if (!g_cb) return funcId;

            if (const std::optional<FunctionInfoWithoutName> functionInfoWithoutName{
                    g_cb->GetCorProfilerInfo()->TryGetFunctionInfo(funcId) }
                ; functionInfoWithoutName.has_value())
            {
                if (const std::optional<FunctionInfo> functionInfo { TryGetFunctionInfo(
                    g_cb->GetCorProfilerInfo()->TryGetModuleMetadata(functionInfoWithoutName->moduleId, Logger{}),
                    functionInfoWithoutName) }
                    ; functionInfo.has_value())
                {
                    g_cb->m_logger.Log() << "Mapping   function[" << funcId << "] to " << functionInfo->fullName();
                    g_cb->GetInfoHandler().MapFunctionInfo(funcId, functionInfo.value());
                }
            }

            if (pbHookFunction)
            {
                *pbHookFunction = TRUE; // to receive FunctionEnter2, FunctionLeave2, and FunctionTailcall2 callbacks
            }
            return funcId;
        }

    public:
        CProfilerCallback(Logger logger)
            : m_logger(logger)
        {
            m_treeProvider = [this]()
            {
                std::vector<AstEntity> result{};
                std::vector<ExecClassData> coverageData{};
                uint32_t i { 1 };
                uint32_t j { 1 };
                for (const auto& file : std::filesystem::directory_iterator("."))
                {
                    if (file.path().extension() == L".dll"
                        && (
                            !m_packagesPrefixes.has_value()
                            || std::find_if(
                                m_packagesPrefixes->cbegin(),
                                m_packagesPrefixes->cend(),
                                [file](const std::wstring& prefix)
                                {
                                    return StartsWithIgnoreCase(std::wstring{ file.path().filename() }, prefix);
                                }) != m_packagesPrefixes->cend()))
                    {
                        std::wcout << L"File found: " << file.path() << std::endl;

                        MetaDataDispenser currentDllDispenser { Logger{} };
                        MetaDataAssemblyImport currentDllAssemblyImport {
                            currentDllDispenser.OpenScopeMetaDataAssemblyImport(
                                file.path(), Logger{}) };

                        AssemblyProps assemblyProps { currentDllAssemblyImport.GetAssemblyProps(currentDllAssemblyImport.GetAssemblyFromScope()) };

                        MetaDataImport currentDllImport { currentDllDispenser.OpenScopeMetaDataImport(
                            file.path(),
                            Logger{}) };

                        for (const auto& type : currentDllImport.EnumTypeDefinitions())
                        {
                            AstEntity typeAst {
                                assemblyProps.Name,
                                currentDllImport.GetTypeDefProps(type).Name };
                            ExecClassData coveragePart { j++, typeAst.path + L"/" + typeAst.name };
                            coveragePart.testName = L"my_test";
                            int methodsCount { 0 };
                            for (const auto& method : currentDllImport.EnumMethods(type))
                            {
                                ++methodsCount;
                                MethodProps methodDetails { currentDllImport.GetMethodProps(method) };
                                const ParseResult<MethodSignature> signature{
                                    MethodSignature::Parse(methodDetails.SignatureBlob.cbegin(), methodDetails.SignatureBlob.cend()) };

                                std::wstringstream returnType{};
                                returnType << signature.ParsedValue.ReturnType();
                                AstMethod methodAst {
                                    .name { methodDetails.Name },
                                    .returnType { returnType.str() },
                                    .count { 1 },
                                    .probes { i++ } };

                                for (const auto& param : signature.ParsedValue.ParameterTypes())
                                {
                                    std::wstringstream parameter{};
                                    parameter << param;
                                    methodAst.params.push_back(parameter.str());
                                }

                                typeAst.methods.push_back(methodAst);
                            }

                            coveragePart.probes.resize(methodsCount, false);
                            coverageData.push_back(coveragePart);
                            result.push_back(typeAst);
                        }
                    }
                }

                m_tree = result;
                m_coverageData = std::move(coverageData);
                return result;
            };

            m_packagesPrefixesHandler = std::function{ [this](const PackagesPrefixes& prefixes)
            {
                std::vector<std::wstring> packagesPrefixes{};
                for (auto&& prefix : prefixes.packagesPrefixes)
                {
                    if (prefix != L"")
                    {
                        packagesPrefixes.push_back(std::move(prefix));
                    }
                }

                if (packagesPrefixes.empty())
                {
                    m_packagesPrefixes.reset();
                }
                else
                {
                    m_packagesPrefixes = std::move(packagesPrefixes);
                }
            } };

            m_coverageDataSource = [this]()
            {
                return *m_coverageData;
            };
        }

        ProClient<decltype(m_treeProvider), decltype(m_packagesPrefixesHandler), decltype(m_coverageDataSource), TConnector, Logger>& GetClient()
        {
            return *m_pImplClient;
        }

        const std::optional<CorProfilerInfo>& GetCorProfilerInfo()
        {
            return m_corProfilerInfo;
        }

        InfoHandler<Logger>& GetInfoHandler()
        {
            return m_pImplClient->GetInfoHandler();
        }

        // Inherited via ICorProfilerCallback
        virtual HRESULT __stdcall Initialize(IUnknown* pICorProfilerInfoUnk) override
        {
            m_logger.Log() << L"CProfilerCallback::Initialize";
            try
            {
                m_coverageData.emplace();
                m_pImplClient.emplace(m_treeProvider, m_packagesPrefixesHandler, m_coverageDataSource);

                m_adminInteractionThread.emplace([this]()
                {
                    GetClient().GetConnector().InitializeAgent();

                    while (m_corProfilerInfo.has_value())
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                });

                m_corProfilerInfo.emplace(pICorProfilerInfoUnk, Logger{});

                InjectionMetaData injection;
                MetaDataDispenser metaDataDispenser { Logger{} };
                const std::filesystem::path pathInjection = Drill4dotNet::s_Drill4dotNetLibFilePath.parent_path() / L"Injection.dll";

                MetaDataAssemblyImport metaDataAssemblyImport {
                    metaDataDispenser.OpenScopeMetaDataAssemblyImport(
                        pathInjection, Logger{}) };

                injection.Assembly = metaDataAssemblyImport.GetAssemblyFromScope();

                m_logger.Log()
                    << L"GetAssemblyFromScope token: " << HexOutput(injection.Assembly);

                AssemblyProps assemblyProps { metaDataAssemblyImport.GetAssemblyProps(injection.Assembly) };
                m_logger.Log()
                    << L"GetAssemblyProps() name: "
                    << assemblyProps.Name
                    << L" flags: "
                    << HexOutput(assemblyProps.Flags);

                MetaDataImport metaDataImport { metaDataDispenser.OpenScopeMetaDataImport(
                    pathInjection,
                    Logger{}) };

                injection.Class = metaDataImport.FindTypeDefByName(
#pragma message("TODO: Choose a unique name for injection class.")
                    L"Drill4dotNet.CInjection",
                    NULL);

                m_logger.Log()
                    << L"FindTypeDefByName('Drill4dotNet.CInjection'): "
                    << HexOutput(injection.Class);

                const TypeDefProps typeDefProps { metaDataImport.GetTypeDefProps(injection.Class) };
                m_logger.Log()
                    << L"GetTypeDefProps"
                    << InRoundBrackets(HexOutput(injection.Class))
                    << L": Name: "
                    << " Name: " << typeDefProps.Name
                    << " Flags: " << HexOutput(typeDefProps.Flags);

                std::vector<mdMethodDef> foundMethods { metaDataImport.EnumMethodsWithName(
                    injection.Class,
                    L"FInjection") };

                if (foundMethods.size() != 1)
                {
                    throw std::runtime_error("Invalid meta data information about 'Drill4dotNet.CInjection.FInjection' function.");
                }

#pragma message("TODO: Choose a unique name for injection method.")

                for (const mdMethodDef methodToken : foundMethods)
                {
                    injection.Function = methodToken;
                    m_logger.Log()
                        << L"EnumMethodsWithName('FInjection'): "
                        << L" Method: " << methodToken;
                }

                const MethodProps methodProps { metaDataImport.GetMethodProps(injection.Function) };

                m_logger.Log()
                    << L"GetMethodProps(" << HexOutput(injection.Function) << L"): "
                    << L" TypeDef: " << HexOutput(methodProps.EnclosingClass)
                    << L" Name: " << methodProps.Name
                    << L" Attr: " << HexOutput(methodProps.Attributes)
                    << L" Flags: " << HexOutput(methodProps.ImplementationFlags);

                GetInfoHandler().SetInjectionMetaData(injection);

                if (auto runtimeInformation = m_corProfilerInfo->TryGetRuntimeInformation();
                    runtimeInformation)
                {
                    m_logger.Log()
                        << L"Drill profiler is running in:" << InSpaces(runtimeInformation->RuntimeType())
                        << L"version" << InSpaces(runtimeInformation->Version())
                        << L"QFE version" << InSpaces(runtimeInformation->QFEVersion());
                }

                m_corProfilerInfo->SetEventMask(EVENTS_WE_MONITOR);

                // set the enter, leave and tailcall hooks
                g_cb = this;
                m_corProfilerInfo->SetEnterLeaveFunctionHooks(
                    fn_functionEnter,
                    fn_functionLeave,
                    fn_functionTailcall
                );
                m_corProfilerInfo->SetFunctionIDMapper(fn_FunctionIDMapper);

                std::this_thread::sleep_for(std::chrono::seconds(4));
                m_sessionControl.emplace(
                    "http://localhost:8090/api",
                    L"mysuperAgent",
                    L"{6D2831ED-6A9D-42FB-9375-1ABFAAF81933}",
                    L"AUTO");
            }
            catch (const _com_error& exception)
            {
                HRESULT errorCode = exception.Error();
                m_logger.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
                return errorCode;
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall Shutdown() override
        {
            m_logger.Log() << L"CProfilerCallback::Shutdown";
            try
            {
                m_sessionControl->Stop();
                std::this_thread::sleep_for(std::chrono::seconds(4));
                m_sessionControl.reset();
                g_cb = nullptr;
                GetInfoHandler().OutputStatistics();
                m_corProfilerInfo.reset();
                if (m_adminInteractionThread.has_value())
                {
                    m_adminInteractionThread->join();
                    m_adminInteractionThread.reset();
                }
            }
            catch (const _com_error& exception)
            {
                HRESULT errorCode = exception.Error();
                m_logger.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
                return errorCode;
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall AppDomainCreationStarted(AppDomainID appDomainId) override
        {
            m_logger.Log() << L"CProfilerCallback::AppDomainCreationStarted(" << appDomainId << ")";
            return S_OK;
        }

        virtual HRESULT __stdcall AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::AppDomainCreationFinished(" << appDomainId << "), with status: " << HexOutput(hrStatus);
            try
            {
                // valid when the Finished event is called
                std::optional<AppDomainInfo> info = m_corProfilerInfo->TryGetAppDomainInfo(appDomainId);
                if (info)
                {
                    m_logger.Log() << L"Domain name: " << info->name << L", process id: " << info->processId;
                    GetInfoHandler().MapAppDomainInfo(appDomainId, info.value());
                }
            }
            catch (const _com_error& exception)
            {
                HRESULT errorCode = exception.Error();
                m_logger.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall AppDomainShutdownStarted(AppDomainID appDomainId) override
        {
            m_logger.Log() << L"CProfilerCallback::AppDomainShutdownStarted(" << appDomainId << ")";
            try
            {
                GetInfoHandler().OutputAppDomainInfo(appDomainId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::AppDomainShutdownFinished(" << appDomainId << "), with status: " << HexOutput(hrStatus);
            try
            {
                GetInfoHandler().OutputAppDomainInfo(appDomainId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall AssemblyLoadStarted(AssemblyID assemblyId) override
        {
            m_logger.Log() << L"CProfilerCallback::AssemblyLoadStarted(" << assemblyId << ")";
            return S_OK;
        }

        virtual HRESULT __stdcall AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::AssemblyLoadFinished(" << assemblyId << "), with status: " << HexOutput(hrStatus);
            try
            {
                // valid when the Finished event is called
                std::optional<AssemblyInfo> info = m_corProfilerInfo->TryGetAssemblyInfo(assemblyId);
                if (info)
                {
                    GetInfoHandler().MapAssemblyInfo(assemblyId, info.value());
                    GetInfoHandler().OutputAssemblyInfo(assemblyId);
                }
            }
            catch (const _com_error& exception)
            {
                HRESULT errorCode = exception.Error();
                m_logger.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall AssemblyUnloadStarted(AssemblyID assemblyId) override
        {
            m_logger.Log() << L"CProfilerCallback::AssemblyUnloadStarted(" << assemblyId << ")";
            try
            {
                GetInfoHandler().OutputAssemblyInfo(assemblyId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::AssemblyUnloadFinished(" << assemblyId << "), with status: " << HexOutput(hrStatus);
            try
            {
                GetInfoHandler().OutputAssemblyInfo(assemblyId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall ModuleLoadStarted(ModuleID moduleId) override\
        {
            m_logger.Log() << L"CProfilerCallback::ModuleLoadStarted(" << moduleId << ")";
            return S_OK;
        }

        virtual HRESULT __stdcall ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::ModuleLoadFinished(" << moduleId << ")";
            try
            {
                // valid when the Finished event is called
                std::optional<ModuleInfo> info = m_corProfilerInfo->TryGetModuleInfo(moduleId);
                if (info)
                {
                    GetInfoHandler().MapModuleInfo(moduleId, info.value());
                    GetInfoHandler().OutputModuleInfo(moduleId);
                }
            }
            catch (const _com_error& exception)
            {
                HRESULT errorCode = exception.Error();
                m_logger.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall ModuleUnloadStarted(ModuleID moduleId) override
        {
            m_logger.Log() << L"CProfilerCallback::ModuleUnloadStarted(" << moduleId << ")";
            try
            {
                GetInfoHandler().OutputModuleInfo(moduleId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::ModuleUnloadFinished(" << moduleId << ")";
            try
            {
                GetInfoHandler().OutputModuleInfo(moduleId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall ClassLoadStarted(ClassID classId) override
        {
            m_logger.Log() << L"CProfilerCallback::ClassLoadStarted(" << classId << ")";
            return S_OK;
        }

        virtual HRESULT __stdcall ClassLoadFinished(ClassID classId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::ClassLoadFinished(" << classId << ")";
            try
            {
                // valid when the Finished event is called
                if (const std::optional<ClassInfoWithoutName> classInfoWithoutName { m_corProfilerInfo->TryGetClassInfo(classId) }
                    ; classInfoWithoutName.has_value())
                {

                    if (const std::optional<ClassInfo> info { TryGetClassInfo(
                            m_corProfilerInfo->TryGetModuleMetadata(
                                classInfoWithoutName->moduleId,
                                Logger{}),
                            classInfoWithoutName) }
                    ; info.has_value())
                    {
                        GetInfoHandler().MapClassInfo(classId, info.value());
                        GetInfoHandler().OutputClassInfo(classId);
                    }
                }
            }
            catch (const _com_error& exception)
            {
                HRESULT errorCode = exception.Error();
                m_logger.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall ClassUnloadStarted(ClassID classId) override
        {
            m_logger.Log() << L"CProfilerCallback::ClassUnloadStarted(" << classId << ")";
            try
            {
                GetInfoHandler().OutputClassInfo(classId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall ClassUnloadFinished(ClassID classId, HRESULT hrStatus) override
        {
            m_logger.Log() << L"CProfilerCallback::ClassUnloadFinished(" << classId << ")";
            try
            {
                GetInfoHandler().OutputClassInfo(classId);
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }

        virtual HRESULT __stdcall JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock) override
        {
            m_logger.Log() << L"CProfilerCallback::JITCompilationStarted";
            try
            {
                // This example injection will insert artificial calls to Console.WriteLine
                // into the HelloWorld.Program.MyInjectionTarget function, at the place of
                // the second Console.WriteLine call, and in the finally clause. Also some
                // NOPs will be added to show ability to turn short branching instructions
                // into long ones.

                const FunctionInfoWithoutName functionInfoWithoutName {
                    m_corProfilerInfo->GetFunctionInfo(functionId) };

                const auto moduleMetaData { m_corProfilerInfo->GetModuleMetadata(
                    functionInfoWithoutName.moduleId,
                    Logger{}) };

                const FunctionInfo functionInfo { GetFunctionInfo(moduleMetaData, functionInfoWithoutName) };

                const std::vector<std::byte> signatureBytes { moduleMetaData
                    .GetMethodProps(functionInfo.token)
                    .SignatureBlob };

                const ParseResult<MethodSignature> signature {
                    MethodSignature::Parse(signatureBytes.cbegin(), signatureBytes.cend()) };

                const std::vector<std::byte> functionBytes {
                    m_corProfilerInfo->GetMethodIntermediateLanguageBody(functionInfo) };

                m_logger.Log()
                    << L"Compiling function "
                    << InSquareBrackets(functionId)
                    << L" "
                    << signature.ParsedValue.WritePreamble()
                    << L" "
                    << functionInfo.fullName()
                    << L" "
                    << signature.ParsedValue.WriteParameters()
                    << L" RVA: "
                    << HexOutput(moduleMetaData.GetMethodProps(functionInfo.token).CodeRelativeVirtualAddress)
                    << L" IL Body size: "
                    << functionBytes.size()
                    << L" bytes";

                if (functionInfo.fullName() != L"HelloWorld.Program.MyInjectionTarget")
                {
                    return S_OK;
                }

                auto functionBody = MethodBody(functionBytes);

                m_logger.Log()
                    << L"Initially decompiled raw bytes:"
                    << std::endl
                    << functionBytes
                    << std::endl
                    << L"Initial instructions:"
                    << std::endl
                    << functionBody;

                const bool roundTripOk{ functionBody.Compile() == functionBytes };
                if (roundTripOk)
                {
                    m_logger.Log() << L"Compiling without any injection is OK.";
                }
                else
                {
                    m_logger.Log() << L"Error: got different bytes when compiled method without any injection.";
                    return S_OK;
                }

                const auto findSecondCall = [this, &body{ std::as_const(functionBody) }]()
                {
                    const auto result = FindInstruction<OpCode::CEE_CALL>(
                        FindNextInstruction(
                            FindInstruction<OpCode::CEE_CALL>(
                                body.begin(),
                                body.end()),
                            body.end()),
                        body.end());
                    if (result == body.end())
                    {
                        throw std::logic_error("Error: position for injection was not found. "
                            "Need to update the example or the injection.");
                    }

                    return result;
                };

                functionBody.Insert(
                    findSecondCall() + 1,
                    OpCode::CEE_LDLOC_0{});

                functionBody.Insert(
                    findSecondCall() + 2,
                    OpCode::CEE_LDC_I4_1{});

                Label label = functionBody.CreateLabel();

                functionBody.Insert(
                    findSecondCall() + 3,
                    OpCode::CEE_BLT_S{ ShortJump { label } });

                functionBody.Insert(
                    findSecondCall() + 4,
                    OpCode::CEE_LDC_I4{ 42 });

                const auto callPosition = findSecondCall();
                functionBody.Insert(
                    callPosition + 5,
                    std::get<OpCodeVariant>(*callPosition));

                for (int i = 0; i != 128; ++i)
                {
                    functionBody.Insert(
                        findSecondCall() + 6,
                        OpCode::CEE_NOP{});
                }

                // Presense of stloc.3 instruction means that
                // MyInjectionTarget has been compiled in Debug.
                // This in turn means there is a string variable s,
                // and we can inject Console.WriteLine(s);
                if (FindInstruction<OpCode::CEE_STLOC_3>(
                    functionBody.begin(),
                    functionBody.end()) != functionBody.end())
                {
                    for (const auto& section : functionBody.ExceptionSections())
                    {
                        for (const auto& clause : section.Clauses())
                        {
                            if (!clause.IsFinally())
                            {
                                continue;
                            }

                            const auto& findEndFinally = [this, clause, &body{ std::as_const(functionBody) }]()
                            {
                                return FindInstruction<OpCode::CEE_ENDFINALLY>(
                                    FindLabel(
                                        body.Stream(),
                                        clause.HandlerOffset()),
                                    FindLabel(
                                        body.Stream(),
                                        clause.HandlerEndOffset()));
                            };

                            const auto& findCallInTry = [this, clause, &body{ std::as_const(functionBody) }]()
                            {
                                const auto endTry = FindLabel(
                                    body.Stream(),
                                    clause.TryEndOffset());
                                const auto result = FindInstruction<OpCode::CEE_CALL>(
                                    FindLabel(
                                        body.Stream(),
                                        clause.TryOffset()),
                                    endTry);
                                if (result == endTry)
                                {
                                    throw std::logic_error("Error: position for injection was not found. "
                                        "Need to update the example or the injection.");
                                }

                                return result;
                            };

                            functionBody.Insert(findEndFinally(), OpCode::CEE_LDLOC_3{});
                            functionBody.Insert(
                                findEndFinally(),
                                std::get<OpCodeVariant>(*findCallInTry()));
                        }
                    }
                }

                functionBody.MarkLabel(
                    FindInstruction<OpCode::CEE_RET>(functionBody.begin(), functionBody.end()),
                    label);

                const std::vector<std::byte> afterInjection = functionBody.Compile();
                m_logger.Log()
                    << L"After injection: IL Body size "
                    << afterInjection.size()
                    << L", raw bytes:"
                    << std::endl
                    << afterInjection
                    << std::endl
                    << L"instructions:"
                    << std::endl
                    << functionBody;

                m_corProfilerInfo->SetILFunctionBody(functionInfo, afterInjection);
                m_logger.Log() << L"Injected successfully.";

                return S_OK;
            }
            catch (const _com_error& exception)
            {
                m_logger.Log()
                    << L"COM error: "
                    << HexOutput(exception.Error())
                    << " "
                    << exception.ErrorMessage();

                return exception.Error();
            }
            catch (const std::exception& exception)
            {
                m_logger.Log() << L"Std exception: " << exception.what();
            }
            return S_OK;
        }
    };
}
