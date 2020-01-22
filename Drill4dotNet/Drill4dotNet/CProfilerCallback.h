#pragma once

#include <optional>
#include <type_traits>

#include "LogBuffer.h"
#include "CorProfilerInfo.h"

namespace Drill4dotNet
{
    class ProClient;

    // Wraps ProClient to make its logging interface
    // compatible with CorProfilerInfo.
    class LogToProClient
    {
    private:
        std::reference_wrapper<ProClient> m_proClient;

    public:

        explicit LogToProClient(ProClient& target) noexcept
            : m_proClient(target)
        {
        }

        constexpr bool IsLogEnabled() const noexcept
        {
            return true;
        }

        LogBuffer<std::wostream> Log() const;
    };

    class CProfilerCallback :
        public ICorProfilerCallback2
    {
    protected:
        ProClient& m_pImplClient;
        volatile ULONG m_lRef = 0;
        std::optional<CorProfilerInfo<LogToProClient>> m_corProfilerInfo{};
    public:
        CProfilerCallback(ProClient& client);
        ProClient& GetClient()
        {
            return m_pImplClient;
        }
        CorProfilerInfo<LogToProClient>& GetCorProfilerInfo()
        {
            return m_corProfilerInfo.value();
        }

        // Inherited via IUnknown
        virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
        virtual ULONG __stdcall AddRef(void) override;
        virtual ULONG __stdcall Release(void) override;

        // Inherited via ICorProfilerCallback
        virtual HRESULT __stdcall Initialize(IUnknown* pICorProfilerInfoUnk) override;
        virtual HRESULT __stdcall Shutdown(void) override;
        virtual HRESULT __stdcall AppDomainCreationStarted(AppDomainID appDomainId) override;
        virtual HRESULT __stdcall AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall AppDomainShutdownStarted(AppDomainID appDomainId) override;
        virtual HRESULT __stdcall AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall AssemblyLoadStarted(AssemblyID assemblyId) override;
        virtual HRESULT __stdcall AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall AssemblyUnloadStarted(AssemblyID assemblyId) override;
        virtual HRESULT __stdcall AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall ModuleLoadStarted(ModuleID moduleId) override;
        virtual HRESULT __stdcall ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall ModuleUnloadStarted(ModuleID moduleId) override;
        virtual HRESULT __stdcall ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId) override;
        virtual HRESULT __stdcall ClassLoadStarted(ClassID classId) override;
        virtual HRESULT __stdcall ClassLoadFinished(ClassID classId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall ClassUnloadStarted(ClassID classId) override;
        virtual HRESULT __stdcall ClassUnloadFinished(ClassID classId, HRESULT hrStatus) override;
        virtual HRESULT __stdcall FunctionUnloadStarted(FunctionID functionId) override;
        virtual HRESULT __stdcall JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock) override;
        virtual HRESULT __stdcall JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
        virtual HRESULT __stdcall JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction) override;
        virtual HRESULT __stdcall JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result) override;
        virtual HRESULT __stdcall JITFunctionPitched(FunctionID functionId) override;
        virtual HRESULT __stdcall JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline) override;
        virtual HRESULT __stdcall ThreadCreated(ThreadID threadId) override;
        virtual HRESULT __stdcall ThreadDestroyed(ThreadID threadId) override;
        virtual HRESULT __stdcall ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId) override;
        virtual HRESULT __stdcall RemotingClientInvocationStarted(void) override;
        virtual HRESULT __stdcall RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync) override;
        virtual HRESULT __stdcall RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync) override;
        virtual HRESULT __stdcall RemotingClientInvocationFinished(void) override;
        virtual HRESULT __stdcall RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync) override;
        virtual HRESULT __stdcall RemotingServerInvocationStarted(void) override;
        virtual HRESULT __stdcall RemotingServerInvocationReturned(void) override;
        virtual HRESULT __stdcall RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync) override;
        virtual HRESULT __stdcall UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
        virtual HRESULT __stdcall ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
        virtual HRESULT __stdcall RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason) override;
        virtual HRESULT __stdcall RuntimeSuspendFinished(void) override;
        virtual HRESULT __stdcall RuntimeSuspendAborted(void) override;
        virtual HRESULT __stdcall RuntimeResumeStarted(void) override;
        virtual HRESULT __stdcall RuntimeResumeFinished(void) override;
        virtual HRESULT __stdcall RuntimeThreadSuspended(ThreadID threadId) override;
        virtual HRESULT __stdcall RuntimeThreadResumed(ThreadID threadId) override;
        virtual HRESULT __stdcall MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
        virtual HRESULT __stdcall ObjectAllocated(ObjectID objectId, ClassID classId) override;
        virtual HRESULT __stdcall ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[]) override;
        virtual HRESULT __stdcall ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[]) override;
        virtual HRESULT __stdcall RootReferences(ULONG cRootRefs, ObjectID rootRefIds[]) override;
        virtual HRESULT __stdcall ExceptionThrown(ObjectID thrownObjectId) override;
        virtual HRESULT __stdcall ExceptionSearchFunctionEnter(FunctionID functionId) override;
        virtual HRESULT __stdcall ExceptionSearchFunctionLeave(void) override;
        virtual HRESULT __stdcall ExceptionSearchFilterEnter(FunctionID functionId) override;
        virtual HRESULT __stdcall ExceptionSearchFilterLeave(void) override;
        virtual HRESULT __stdcall ExceptionSearchCatcherFound(FunctionID functionId) override;
        virtual HRESULT __stdcall ExceptionOSHandlerEnter(UINT_PTR __unused) override;
        virtual HRESULT __stdcall ExceptionOSHandlerLeave(UINT_PTR __unused) override;
        virtual HRESULT __stdcall ExceptionUnwindFunctionEnter(FunctionID functionId) override;
        virtual HRESULT __stdcall ExceptionUnwindFunctionLeave(void) override;
        virtual HRESULT __stdcall ExceptionUnwindFinallyEnter(FunctionID functionId) override;
        virtual HRESULT __stdcall ExceptionUnwindFinallyLeave(void) override;
        virtual HRESULT __stdcall ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId) override;
        virtual HRESULT __stdcall ExceptionCatcherLeave(void) override;
        virtual HRESULT __stdcall COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots) override;
        virtual HRESULT __stdcall COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable) override;
        virtual HRESULT __stdcall ExceptionCLRCatcherFound(void) override;
        virtual HRESULT __stdcall ExceptionCLRCatcherExecute(void) override;

        // Inherited via ICorProfilerCallback2
        virtual HRESULT __stdcall ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[]) override;
        virtual HRESULT __stdcall GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason) override;
        virtual HRESULT __stdcall SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
        virtual HRESULT __stdcall GarbageCollectionFinished(void) override;
        virtual HRESULT __stdcall FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID) override;
        virtual HRESULT __stdcall RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[]) override;
        virtual HRESULT __stdcall HandleCreated(GCHandleID handleId, ObjectID initialObjectId) override;
        virtual HRESULT __stdcall HandleDestroyed(GCHandleID handleId) override;
    };
}
