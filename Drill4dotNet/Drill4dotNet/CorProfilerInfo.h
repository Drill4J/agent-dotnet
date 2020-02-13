#pragma once

#include <cstddef>
#include <vector>

#include "framework.h"
#include "CoreInteract.h"
#include "OutputUtils.h"
#include "CorDataStructures.h"
#include "ComWrapperBase.h"
#include "MetaDataImport.h"
#include "MethodMalloc.h"
#include "InfoHandler.h"

namespace Drill4dotNet
{
    // Provides logging and error handling capabilities for ICorProfilerInfo3.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename TLogger>
    class CorProfilerInfo : protected ComWrapperBase<TLogger>, public ICoreInteract
    {
    private:
        ATL::CComQIPtr<ICorProfilerInfo3> m_corProfilerInfo{};
        std::shared_ptr<InfoHandler> m_InfoCache;

        CorProfilerInfo(const TLogger logger) : ComWrapperBase(logger)
        {
        }

        // Updates m_corProfilerInfo with the extracted interface.
        auto InitCallable(IUnknown* pICorProfilerInfoUnk)
        {
            return [&info = m_corProfilerInfo, pICorProfilerInfoUnk]()
            {
                return pICorProfilerInfoUnk->QueryInterface(
                    IID_ICorProfilerInfo2,
                    (LPVOID*)&info);
            };
        }

        inline static const wchar_t s_InitError[] { L"Failed to initialize CorProfilerInfo." };

        // Fills m_corProfilerInfo. Returns false in case of error.
        bool TryInit(IUnknown* pICorProfilerInfoUnk)
        {
            return this->TryCallCom(InitCallable(pICorProfilerInfoUnk), s_InitError);
        }

        // Fills m_corProfilerInfo. Throws in case of error.
        void Init(IUnknown* pICorProfilerInfoUnk)
        {
            this->CallComOrThrow(InitCallable(pICorProfilerInfoUnk), s_InitError);
        }

        // Wraps ICorProfilerInfo3::SetEventMask.
        auto SetEventMaskCallable(DWORD eventMask) const
        {
            return [this, eventMask]()
            {
                return m_corProfilerInfo->SetEventMask(eventMask);
            };
        }

        // Wraps ICorProfilerInfo3::SetEnterLeaveFunctionHooks2.
        auto SetEnterLeaveFunctionHooksCallable(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall) const
        {
            return [this, pFuncEnter, pFuncLeave, pFuncTailcall]()
            {
                return m_corProfilerInfo->SetEnterLeaveFunctionHooks2(
                    pFuncEnter,
                    pFuncLeave,
                    pFuncTailcall);
            };
        }

        // Wraps ICorProfilerInfo3::SetFunctionIDMapper.
        auto SetFunctionIDMapperCallable(FunctionIDMapper* pFunc) const
        {
            return [this, pFunc]()
            {
                return m_corProfilerInfo->SetFunctionIDMapper(pFunc);
            };
        }

        // Wraps ICorProfilerInfo3::GetFunctionInfo.
        auto GetFunctionInfoCallable(const FunctionID functionId, FunctionInfo& result) const
        {
            return [this, functionId, &result]()
            {
                return m_corProfilerInfo->GetFunctionInfo(
                    functionId,
                    &result.classId,
                    &result.moduleId,
                    &result.token);
            };
        }

        // Wraps ICorProfilerInfo3::GetModuleMetaData
        auto GetModuleMetadataCallable(
            const ModuleID moduleId,
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2>& result) const
        {
            return [this, moduleId, &result]()
            {
                return m_corProfilerInfo->GetModuleMetaData(
                    moduleId,
                    CorOpenFlags::ofRead,
                    IID_IMetaDataImport2,
                    (IUnknown**)&result);
            };
        }

