#pragma once

#include "framework.h"
#include "OutputUtils.h"
#include "FunctionInfo.h"
#include "ComWrapperBase.h"

namespace Drill4dotNet
{
    // Provides logging and error handling capabilities for IMetaDataImport2.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename TLogger>
    class MetaDataImport2 : protected ComWrapperBase<TLogger>
    {
    private:
        ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> m_metaDataImport2{};

    public:
        // Captures IID_IMetaDataImport2 object allowing safe access to its methods.
        // metaDataImport2: the IID_IMetaDataImport2 object, which allows accessing
        //     information on classes and methods
        // logger: tool to log the exceptions.
        MetaDataImport2(
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport2,
            TLogger logger)
            : ComWrapperBase(logger),
            m_metaDataImport2(metaDataImport2)
        {
        }

        // Gets the name of the method with IID_IMetaDataImport2::GetMethodProps.
        // Throws _com_error in case of error.
        // methodMetadataToken : points to the method
        std::wstring GetMethodName(const mdToken methodMetadataToken) const
        {
            try
            {
                ULONG actualLength;
                CallComOrThrow([this, methodMetadataToken, &actualLength]()
                {
                    return m_metaDataImport2->GetMethodProps(
                        methodMetadataToken,
                        nullptr,
                        nullptr,
                        0,
                        &actualLength,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr);
                }, L"MetadataImport2::GetMethodName: Failed to retrieve the length of the method name");

                if (actualLength == 0)
                {
                    return std::wstring{};
                }

                std::wstring result(actualLength, L'\0');
                CallComOrThrow([this, methodMetadataToken, &result, actualLength]()
                {
                    ULONG dummy;
                    return m_metaDataImport2->GetMethodProps(
                        methodMetadataToken,
                        nullptr,
                        result.data(),
                        actualLength,
                        &dummy,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr);
                }, L"MetadataImport2::GetMethodName: Failed to retrieve the characters of the method name");

                TrimTrailingNull(result);
                return result;
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"MetadataImport2::GetMethodName failed";
                throw;
            }
        }
    };
}
