#include "pch.h"
#include "ComWrapperBase.h"
#include "CoreInteractMock.h"
#include "MetaDataAssemblyImportMock.h"
#include "MetaDataDispenserMock.h"
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
        proClient.emplace();
        profilerCallback.emplace(*proClient);
        WCHAR injectionFileName[_MAX_PATH];
        ::GetModuleFileName(NULL, injectionFileName, _MAX_PATH);
        Drill4dotNet::s_Drill4dotNetLibFilePath = injectionFileName;
    }

    void TearDown()
    {
        profilerCallback.reset();
        proClient.reset();
    }

    std::optional<ProClient<ConnectorMock>> proClient;
    std::optional<CProfilerCallback<
        ConnectorMock,
        CoreInteractMock,
        MetaDataDispenserMock,
        MetaDataAssemblyImportMock,
        MetadataImportMock,
        TrivialLogger>> profilerCallback;
};

TEST_F(CProfilerCallbackTest, GetClient)
{
    EXPECT_EQ(&*proClient, &profilerCallback->GetClient());
}

TEST_F(CProfilerCallbackTest, Initialize_Shutdown)
{
    // We have to pair each Initialize() by Shutdown() to properly clean up resources
    // ...until we change the design of ownership on CorProfilerInterface.
    const auto rti_expected = std::optional<RuntimeInformation>({ 0, COR_PRF_DESKTOP_CLR, 9, 9, 999, 8 });
    const auto coreInteractCreation { CoreInteractMock::SetCreationCallBack([rti_expected](
        CoreInteractMock& corInteractMock,
        IUnknown* query,
        TrivialLogger logger)
    {
        EXPECT_CALL(corInteractMock, TryGetRuntimeInformation()).WillOnce(Return(rti_expected));
        EXPECT_CALL(corInteractMock, SetEventMask(_)).WillOnce(Return());
        EXPECT_CALL(corInteractMock, SetEnterLeaveFunctionHooks(_,_,_)).WillOnce(Return());
        EXPECT_CALL(corInteractMock, SetFunctionIDMapper(_)).WillOnce(Return());
    }) };

    const InjectionMetaData expectedInjection { 0x69'6D'71'EF, 0x9A'F0'D6'D8, 0xE9'C2'80'22 };
    const std::wstring injectionAssemblyName { L"Injection" };
    const auto metaDataAssemblyImportCreation { MetaDataAssemblyImportMock::SetOnCreate([expectedInjection, injectionAssemblyName](MetaDataAssemblyImportMock& metaDataAssemblyImportMock)
    {
        EXPECT_CALL(metaDataAssemblyImportMock, GetAssemblyFromScope()).WillOnce(Return(expectedInjection.Assembly));
        EXPECT_CALL(metaDataAssemblyImportMock, GetAssemblyProps(expectedInjection.Assembly)).WillOnce(Return(AssemblyProps { injectionAssemblyName, 0 }));
    }) };

    const std::wstring injectionClassName { L"Drill4dotNet.CInjection" };
    const std::wstring injectionMethodName { L"FInjection" };
    const auto metaDataImportCreation { MetadataImportMock::SetOnCreate([expectedInjection, injectionClassName, injectionMethodName](MetadataImportMock& metaDataImportMock)
    {
        EXPECT_CALL(metaDataImportMock, FindTypeDefByName(injectionClassName, 0)).WillOnce(Return(expectedInjection.Class));
        EXPECT_CALL(metaDataImportMock, GetTypeDefProps(expectedInjection.Class)).WillOnce(Return(TypeDefProps { injectionClassName, 0, 0 }));
        EXPECT_CALL(metaDataImportMock, EnumMethodsWithName(expectedInjection.Class, injectionMethodName)).WillOnce(Return(std::vector { expectedInjection.Function }));
    }) };

    EXPECT_CALL(proClient->GetConnector(), SendMessage1("ready")).WillOnce(Return());

    IUnknown* p = reinterpret_cast<IUnknown*>(this);
    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Initialize(p));
    EXPECT_EQ(typeid(std::optional<CoreInteractMock>), typeid(profilerCallback->GetCorProfilerInfo()) );

    const InjectionMetaData actualInjection = profilerCallback->GetInfoHandler().GetInjectionMetaData();
    EXPECT_EQ(expectedInjection.Assembly, actualInjection.Assembly);
    EXPECT_EQ(expectedInjection.Class, actualInjection.Class);
    EXPECT_EQ(expectedInjection.Function, actualInjection.Function);

    EXPECT_HRESULT_SUCCEEDED(profilerCallback->Shutdown());
    EXPECT_FALSE(profilerCallback->GetCorProfilerInfo().has_value());
}
