#pragma once

#include <cstddef>
#include <vector>

#include "framework.h"
#include "OutputUtils.h"
#include "FunctionInfo.h"
#include "ComWrapperBase.h"
#include "MetaDataImport2.h"

namespace Drill4dotNet
{
    // Provides logging and error handling capabilities for ICorProfilerInfo2.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename TLogger>
    class CorProfilerInfo2 : protected ComWrapperBase<TLogger>
    {
    private:
        ATL::CComQIPtr<ICorProfilerInfo2> m_corProfilerInfo2{};

        CorProfilerInfo2(const TLogger logger) : ComWrapperBase(logger)
        {
        }

        // Updates m_corProfilerInfo2 with the extracted interface.
        auto InitCallable(IUnknown* pICorProfilerInfoUnk)
        {
            return [&info = m_corProfilerInfo2, &pICorProfilerInfoUnk]()
            {
                return pICorProfilerInfoUnk->QueryInterface(
                    IID_ICorProfilerInfo2,
                    (LPVOID*)&info);
            };
        }

        inline static const wchar_t s_InitError[] { L"Failed to initialize CorProfilerInfo2." };

        // Fills m_corProfilerInfo2. Returns false in case of error.
        bool TryInit(IUnknown* pICorProfilerInfoUnk)
        {
            return this->TryCallCom(InitCallable(pICorProfilerInfoUnk), s_InitError);
        }

        // Fills m_corProfilerInfo2. Throws in case of error.
        void Init(IUnknown* pICorProfilerInfoUnk)
        {
            this->CallComOrThrow(InitCallable(pICorProfilerInfoUnk), s_InitError);
        }

        // Wraps ICorProfilerInfo2::SetEventMask.
        auto SetEventMaskCallable(DWORD eventMask)
        {
            return [this, eventMask]()
            {
                return m_corProfilerInfo2->SetEventMask(eventMask);
            };
        }

        // Wraps ICorProfilerInfo2::SetEnterLeaveFunctionHooks2.
        auto SetEnterLeaveFunctionHooks2Callable(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall)
        {
            return [this, pFuncEnter, pFuncLeave, pFuncTailcall]()
            {
                return m_corProfilerInfo2->SetEnterLeaveFunctionHooks2(
                    pFuncEnter,
                    pFuncLeave,
                    pFuncTailcall);
            };
        }

        // Wraps ICorProfilerInfo2::GetFunctionInfo.
        auto GetFunctionInfoCallable(const FunctionID functionId, FunctionInfo& result) const
        {
            return [this, functionId, &result]()
            {
                return m_corProfilerInfo2->GetFunctionInfo(
                    functionId,
                    &result.classId,
                    &result.moduleId,
                    &result.token);
            };
        }

        // Wraps ICorProfilerInfo2::GetModuleMetaData
        auto GetModuleMetadataCallable(
            const ModuleID moduleId,
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2>& result) const
        {
            return [this, moduleId, &result]()
            {
                return m_corProfilerInfo2->GetModuleMetaData(
                    moduleId,
                    CorOpenFlags::ofRead,
                    IID_IMetaDataImport2,
                    (IUnknown**)&result);
            };
        }

