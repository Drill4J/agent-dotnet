#pragma once


#include "framework.h"
#include "OutputUtils.h"
#include "ComWrapperBase.h"

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
