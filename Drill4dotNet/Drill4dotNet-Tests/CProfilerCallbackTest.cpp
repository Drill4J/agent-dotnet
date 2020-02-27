#include "pch.h"
#include "CoreInteractMock.h"
#include "ProClient.h"
#include <CProfilerCallback.h>

using namespace Drill4dotNet;
using namespace testing;

class CProfilerCallbackTest : public Test
{
public:

    void SetUp()
    {
        coreInteractMock = std::make_unique<CoreInteractMock>();
        proClient = std::make_unique<ProClient>();
        profilerCallback = std::make_unique<CProfilerCallback>(*proClient.get());
    }

    void TearDown()
    {
        profilerCallback.release();
        proClient.release();
        coreInteractMock.release();
    }

    std::unique_ptr<CoreInteractMock> coreInteractMock;
    std::unique_ptr<ProClient> proClient;
    std::unique_ptr<CProfilerCallback> profilerCallback;
};

namespace Drill4dotNet
{
    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> CreateCorProfilerInfo(TQueryInterface query, const TLogger logger)
    {
        // we pass ownership over the caller
        return std::move(reinterpret_cast<CProfilerCallbackTest*>(query)->coreInteractMock);
    }

    template
        std::unique_ptr<ICoreInteract> CreateCorProfilerInfo<IUnknown*, LogToProClient>(
            IUnknown* pICorProfilerInfoUnk,
            const LogToProClient logger);

    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> TryCreateCorProfilerInfo(TQueryInterface query, const TLogger logger)
    {
        // we pass ownership over the caller
        return std::move(reinterpret_cast<CProfilerCallbackTest*>(query)->coreInteractMock);
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

TEST_F(CProfilerCallbackTest, Initialize_Shutdown)
{
    // We have to pair each Initialize() by Shutdown() to properly clean up resources
    // ...until we change the design of ownership on CorProfilerInterface.
    const auto rti_expected = std::optional<RuntimeInformation>({ 0, COR_PRF_DESKTOP_CLR, 9, 9, 999, 8 });
    EXPECT_CALL(*coreInteractMock, TryGetRuntimeInformation()).WillOnce(Return(rti_expected));
    EXPECT_CALL(*coreInteractMock, SetEventMask(_)).WillOnce(Return());
    EXPECT_CALL(*coreInteractMock, SetEnterLeaveFunctionHooks(_,_,_)).WillOnce(Return());
    EXPECT_CALL(*coreInteractMock, SetFunctionIDMapper(_)).WillOnce(Return());

    IUnknown* p = reinterpret_cast<IUnknown*>(this);
    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Initialize(p));
    EXPECT_EQ(typeid(CoreInteractMock), typeid(profilerCallback->GetCorProfilerInfo()) );

    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Shutdown());
    EXPECT_EQ(nullptr, &profilerCallback->GetCorProfilerInfo());
}