        // Wraps ICorProfilerInfo3::GetILFunctionBody
        auto GetMethodIntermediateLanguageBodyCallable(
            const FunctionInfo& functionInfo,
            LPCBYTE& methodHeader,
            ULONG& methodSize) const
        {
            return [this, functionInfo, &methodHeader, &methodSize]()
            {
                return m_corProfilerInfo->GetILFunctionBody(
                    functionInfo.moduleId,
                    functionInfo.token,
                    &methodHeader,
                    &methodSize);
            };
        }

        //  Wraps ICorProfilerInfo3::GetILFunctionBodyAllocator
        auto GetILFunctionBodyAllocatorCallable(
            const ModuleID moduleId,
            ATL::CComQIPtr<IMethodMalloc>& result) const
        {
            return [this, moduleId, &result]()
            {
                return m_corProfilerInfo->GetILFunctionBodyAllocator(
                    moduleId,
                    &result);
            };
        }

        //  Wraps ICorProfilerInfo3::SetILFunctionBody
        auto SetILFunctionBodyCallable(
            const FunctionInfo& target,
            const LPCBYTE pbNewILMethodHeader) const
        {
            return [this, target, pbNewILMethodHeader]
            {
                return m_corProfilerInfo->SetILFunctionBody(
                    target.moduleId,
                    target.token,
                    pbNewILMethodHeader);
            };
        }

        // Copies the method body given by pointer and length, to a separate vector.
        static std::vector<std::byte> CopyBody(const LPCBYTE methodHeader, const ULONG methodSize)
        {
            return std::vector<std::byte>(
                reinterpret_cast<const std::byte*>(methodHeader),
                reinterpret_cast<const std::byte*>(methodHeader + methodSize));
        }

    public:
        // Creates wrapper with logging and error handling capabilities.
        // Throws _com_error in case of an error.
        // pICorProfilerInfoUnk : should provide ICorProfilerInfo3.
        // logger : tool to log the exceptions.
        CorProfilerInfo(IUnknown* pICorProfilerInfoUnk, const TLogger logger)
            : CorProfilerInfo(logger)
        {
            Init(pICorProfilerInfoUnk);
        }

        // Creates wrapper of `ICorProfilerInfo3` with logging and error handling capabilities.
        // @returns CorProfilerInfo instance or std::nullopt in case of an error.
        // @param pICorProfilerInfoUnk : should provide a query for ICorProfilerInfo3.
        // @param logger : a tool to log the exceptions.
        static std::optional<CorProfilerInfo<TLogger>> TryCreate(IUnknown* pICorProfilerInfoUnk, const TLogger logger)
        {
            if (CorProfilerInfo<TLogger> result(logger)
                ; result.TryInit(pICorProfilerInfoUnk))
            {
                return result;
            }

            return std::nullopt;
        }

        void SetInfoCache(std::shared_ptr<InfoHandler>& cache) override
        {
            m_InfoCache = cache;
        }

        std::shared_ptr<InfoHandler> GetInfoCache() override
        {
            return m_InfoCache;
        }

        // Calls ICorProfilerInfo3::GetFunctionInfo with the given functionId.
        // Throws _com_error in case of an error.
        FunctionInfo GetFunctionInfo(const FunctionID functionId) const override
        {
            if (m_InfoCache)
            {
                if (const auto oInfo = m_InfoCache->TryGetFunctionInfo(functionId);
                    oInfo.has_value())
                {
                    return oInfo.value();
                }
            }
            FunctionInfo result;
            this->CallComOrThrow(
                GetFunctionInfoCallable(functionId, result),
                L"Failed to call CorProfilerInfo::GetFunctionInfo.");

            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImportPtr{};
            mdToken functionToken;
            CallComOrThrow(
                [this, functionId, &metaDataImportPtr, &functionToken]()
                {
                    return m_corProfilerInfo->GetTokenAndMetaDataFromFunction(
                        functionId,
                        IID_IMetaDataImport2,
                        (IUnknown**)&metaDataImportPtr,
                        &functionToken);
                },
                L"Calling ICorProfilerInfo3::GetTokenAndMetaDataFromFunction.");
            MetaDataImport<TLogger> metaDataImport(metaDataImportPtr, m_logger);
            result.name = metaDataImport.GetFunctionFullName(functionToken);
            if (m_InfoCache)
            {
                m_InfoCache->MapFunctionInfo(functionId, result);
            }
            return result;
        }

