#include "pch.h"
#include "CoreInteractMock.h"
#include "ConnectorMock.h"
#include "ProClient.h"
#include <CProfilerCallback.h>

using namespace Drill4dotNet;
using namespace testing;

class CProfilerCallbackTest : public Test
{
public:

    void SetUp()
    {
        connectorMock = std::make_shared<ConnectorMock>();
        EXPECT_CALL(*connectorMock, InitializeAgent).WillOnce(Return());
        coreInteractMock = std::make_unique<CoreInteractMock>();
        proClient = std::make_unique<ProClient>(connectorMock);
        profilerCallback = std::make_unique<CProfilerCallback>(*proClient.get());
        WCHAR injectionFileName[_MAX_PATH];
        ::GetModuleFileName(NULL, injectionFileName, _MAX_PATH);
        Drill4dotNet::s_Drill4dotNetLibFilePath = injectionFileName;
    }

    void TearDown()
    {
        profilerCallback.release();
        proClient.release();
        coreInteractMock.release();
        // Dirty hack to workaround leaked mock objects GTest bug when using shared_ptr
        // Refer to https://github.com/google/googletest/issues/1310
        connectorMock.~shared_ptr();
        connectorMock.reset();
    }

    std::unique_ptr<CoreInteractMock> coreInteractMock;
    std::shared_ptr<ConnectorMock> connectorMock;
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

    std::shared_ptr<IConnector> IConnector::CreateInstance()
    {
        return std::make_shared<ConnectorMock>();
    }
}

TEST_F(CProfilerCallbackTest, GetClient)
{
    EXPECT_EQ(proClient.get(), &(profilerCallback->GetClient()));
}

TEST_F(CProfilerCallbackTest, Client_GetConnector)
{
    EXPECT_EQ(connectorMock.get(), &(proClient->GetConnector()));
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

    EXPECT_CALL(*connectorMock, SendMessage1("ready")).WillOnce(Return());

    IUnknown* p = reinterpret_cast<IUnknown*>(this);
    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Initialize(p));
    EXPECT_EQ(typeid(CoreInteractMock), typeid(profilerCallback->GetCorProfilerInfo()) );

    const InjectionMetaData emptyInjection;
    const InjectionMetaData actualInjection = profilerCallback->GetInfoHandler().GetInjectionMetaData();
    EXPECT_NE(emptyInjection.Assembly, actualInjection.Assembly);
    EXPECT_NE(emptyInjection.Class, actualInjection.Class);
    EXPECT_NE(emptyInjection.Function, actualInjection.Function);

    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Shutdown());
    EXPECT_EQ(nullptr, &profilerCallback->GetCorProfilerInfo());
}
