#include "pch.h"
#include "CoreInteractMock.h"
#include "ProClient.h"
#include <CProfilerCallback.h>

using namespace Drill4dotNet;
using namespace testing;

class CProfilerCallbackTest : public Test
{
public:
    CProfilerCallbackTest()
    {
    }

    void SetUp()
    {
        profilerCallback = new CProfilerCallback(*proClient);
    }

    void TearDown()
    {
        delete profilerCallback;
    }

    ~CProfilerCallbackTest()
    {
        delete proClient;
    }

    ProClient * proClient = new ProClient;
    CProfilerCallback * profilerCallback = nullptr;
};

namespace Drill4dotNet
{
    static CoreInteractMock* coreInteractMock(new CoreInteractMock);

    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> CreateCorProfilerInfo(TQueryInterface query, const TLogger logger)
    {
        return std::unique_ptr<ICoreInteract>(coreInteractMock);
    }

    template
        std::unique_ptr<ICoreInteract> CreateCorProfilerInfo<IUnknown*, LogToProClient>(
            IUnknown* pICorProfilerInfoUnk,
            const LogToProClient logger);

    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> TryCreateCorProfilerInfo(TQueryInterface query, const TLogger logger)
    {
        return std::unique_ptr<ICoreInteract>(coreInteractMock);
    }

    template
        std::unique_ptr<ICoreInteract> TryCreateCorProfilerInfo<IUnknown*, LogToProClient>(
            IUnknown* pICorProfilerInfoUnk,
            const LogToProClient logger);
}

TEST_F(CProfilerCallbackTest, GetClient)
{
    ProClient _proClient;
    CProfilerCallback _obj(_proClient);
    EXPECT_EQ(&_proClient, &(_obj.GetClient()));
}

TEST_F(CProfilerCallbackTest, Initialize)
{
    const auto rti_expected = std::optional<RuntimeInformation>({ 0, COR_PRF_DESKTOP_CLR, 9, 9, 999, 8 });
    EXPECT_CALL(*coreInteractMock, TryGetRuntimeInformation()).WillOnce(Return(rti_expected));
    EXPECT_CALL(*coreInteractMock, SetEventMask(_)).WillOnce(Return());
    EXPECT_CALL(*coreInteractMock, SetEnterLeaveFunctionHooks(_,_,_)).WillOnce(Return());
    EXPECT_CALL(*coreInteractMock, SetFunctionIDMapper(_)).WillOnce(Return());

    IUnknown* p = (IUnknown *)(~(NULL));
    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Initialize(p));
    EXPECT_EQ(typeid(CoreInteractMock), typeid(profilerCallback->GetCorProfilerInfo()) );
    EXPECT_EQ(coreInteractMock, &profilerCallback->GetCorProfilerInfo());
}

TEST_F(CProfilerCallbackTest, Shutdown)
{
    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Shutdown());
    EXPECT_EQ(NULL, &profilerCallback->GetCorProfilerInfo());
}

// Attach definitions of IID_ICorProfilerCallback, IID_ICorProfilerCallback2, IID_ICorProfilerInfo2, IID_ICorProfilerInfo3, etc.
#include <corprof_i.cpp>
