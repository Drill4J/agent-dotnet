#include "pch.h"

#include "OutputUtils.h"
#include "CProfilerCallback.h"
#include "ProClient.h"

namespace Drill4dotNet
{

    CProfilerCallback::CProfilerCallback(ProClient& client)
        : m_pImplClient(client)
    {
    }

    ProClient& CProfilerCallback::GetClient()
    {
        return  m_pImplClient;
    }

    namespace
    {

        static CProfilerCallback* g_cb = nullptr;

        static void __stdcall fn_functionEnter2(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO func, COR_PRF_FUNCTION_ARGUMENT_INFO* argumentInfo)
        {
            if (!g_cb) return;

            g_cb->GetClient().Log() << L"Enter function: " << HexOutput(funcId);
        }

        static void __stdcall fn_functionLeave2(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO func, COR_PRF_FUNCTION_ARGUMENT_RANGE* retvalRange)
        {
            if (!g_cb) return;

            g_cb->GetClient().Log() << L"Leave function: " << HexOutput(funcId);
        }

        static void __stdcall fn_functionTailcall2(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO func)
        {
            if (!g_cb) return;

            g_cb->GetClient().Log() << L"Tailcall at function: " << HexOutput(funcId);
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
            m_corProfilerInfo2 = CorProfilerInfo2(pICorProfilerInfoUnk, LogToProClient(m_pImplClient));

            DWORD eventMask = (DWORD)(COR_PRF_MONITOR_ENTERLEAVE | COR_PRF_MONITOR_JIT_COMPILATION);
            m_corProfilerInfo2->SetEventMask(eventMask);

            // set the enter, leave and tailcall hooks
            g_cb = this;
            m_corProfilerInfo2->SetEnterLeaveFunctionHooks2(fn_functionEnter2, fn_functionLeave2, fn_functionTailcall2);
        }
        catch (const _com_error& exception)
        {
            HRESULT errorCode = exception.Error();
            m_pImplClient.Log() << L"CProfilerCallback::Initialize failed: " << HexOutput(errorCode);
            return errorCode;
        }

        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::Shutdown(void)
    {
        m_pImplClient.Log() << L"CProfilerCallback::Shutdown";
        g_cb = nullptr;

        return S_OK;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainCreationStarted(AppDomainID appDomainId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainShutdownStarted(AppDomainID appDomainId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyLoadStarted(AssemblyID assemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyUnloadStarted(AssemblyID assemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ModuleLoadStarted(ModuleID moduleId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ModuleUnloadStarted(ModuleID moduleId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ClassLoadStarted(ClassID classId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ClassUnloadStarted(ClassID classId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::FunctionUnloadStarted(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallback::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
    {
        try
        {
            std::wstring functionName{ m_corProfilerInfo2->GetFunctionName(functionId) };

            std::vector<std::byte> functionBody{
                m_corProfilerInfo2->GetMethodIntermediateLanguageBody(
                    m_corProfilerInfo2->GetFunctionInfo(functionId)) };

            GetClient().Log()
                << L"Compiling function "
                << HexOutput(functionId)
                << L" "
                << functionName
                << L" IL Body size: "
                << functionBody.size()
                << L" bytes";

            return S_OK;
        }
        catch (const _com_error & exception)
        {
            GetClient().Log() << L"CProfilerCallback::JITCompilationStarted failed.";
            return exception.Error();
        }
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