        // Wraps ICorProfilerInfo2::GetILFunctionBody
        auto GetMethodIntermediateLanguageBodyCallable(
            const FunctionInfo& functionInfo,
            LPCBYTE& methodHeader,
            ULONG& methodSize) const
        {
            return [this, &functionInfo, &methodHeader, &methodSize]()
            {
                return m_corProfilerInfo2->GetILFunctionBody(
                    functionInfo.moduleId,
                    functionInfo.token,
                    &methodHeader,
                    &methodSize);
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
        // pICorProfilerInfoUnk : should provide ICorProfilerInfo2.
        // logger : tool to log the exceptions.
        CorProfilerInfo2(IUnknown* pICorProfilerInfoUnk, const TLogger logger)
            : CorProfilerInfo2(logger)
        {
            Init(pICorProfilerInfoUnk);
        }

        // Creates wrapper with logging and error handling capabilities.
        // Returns an empty optional in case of an error.
        // pICorProfilerInfoUnk : should provide ICorProfilerInfo2.
        // logger : tool to log the exceptions.
        static std::optional<CorProfilerInfo2<TLogger>> TryCreate(IUnknown* pICorProfilerInfoUnk, const TLogger logger)
        {
            if (CorProfilerInfo2<TLogger> result(logger)
                ; result.TryInit(pICorProfilerInfoUnk))
            {
                return result;
            }

            return std::nullopt;
        }

        // Calls ICorProfilerInfo2::GetFunctionInfo with the given functionId.
        // Throws _com_error in case of an error.
        FunctionInfo GetFunctionInfo(const FunctionID functionId) const
        {
            FunctionInfo result;
            this->CallComOrThrow(
                GetFunctionInfoCallable(functionId, result),
                L"Failed to call CorProfilerInfo2::GetFunctionInfo.");
            return result;
        }

        // Calls ICorProfilerInfo2::GetFunctionInfo with the given functionId.
        // Returns an empty optional in case of an error.
        std::optional<FunctionInfo> TryGetFunctionInfo(const FunctionID functionId) const
        {
            if (FunctionInfo result
                ; this->TryCallCom(
                    GetFunctionInfoCallable(functionId, result),
                    L"Failed to call CorProfilerInfo2::TryGetFunctionInfo."))
            {
                return result;
            }

            return std::nullopt;
        }

        // Calls ICorProfilerInfo2::GetModuleMetadata and creates
        // a MetadataImport2 wrapper around it.
        // The resulting object will become independent, and because
        // some logging context is required to create it, user of this
        // function must provide loggerForMetadata.
        // Throws _com_error in case of an error.
        template <typename TMetaDataLogger = TLogger>
        MetaDataImport2<TMetaDataLogger> GetModuleMetadata(
            const ModuleID moduleId,
            const TMetaDataLogger loggerForMetadata) const
        {
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport2{};
            CallComOrThrow(
                GetModuleMetadataCallable(moduleId, metaDataImport2),
                L"Failed to call CorProfilerInfo2::GetModuleMetadata.");
            return MetaDataImport2<TMetaDataLogger>(metaDataImport2, loggerForMetadata);
        }

        // Calls ICorProfilerInfo2::GetModuleMetadata and creates
        // a MetadataImport2 wrapper around it.
        // The resulting object will become independent, and because
        // some logging context is required to create it, user of this
        // function must provide loggerForMetadata.
        // Returns an empty optional in case of an error.
        template <typename TMetaDataLogger = TLogger>
        std::optional<MetaDataImport2<TMetaDataLogger>> TryGetModuleMetadata(
            const ModuleID moduleId,
            const TMetaDataLogger loggerForMetadata) const
        {
            if (ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport2{}
                ; TryCallCom(
                    GetModuleMetadataCallable(moduleId, metaDataImport2),
                    L"Failed to call CorProfilerInfo2::TryGetModuleMetadata."))
            {
                return MetaDataImport2<TMetaDataLogger>(metaDataImport2, loggerForMetadata);
            }

            return std::nullopt;
        }

        // Gets the name of the function specified by the FunctionID.
        // Throws _com_error in case of an error.
        std::wstring GetFunctionName(const FunctionID functionId) const
        {
            try
            {
                FunctionInfo functionInfo{ GetFunctionInfo(functionId) };
                return GetModuleMetadata(functionInfo.moduleId, m_logger)
                    .GetMethodName(functionInfo.token);
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"CorProfilerInfo2::GetFunctionName failed.";
                throw;
            }
        }

        // Calls ICorProfilerInfo2::GetILFunctionBody with the given FunctionInfo.
        // Returns vector with a copy of the bytes of the Intermediate Language
        // representation of the function body.
        // Throws _com_error in case of an error.
        std::vector<std::byte> GetMethodIntermediateLanguageBody(const FunctionInfo& functionInfo) const
        {
            LPCBYTE methodHeader;
            ULONG methodSize;
            this->CallComOrThrow(
                GetMethodIntermediateLanguageBodyCallable(
                    functionInfo,
                    methodHeader,
                    methodSize),
                L"Failed to call CorProfilerInfo2::GetMethodIntermediateLanguageBody.");

            return CopyBody(methodHeader, methodSize);
        }

        // Calls ICorProfilerInfo2::GetILFunctionBody with the given FunctionInfo.
        // Returns vector with a copy of the bytes of the Intermediate Language
        // representation of the function body.
        // Returns an empty optional in case of an error.
        std::optional<std::vector<std::byte>> TryGetMethodIntermediateLanguageBody(const FunctionInfo& functionInfo) const
        {
            LPCBYTE methodHeader;
            ULONG methodSize;
            if (this->TryCallCom(
                GetMethodIntermediateLanguageBodyCallable(
                    functionInfo,
                    methodHeader,
                    methodSize),
                L"Failed to call CorProfilerInfo2::TryGetMethodIntermediateLanguageBody."))
            {
                return CopyBody(methodHeader, methodSize);
            }

            return std::nullopt;
        }

        // Calls ICorProfilerInfo2::SetEventMask with the given mask.
        // Throws _com_error in case of an error.
        void SetEventMask(DWORD eventMask)
        {
            this->CallComOrThrow(SetEventMaskCallable(eventMask), L"Failed to call CorProfilerInfo2::SetEventMask.");
        }

        // Calls ICorProfilerInfo2::SetEventMask with the given mask.
        // Returns false in case of an error.
        bool TrySetEventMask(DWORD eventMask)
        {
            return this->TryCallCom(SetEventMaskCallable(eventMask) , L"Failed to call CorProfilerInfo2::TrySetEventMask.");
        }

        // Calls ICorProfilerInfo2::SetEnterLeaveFunctionHooks2 with the given parameters.
        // Throws _com_error in case of an error.
        void SetEnterLeaveFunctionHooks2(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall)
        {
            this->CallComOrThrow(
                SetEnterLeaveFunctionHooks2Callable(
                    pFuncEnter,
                    pFuncLeave,
                    pFuncTailcall),
                L"Failed to call CorProfilerInfo2::SetEnterLeaveFunctionHooks2.");
        }

        // Calls ICorProfilerInfo2::SetEnterLeaveFunctionHooks2 with the given parameters.
        // Returns false in case of an error.
        bool TrySetEnterLeaveFunctionHooks2(
            FunctionEnter2* pFuncEnter,
            FunctionLeave2* pFuncLeave,
            FunctionTailcall2* pFuncTailcall)
        {
            return this->TryCallCom(
                SetEnterLeaveFunctionHooks2Callable(
                    pFuncEnter,
                    pFuncLeave,
                    pFuncTailcall),
                L"Failed to call CorProfilerInfo2::TrySetEnterLeaveFunctionHooks2.");
        }
    };
}