        // Calls ICorProfilerInfo3::GetFunctionInfo with the given functionId.
        // Returns an empty optional in case of an error.
        std::optional<FunctionInfo> TryGetFunctionInfo(const FunctionID functionId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetFunctionInfo(functionId);
                    oInfo.has_value())
                {
                    return oInfo;
                }
            }
            if (FunctionInfo result
                ; this->TryCallCom(
                    GetFunctionInfoCallable(functionId, result),
                    L"Failed to call CorProfilerInfo::TryGetFunctionInfo."))
            {
                ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImportPtr{};
                if (mdToken functionToken;
                    TryCallCom(
                    [this, functionId, &metaDataImportPtr, &functionToken]()
                    {
                        return m_corProfilerInfo->GetTokenAndMetaDataFromFunction(
                            functionId,
                            IID_IMetaDataImport2,
                            (IUnknown**)&metaDataImportPtr,
                            &functionToken);
                    },
                    L"Calling ICorProfilerInfo3::GetTokenAndMetaDataFromFunction."))
                {
                    MetaDataImport<TLogger> metaDataImport(metaDataImportPtr, m_logger);
                    if (auto oFullName = metaDataImport.TryGetFunctionFullName(functionToken);
                        oFullName.has_value())
                    {
                        result.name = oFullName.value();
                        if (m_InfoCache)
                        {
                            m_InfoCache->MapFunctionInfo(functionId, result);
                        }
                        return result;
                    }
                }
            }

