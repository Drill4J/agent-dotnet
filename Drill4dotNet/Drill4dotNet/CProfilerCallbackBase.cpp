#include "pch.h"

#include "CProfilerCallbackBase.h"

namespace Drill4dotNet
{
    //
    // Inherited via IUnknown (for isolated testing)
    //
    HRESULT __stdcall CProfilerCallbackBase::QueryInterface(REFIID riid, void** ppvObject)
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

    ULONG __stdcall CProfilerCallbackBase::AddRef(void)
    {
        return InterlockedIncrement(&m_lRef);
    }

    ULONG __stdcall CProfilerCallbackBase::Release(void)
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
    HRESULT __stdcall CProfilerCallbackBase::Initialize(IUnknown* pICorProfilerInfoUnk)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::Shutdown(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AppDomainCreationStarted(AppDomainID appDomainId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AppDomainShutdownStarted(AppDomainID appDomainId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AssemblyLoadStarted(AssemblyID assemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AssemblyUnloadStarted(AssemblyID assemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ModuleLoadStarted(ModuleID moduleId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ModuleUnloadStarted(ModuleID moduleId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ClassLoadStarted(ClassID classId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ClassUnloadStarted(ClassID classId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::FunctionUnloadStarted(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::JITFunctionPitched(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ThreadCreated(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ThreadDestroyed(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingClientInvocationStarted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingClientInvocationFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingServerInvocationStarted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingServerInvocationReturned(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeSuspendFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeSuspendAborted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeResumeStarted(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeResumeFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeThreadSuspended(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RuntimeThreadResumed(ThreadID threadId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ObjectAllocated(ObjectID objectId, ClassID classId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionThrown(ObjectID thrownObjectId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionSearchFunctionEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionSearchFunctionLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionSearchFilterEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionSearchFilterLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionSearchCatcherFound(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionOSHandlerEnter(UINT_PTR __unused)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionOSHandlerLeave(UINT_PTR __unused)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionUnwindFunctionEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionUnwindFunctionLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionUnwindFinallyEnter(FunctionID functionId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionUnwindFinallyLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionCatcherLeave(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionCLRCatcherFound(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::ExceptionCLRCatcherExecute(void)
    {
        return E_NOTIMPL;
    }

    //
    // Inherited via ICorProfilerCallback2
    //
    HRESULT __stdcall CProfilerCallbackBase::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::GarbageCollectionFinished(void)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
    {
        return E_NOTIMPL;
    }

    HRESULT __stdcall CProfilerCallbackBase::HandleDestroyed(GCHandleID handleId)
    {
        return E_NOTIMPL;
    }
}
