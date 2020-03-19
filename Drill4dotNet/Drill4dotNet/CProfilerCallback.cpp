#include "pch.h"
#include "OutputUtils.h"
#include "CProfilerCallback.h"
#include "CorDataStructures.h"
#include "ProClient.h"
#include "InfoHandler.h"
#include "OpCodes.h"
#include "MethodBody.h"
#include <comdef.h>

namespace Drill4dotNet
{

    constexpr DWORD EVENTS_WE_MONITOR =
        COR_PRF_MONITOR_CLASS_LOADS |
        COR_PRF_MONITOR_MODULE_LOADS |
        COR_PRF_MONITOR_ASSEMBLY_LOADS |
        COR_PRF_MONITOR_APPDOMAIN_LOADS |
        COR_PRF_MONITOR_JIT_COMPILATION |
        COR_PRF_MONITOR_ENTERLEAVE;

    CProfilerCallback::CProfilerCallback(ProClient& client)
        : m_pImplClient(client)
    {
    }

    ProClient& CProfilerCallback::GetClient()
    {
        return  m_pImplClient;
    }

    InfoHandler& CProfilerCallback::GetInfoHandler()
    {
        return m_pImplClient.GetInfoHandler();
    }

    ICoreInteract& CProfilerCallback::GetCorProfilerInfo()
    {
        return *(m_corProfilerInfo.get());
    }

    namespace
    {