            return std::nullopt;
        }

    private:
        // Calls ICorProfilerInfo3::GetModuleMetadata and creates
        // a MetadataImport wrapper around it.
        // The resulting object will become independent, and because
        // some logging context is required to create it, user of this
        // function must provide loggerForMetadata.
        // Throws _com_error in case of an error.
        template <typename TMetaDataLogger = TLogger>
        MetaDataImport<TMetaDataLogger> GetModuleMetadata(
            const ModuleID moduleId,
            const TMetaDataLogger loggerForMetadata) const
        {
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport{};
            CallComOrThrow(
                GetModuleMetadataCallable(moduleId, metaDataImport),
                L"Failed to call CorProfilerInfo::GetModuleMetadata.");
            return MetaDataImport<TMetaDataLogger>(metaDataImport, loggerForMetadata);
        }

        // Calls ICorProfilerInfo3::GetModuleMetadata and creates
        // a MetadataImport wrapper around it.
        // The resulting object will become independent, and because
        // some logging context is required to create it, user of this
        // function must provide loggerForMetadata.
        // Returns an empty optional in case of an error.
        template <typename TMetaDataLogger = TLogger>
        std::optional<MetaDataImport<TMetaDataLogger>> TryGetModuleMetadata(
            const ModuleID moduleId,
            const TMetaDataLogger loggerForMetadata) const
        {
            if (ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport{}
                ; TryCallCom(
                    GetModuleMetadataCallable(moduleId, metaDataImport),
                    L"Failed to call CorProfilerInfo::TryGetModuleMetadata."))
            {
                return MetaDataImport<TMetaDataLogger>(metaDataImport, loggerForMetadata);
            }

            return std::nullopt;
        }

    public:
        // Calls ICorProfilerInfo3::GetILFunctionBody with the given FunctionInfo.
        // Returns vector with a copy of the bytes of the Intermediate Language
        // representation of the function body.
        // Throws _com_error in case of an error.
        std::vector<std::byte> GetMethodIntermediateLanguageBody(const FunctionInfo& functionInfo) const override
        {
            LPCBYTE methodHeader;
            ULONG methodSize;
            this->CallComOrThrow(
                GetMethodIntermediateLanguageBodyCallable(
                    functionInfo,
                    methodHeader,
                    methodSize),
                L"Failed to call CorProfilerInfo::GetMethodIntermediateLanguageBody.");

            return CopyBody(methodHeader, methodSize);
        }

        // Calls ICorProfilerInfo3::GetILFunctionBody with the given FunctionInfo.
        // Returns vector with a copy of the bytes of the Intermediate Language
        // representation of the function body.
        // Returns an empty optional in case of an error.
        std::optional<std::vector<std::byte>> TryGetMethodIntermediateLanguageBody(const FunctionInfo& functionInfo) const override
        {
            LPCBYTE methodHeader;
            ULONG methodSize;
            if (this->TryCallCom(
                GetMethodIntermediateLanguageBodyCallable(
                    functionInfo,
                    methodHeader,
                    methodSize),
                L"Failed to call CorProfilerInfo::TryGetMethodIntermediateLanguageBody."))
            {
                return CopyBody(methodHeader, methodSize);
            }

            return std::nullopt;
        }

        // Calls ICorProfilerInfo3::SetEventMask with the given mask.
        // Throws _com_error in case of an error.
        void SetEventMask(const uint32_t eventMask) const override
        {
            this->CallComOrThrow(SetEventMaskCallable(eventMask), L"Failed to call CorProfilerInfo::SetEventMask.");
        }

        // Calls ICorProfilerInfo3::SetEventMask with the given mask.
        // Returns false in case of an error.
        bool TrySetEventMask(const uint32_t eventMask) const override
        {
            return this->TryCallCom(SetEventMaskCallable(eventMask) , L"Failed to call CorProfilerInfo::TrySetEventMask.");
        }

        // Calls ICorProfilerInfo3::SetEnterLeaveFunctionHooks2 with the given parameters.
        // Throws _com_error in case of an error.
        void SetEnterLeaveFunctionHooks(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall) const override
        {
            this->CallComOrThrow(
                SetEnterLeaveFunctionHooksCallable(
                    pFuncEnter,
                    pFuncLeave,
                    pFuncTailcall),
                L"Failed to call CorProfilerInfo::SetEnterLeaveFunctionHooks.");
        }

        // Calls ICorProfilerInfo3::SetEnterLeaveFunctionHooks2 with the given parameters.
        // Returns false in case of an error.
        bool TrySetEnterLeaveFunctionHooks(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall) const override
        {
            return this->TryCallCom(
                SetEnterLeaveFunctionHooksCallable(
                    pFuncEnter,
                    pFuncLeave,
                    pFuncTailcall),
                L"Failed to call CorProfilerInfo::TrySetEnterLeaveFunctionHooks.");
        }

        // Calls ICorProfilerInfo3::SetFunctionIDMapper
        // Throws _com_error in case of an error.
        void SetFunctionIDMapper(FunctionIDMapper* pFunc) const override
        {
            this->CallComOrThrow(
                SetFunctionIDMapperCallable(pFunc),
                L"Failed to call CorProfilerInfo::SetFunctionIDMapper.");
        }

        // Calls ICorProfilerInfo3::SetFunctionIDMapper
        // Returns false in case of an error.
        bool TrySetFunctionIDMapper(FunctionIDMapper* pFunc) const override
        {
            return this->TryCallCom(
                SetFunctionIDMapperCallable(pFunc),
                L"Failed to call CorProfilerInfo::TrySetFunctionIDMapper.");
        }

        // Gets application domain information
        // Wraps ICorProfilerInfo3::GetAppDomainInfo
        // @returns AppDomain's name and process, if obtained, std::nullopt otherwise.
        std::optional<AppDomainInfo> TryGetAppDomainInfo(const AppDomainID appDomainId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetAppDomainInfo(appDomainId);
                    oInfo.has_value())
                {
                    return oInfo;
                }
            }
            AppDomainInfo info;
            if (ULONG cchName;
                TryCallCom(
                [this, appDomainId, &cchName, &info]()
                {
                    return m_corProfilerInfo->GetAppDomainInfo(
                        appDomainId,
                        0,
                        &cchName,
                        nullptr,
                        &info.processId);
                },
                L"Calling ICorProfilerInfo3::GetAppDomainInfo 1-st try."))
            {
                info.name.resize(cchName, L'\0');
                if (cchName > 0 && TryCallCom(
                    [this, appDomainId, cchName , &info]()
                    {
                        ULONG cchDrop;
                        return m_corProfilerInfo->GetAppDomainInfo(
                            appDomainId,
                            cchName,
                            &cchDrop,
                            info.name.data(),
                            &info.processId);
                    },
                    L"Calling ICorProfilerInfo3::GetAppDomainInfo 2-nd try."))
                {
                    TrimTrailingNull(info.name);
                }
                if (m_InfoCache)
                {
                    m_InfoCache->MapAppDomainInfo(appDomainId, info);
                }
                return info;
            }
            return std::nullopt;
        }

        // Gets application domain information
        // Throws _com_error in case of an error.
        AppDomainInfo GetAppDomainInfo(const AppDomainID appDomainId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetAppDomainInfo(appDomainId);
                    oInfo.has_value())
                {
                    return oInfo.value();
                }
            }
            AppDomainInfo info;
            ULONG cchName;
            CallComOrThrow(
                [this, appDomainId, &cchName, &info]()
                {
                    return m_corProfilerInfo->GetAppDomainInfo(
                        appDomainId,
                        0,
                        &cchName,
                        nullptr,
                        &info.processId);
                },
                L"Calling ICorProfilerInfo3::GetAppDomainInfo 1-st try.");

            if (cchName > 0)
            {
            info.name.resize(cchName, L'\0');

            CallComOrThrow(
                [this, appDomainId, cchName, &info]()
                {
                    ULONG cchDrop;
                    return m_corProfilerInfo->GetAppDomainInfo(
                        appDomainId,
                        cchName,
                        &cchDrop,
                        info.name.data(),
                        &info.processId);
                },
                L"Calling ICorProfilerInfo3::GetAppDomainInfo 2-nd try.");

            TrimTrailingNull(info.name);
            }
            if (m_InfoCache)
            {
                m_InfoCache->MapAppDomainInfo(appDomainId, info);
            }
            return info;
        }

        // Gets assembly information
        // Wraps ICorProfilerInfo3::GetAssemblyInfo
        // @returns Assembly's name, domain, and module, if obtained, std::nullopt otherwise.
        std::optional<AssemblyInfo> TryGetAssemblyInfo(const AssemblyID assemblyId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetAssemblyInfo(assemblyId);
                    oInfo.has_value())
                {
                    return oInfo;
                }
            }
            AssemblyInfo info;
            if (ULONG cchName;
                TryCallCom(
                [this, assemblyId, &cchName, &info]()
                {
                    return m_corProfilerInfo->GetAssemblyInfo(
                        assemblyId,
                        0,
                        &cchName,
                        nullptr,
                        &info.appDomainId,
                        &info.moduleId);
                },
                L"Calling ICorProfilerInfo3::GetAssemblyInfo 1-st try."))
            {
                info.name.resize(cchName, L'\0');
                if (cchName > 0 && TryCallCom(
                    [this, assemblyId, cchName, &info]()
                    {
                        ULONG cchDrop;
                        return m_corProfilerInfo->GetAssemblyInfo(
                            assemblyId,
                            cchName,
                            &cchDrop,
                            info.name.data(),
                            &info.appDomainId,
                            &info.moduleId);
                    },
                    L"Calling ICorProfilerInfo3::GetAssemblyInfo 2-nd try."))
                {
                    TrimTrailingNull(info.name);
                }
                if (m_InfoCache)
                {
                    m_InfoCache->MapAssemblyInfo(assemblyId, info);
                }
                return info;
            }
            return std::nullopt;
        }

        // Gets assembly information
        // Wraps ICorProfilerInfo3::GetAssemblyInfo
        // Throws _com_error in case of an error.
        AssemblyInfo GetAssemblyInfo(const AssemblyID assemblyId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetAssemblyInfo(assemblyId);
                    oInfo.has_value())
                {
                    return oInfo.value();
                }
            }
            AssemblyInfo info;
            ULONG cchName;
            CallComOrThrow(
                [this, assemblyId, &cchName, &info]()
                {
                    return m_corProfilerInfo->GetAssemblyInfo(
                        assemblyId,
                        0,
                        &cchName,
                        nullptr,
                        &info.appDomainId,
                        &info.moduleId);
                },
                L"Calling ICorProfilerInfo3::GetAssemblyInfo 1-st try.");

            if (cchName > 0)
            {
            info.name.resize(cchName, L'\0');
            CallComOrThrow(
                [this, assemblyId, cchName, &info]()
                {
                    ULONG cchDrop;
                    return m_corProfilerInfo->GetAssemblyInfo(
                        assemblyId,
                        cchName,
                        &cchDrop,
                        info.name.data(),
                        &info.appDomainId,
                        &info.moduleId);
                },
                L"Calling ICorProfilerInfo3::GetAssemblyInfo 2-nd try.");
            TrimTrailingNull(info.name);
            }
            if (m_InfoCache)
            {
                m_InfoCache->MapAssemblyInfo(assemblyId, info);
            }
            return info;
        }

        // Gets module information
        // Wraps ICorProfilerInfo3::GetModuleInfo
        // @returns Module's name, assembly, and base load address, if obtained, std::nullopt otherwise.
        std::optional<ModuleInfo> TryGetModuleInfo(const ModuleID moduleId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetModuleInfo(moduleId);
                    oInfo.has_value())
                {
                    return oInfo;
                }
            }
            ModuleInfo info;
            if (ULONG cchName;
                TryCallCom(
                [this, moduleId, &cchName, &info]()
                {
                    return m_corProfilerInfo->GetModuleInfo(
                        moduleId,
                        &info.baseLoadAddress,
                        0,
                        &cchName,
                        nullptr,
                        &info.assemblyId);
                },
                L"Calling ICorProfilerInfo3::GetModuleInfo 1-st try."))
            {
                info.name.resize(cchName, L'\0');
                if (cchName > 0 && TryCallCom(
                    [this, moduleId, cchName, &info]()
                    {
                        ULONG cchDrop;
                        return m_corProfilerInfo->GetModuleInfo(
                            moduleId,
                            &info.baseLoadAddress,
                            cchName,
                            &cchDrop,
                            info.name.data(),
                            &info.assemblyId);
                    },
                    L"Calling ICorProfilerInfo3::GetModuleInfo 2-nd try."))
                {
                    TrimTrailingNull(info.name);
                }
                if (m_InfoCache)
                {
                    m_InfoCache->MapModuleInfo(moduleId, info);
                }
                return info;
            }
            return std::nullopt;
        }

        // Gets module information
        // Wraps ICorProfilerInfo3::GetModuleInfo
        // Throws _com_error in case of an error.
        ModuleInfo GetModuleInfo(const ModuleID moduleId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetModuleInfo(moduleId);
                    oInfo.has_value())
                {
                    return oInfo.value();
                }
            }
            ModuleInfo info;
            ULONG cchName;
            CallComOrThrow(
                [this, moduleId, &cchName, &info]()
                {
                    return m_corProfilerInfo->GetModuleInfo(
                        moduleId,
                        &info.baseLoadAddress,
                        0,
                        &cchName,
                        nullptr,
                        &info.assemblyId);
                },
                L"Calling ICorProfilerInfo3::GetModuleInfo 1-st try.");

            if (cchName > 0)
            {
            info.name.resize(cchName, L'\0');
            CallComOrThrow(
                [this, moduleId, cchName, &info]()
                {
                    ULONG cchDrop;
                    return m_corProfilerInfo->GetModuleInfo(
                        moduleId,
                        &info.baseLoadAddress,
                        cchName,
                        &cchDrop,
                        info.name.data(),
                        &info.assemblyId);
                },
                L"Calling ICorProfilerInfo3::GetModuleInfo 2-nd try.");
            TrimTrailingNull(info.name);
            }
            if (m_InfoCache)
            {
                m_InfoCache->MapModuleInfo(moduleId, info);
            }
            return info;
        }

        // Gets class (type) information
        // It wraps ICorProfilerInfo3::GetClassIDInfo and IMetaDataImport2::GetTypeDefProps
        // @param classId : ID of the class
        // @returns Class's name, module, token, if obtained, std::nullopt otherwise.
        std::optional<ClassInfo> TryGetClassInfo(const ClassID classId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetClassInfo(classId);
                    oInfo.has_value())
                {
                    return oInfo;
                }
            }
            ClassInfo info;
            if (TryCallCom(
                [this, classId, &info]()
                {
                    return m_corProfilerInfo->GetClassIDInfo(
                        classId,
                        &info.moduleId,
                        &info.typeDefToken);
                },
                L"Calling ICorProfilerInfo3::GetClassIDInfo."))
            {
                auto metadata = GetModuleMetadata(info.moduleId, m_logger);
                if (const auto oName = metadata.TryGetTypeName(info.typeDefToken);
                    oName.has_value())
                {
                    info.name = oName.value();
                    if (m_InfoCache)
                    {
                        m_InfoCache->MapClassInfo(classId, info);
                    }
                    return info;
                }
            }
            return std::nullopt;
        }

        // Gets class (type) information
        // It wraps ICorProfilerInfo3::GetClassIDInfo and IMetaDataImport2::GetTypeDefProps
        // @param classId : ID of the class
        // Throws _com_error in case of an error.
        ClassInfo GetClassInfo(const ClassID classId) const override
        {
            if (m_InfoCache)
            {
                if (auto oInfo = m_InfoCache->TryGetClassInfo(classId);
                    oInfo.has_value())
                {
                    return oInfo.value();
                }
            }
            ClassInfo info;
            CallComOrThrow(
                [this, classId, &info]()
                {
                    return m_corProfilerInfo->GetClassIDInfo(
                        classId,
                        &info.moduleId,
                        &info.typeDefToken);
                },
                L"Calling ICorProfilerInfo3::GetClassIDInfo.");

            auto metadata = GetModuleMetadata(info.moduleId, m_logger);
            info.name = metadata.GetTypeName(info.typeDefToken);
            if (m_InfoCache)
            {
                m_InfoCache->MapClassInfo(classId, info);
            }
            return info;
        }

        std::optional<RuntimeInformation> TryGetRuntimeInformation() const override
        {
            RuntimeInformation info;
            if (TryCallCom(
                [this, &info]()
                {
                    return m_corProfilerInfo->GetRuntimeInformation(
                        &info.clrInstanceId,
                        &info.runtimeType,
                        &info.majorVersion,
                        &info.minorVersion,
                        &info.buildNumber,
                        &info.qfeVersion,
                        0,
                        nullptr,
                        nullptr);
                },
                L"Calling ICorProfilerInfo3::GetRuntimeInformation."))
            {
                return info;
            }
            return std::nullopt;
        }

        RuntimeInformation GetRuntimeInformation() const override
        {
            RuntimeInformation info;
            CallComOrThrow(
                [this, &info]()
                {
                    return m_corProfilerInfo->GetRuntimeInformation(
                        &info.clrInstanceId,
                        &info.runtimeType,
                        &info.majorVersion,
                        &info.minorVersion,
                        &info.buildNumber,
                        &info.qfeVersion,
                        0,
                        nullptr,
                        nullptr);
                },
                L"Calling ICorProfilerInfo3::GetRuntimeInformation.");

            return info;
        }

    private:
        // Calls ICorProfilerInfo3::GetILFunctionBodyAllocator and creates
        // a MethodMalloc wrapper around it.
        // The resulting object will become independent, and because
        // some logging context is required to create it, user of this
        // function must provide loggerForMalloc.
        // Throws _com_error in case of an error.
        template <typename TMetaDataLogger = TLogger>
        MethodMalloc<TMetaDataLogger> GetILFunctionBodyAllocator(
            const ModuleID moduleId,
            const TMetaDataLogger loggerForMalloc) const
        {
            ATL::CComQIPtr<IMethodMalloc> methodMalloc{};
            CallComOrThrow(
                GetILFunctionBodyAllocatorCallable(moduleId, methodMalloc),
                L"Failed to call CorProfilerInfo::GetILFunctionBodyAllocator");
            return MethodMalloc(methodMalloc, loggerForMalloc);
        }

        // Calls ICorProfilerInfo3::GetILFunctionBodyAllocator and creates
        // a MethodMalloc wrapper around it.
        // The resulting object will become independent, and because
        // some logging context is required to create it, user of this
        // function must provide loggerForMalloc.
        // Returns std::nullopt in case of an error.
        template <typename TMetaDataLogger = TLogger>
        std::optional<MethodMalloc<TMetaDataLogger>> TryGetILFunctionBodyAllocator(
            const ModuleID moduleId,
            const TMetaDataLogger loggerForMalloc) const
        {
            if (ATL::CComQIPtr<IMethodMalloc> methodMalloc{}
                ; this->TryCallCom(
                    GetILFunctionBodyAllocatorCallable(moduleId, methodMalloc),
                    L"Failed to call CorProfilerInfo::TryGetILFunctionBodyAllocator"))
            {
                return MethodMalloc(methodMalloc, loggerForMalloc);
            }

            return std::nullopt;
        }

    public:
        // Sets the given byte representation of a .net method body as
        // an implementation for the given method.
        // Calls ICorProfilerInfo3::SetILFunctionBody.
        // Throws _com_error in case of an error.
        // @param target : the target method
        // @param newILMethodBody : vector of bytes containing the new method body
        void SetILFunctionBody(
            const FunctionInfo& target,
            const std::vector<std::byte>& newILMethodBody) const override
        {
            MethodMalloc allocator = GetILFunctionBodyAllocator(
                target.moduleId,
                m_logger);

            BYTE* buffer = static_cast<BYTE*>(
                allocator.Alloc(
                static_cast<uint32_t>(newILMethodBody.size())));

            std::copy(
                newILMethodBody.cbegin(),
                newILMethodBody.cend(),
                (std::byte*)(buffer));

            CallComOrThrow(
                SetILFunctionBodyCallable(
                    target,
                    buffer),
                L"Failed to call CorProfilerInfo::SetILFunctionBody"
            );
        }

        // Sets the given byte representation of a .net method body as
        // an implementation for the given method.
        // Calls ICorProfilerInfo3::SetILFunctionBody.
        // Returns false in case of an error.
        // @param target : the target method
        // @param newILMethodBody : vector of bytes containing the new method body
        bool TrySetILFunctionBody(
            const FunctionInfo& target,
            const std::vector<std::byte>& newILMethodBody) const override
        {
            MethodMalloc allocator = GetILFunctionBodyAllocator(
                target.moduleId,
                m_logger);

            BYTE* buffer = static_cast<BYTE*>(
                allocator.Alloc(
                static_cast<uint32_t>(newILMethodBody.size())));

            std::copy(
                newILMethodBody.cbegin(),
                newILMethodBody.cend(),
                (std::byte*)(buffer));

            return this->TryCallCom(
                SetILFunctionBodyCallable(
                    target,
                    buffer),
                L"Failed to call CorProfilerInfo::TrySetILFunctionBody"
            );
        }
    };
}