        static CProfilerCallback* g_cb = nullptr;

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
                g_cb->GetClient().Log() << L"Enter function: " << functionInfo->fullName();
            }
            else
            {
                g_cb->GetClient().Log() << L"Enter function: " << funcId;
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
                g_cb->GetClient().Log() << L"Leave function: " << functionInfo->fullName();
            }
            else
            {
                g_cb->GetClient().Log() << L"Leave function: " << funcId;
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
                g_cb->GetClient().Log() << L"Tailcall at function: " << functionInfo->fullName();
            }
            else
            {
                g_cb->GetClient().Log() << L"Tailcall at function: " << funcId;
            }
        }

        static UINT_PTR __stdcall fn_FunctionIDMapper(
            FunctionID funcId,
            BOOL* pbHookFunction)
        {
            if (!g_cb) return funcId;

            if (auto functionInfo = g_cb->GetCorProfilerInfo().TryGetFunctionInfo(funcId);
                functionInfo.has_value())
            {
                g_cb->GetClient().Log() << "Mapping   function[" << funcId << "] to " << functionInfo->fullName();
                g_cb->GetInfoHandler().MapFunctionInfo(funcId, functionInfo.value());
            }

            if (pbHookFunction)
            {
                *pbHookFunction = TRUE; // to receive FunctionEnter2, FunctionLeave2, and FunctionTailcall2 callbacks
            }
            return funcId;
        }
    } // anonymous namespace

    //
    // Inherited via IUnknown (for isolated testing)
    //
    HRESULT __stdcall CProfilerCallback::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == IID_IUnknown ||
            riid == IID_ICorProfilerCallback ||
            riid == IID_ICorProfilerCallback2)
        {
            *ppvObject = (ICorProfilerCallback2*)this;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    ULONG __stdcall CProfilerCallback::AddRef(void)
    {
        return InterlockedIncrement(&m_lRef);
    }

    ULONG __stdcall CProfilerCallback::Release(void)
    {
        InterlockedDecrement(&m_lRef);
        if (0 == m_lRef)
        {
            delete this;
            return 0;
        }
        return m_lRef;
    }

    //
    // Inherited via ICorProfilerCallback
    //
    HRESULT __stdcall CProfilerCallback::Initialize(IUnknown* pICorProfilerInfoUnk)
    {
        m_pImplClient.Log() << L"CProfilerCallback::Initialize";
        try
        {
            m_corProfilerInfo = CreateCorProfilerInfo(pICorProfilerInfoUnk, LogToProClient(m_pImplClient));

            if (auto runtimeInformation = m_corProfilerInfo->TryGetRuntimeInformation();
                runtimeInformation)
            {
                m_pImplClient.Log() 
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
        }
        catch (const _com_error& exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
            return errorCode;
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::Shutdown(void)
    {
        m_pImplClient.Log() << L"CProfilerCallback::Shutdown";
        try
        {
            g_cb = nullptr;
            GetInfoHandler().OutputStatistics();
            m_corProfilerInfo.reset();
        }
        catch (const _com_error & exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
            return errorCode;
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainCreationStarted(AppDomainID appDomainId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AppDomainCreationStarted(" << appDomainId << ")";
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AppDomainCreationFinished(" << appDomainId << "), with status: " << HexOutput(hrStatus);
        try
        {
            // valid when the Finished event is called
            std::optional<AppDomainInfo> info = m_corProfilerInfo->TryGetAppDomainInfo(appDomainId);
            if (info)
            {
                m_pImplClient.Log() << L"Domain name: " << info->name << L", process id: " << info->processId;
                GetInfoHandler().MapAppDomainInfo(appDomainId, info.value());
            }
        }
        catch (const _com_error & exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainShutdownStarted(AppDomainID appDomainId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AppDomainShutdownStarted(" << appDomainId << ")";
        try
        {
            GetInfoHandler().OutputAppDomainInfo(appDomainId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AppDomainShutdownFinished(" << appDomainId << "), with status: " << HexOutput(hrStatus);
        try
        {
            GetInfoHandler().OutputAppDomainInfo(appDomainId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyLoadStarted(AssemblyID assemblyId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AssemblyLoadStarted(" << assemblyId << ")";
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AssemblyLoadFinished(" << assemblyId << "), with status: " << HexOutput(hrStatus);
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
        catch (const _com_error & exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyUnloadStarted(AssemblyID assemblyId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AssemblyUnloadStarted(" << assemblyId << ")";
        try
        {
            GetInfoHandler().OutputAssemblyInfo(assemblyId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::AssemblyUnloadFinished(" << assemblyId << "), with status: " << HexOutput(hrStatus);
        try
        {
            GetInfoHandler().OutputAssemblyInfo(assemblyId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ModuleLoadStarted(ModuleID moduleId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ModuleLoadStarted(" << moduleId << ")";
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ModuleLoadFinished(" << moduleId << ")";
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
        catch (const _com_error & exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ModuleUnloadStarted(ModuleID moduleId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ModuleUnloadStarted(" << moduleId << ")";
        try
        {
            GetInfoHandler().OutputModuleInfo(moduleId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ModuleUnloadFinished(" << moduleId << ")";
        try
        {
            GetInfoHandler().OutputModuleInfo(moduleId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ClassLoadStarted(ClassID classId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ClassLoadStarted(" << classId << ")";
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ClassLoadFinished(" << classId << ")";
        try
        {
            // valid when the Finished event is called
            auto info = m_corProfilerInfo->TryGetClassInfo(classId);
            if (info)
            {
                GetInfoHandler().MapClassInfo(classId, info.value());
                GetInfoHandler().OutputClassInfo(classId);
            }
        }
        catch (const _com_error & exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"COM error: " << HexOutput(errorCode) << " " << exception.ErrorMessage();
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ClassUnloadStarted(ClassID classId)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ClassUnloadStarted(" << classId << ")";
        try
        {
            GetInfoHandler().OutputClassInfo(classId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
    {
        m_pImplClient.Log() << L"CProfilerCallback::ClassUnloadFinished(" << classId << ")";
        try
        {
            GetInfoHandler().OutputClassInfo(classId);
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::FunctionUnloadStarted(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
    {
        GetClient().Log() << L"CProfilerCallback::JITCompilationStarted";
        try
        {
            // This example injection will insert artificial calls to Console.WriteLine
            // into the HelloWorld.Program.MyInjectionTarget function, at the place of
            // the second Console.WriteLine call, and in the finally clause. Also some
            // NOPs will be added to show ability to turn short branching instructions
            // into long ones.

            const FunctionInfo functionInfo{
                m_corProfilerInfo->GetFunctionInfo(functionId) };

            const std::vector<std::byte> functionBytes{
                m_corProfilerInfo->GetMethodIntermediateLanguageBody(functionInfo) };

            GetClient().Log()
                << L"Compiling function "
                << InSquareBrackets(functionId)
                << L" "
                << functionInfo.fullName()
                << L" IL Body size: "
                << functionBytes.size()
                << L" bytes";

            if (functionInfo.fullName() != L"HelloWorld.Program.MyInjectionTarget")
            {
                return S_OK;
            }

            auto functionBody = MethodBody(functionBytes);

            GetClient().Log()
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
                GetClient().Log() << L"Compiling without any injection is OK.";
            }
            else
            {
                GetClient().Log() << L"Error: got different bytes when compiled method without any injection.";
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
                OpCode::CEE_BLT_S { ShortJump { label } });

            functionBody.Insert(
                findSecondCall() + 4,
                OpCode::CEE_LDC_I4 { 42 });

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
            GetClient().Log()
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
            GetClient().Log() << L"Injected successfully.";

            return S_OK;
        }
        catch (const _com_error& exception)
        {
            m_pImplClient.Log()
                << L"COM error: "
                << HexOutput(exception.Error())
                << " "
                << exception.ErrorMessage();

            return exception.Error();
        }
        catch (const std::exception & exception)
        {
            m_pImplClient.Log() << L"Std exception: " << exception.what();
        }
        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::JITFunctionPitched(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ThreadCreated(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ThreadDestroyed(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingClientInvocationStarted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingClientInvocationFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingServerInvocationStarted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingServerInvocationReturned(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeSuspendFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeSuspendAborted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeResumeStarted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeResumeFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeThreadSuspended(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RuntimeThreadResumed(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ObjectAllocated(ObjectID objectId, ClassID classId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionThrown(ObjectID thrownObjectId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionSearchFunctionEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionSearchFunctionLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionSearchFilterEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionSearchFilterLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionSearchCatcherFound(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionOSHandlerEnter(UINT_PTR __unused)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionOSHandlerLeave(UINT_PTR __unused)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionUnwindFunctionEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionUnwindFunctionLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionUnwindFinallyEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionUnwindFinallyLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionCatcherLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionCLRCatcherFound(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ExceptionCLRCatcherExecute(void)
    {
        return E_NOTIMPL;
    }

    //
    // Inherited via ICorProfilerCallback2
    //
    HRESULT __stdcall CProfilerCallback::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::GarbageCollectionFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::HandleDestroyed(GCHandleID handleId)
    {
        return E_NOTIMPL;
    }

    LogBuffer<std::wostream> LogToProClient::Log() const
    {
        return m_proClient.get().Log();
    }
}
