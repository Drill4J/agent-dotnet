#pragma once

#include "framework.h"
#include "OutputUtils.h"
#include "CorDataStructures.h"
#include "ComWrapperBase.h"

namespace Drill4dotNet
{
    // Provides logging and error handling capabilities for IMetaDataImport2.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename TLogger>
    class MetaDataImport : protected ComWrapperBase<TLogger>
    {
    private:
        ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> m_metaDataImport{};

    public:
        // Captures IID_IMetaDataImport2 object allowing safe access to its methods.
        // metaDataImport: the IID_IMetaDataImport2 object, which allows accessing
        //     information on classes and methods
        // logger: tool to log the exceptions.
        MetaDataImport(
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport,
            TLogger logger)
            : ComWrapperBase(logger),
            m_metaDataImport(metaDataImport)
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
                    return m_metaDataImport->GetMethodProps(
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
                }, L"IMetadataImport2::GetMethodName, 1-st call: Failed to retrieve the length of the method name");

                if (actualLength == 0)
                {
                    return std::wstring{};
                }

                std::wstring result(actualLength, L'\0');
                CallComOrThrow([this, methodMetadataToken, &result, actualLength]()
                {
                    ULONG dummy;
                    return m_metaDataImport->GetMethodProps(
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
                }, L"IMetadataImport2::GetMethodName, 2-nd call: Failed to retrieve the characters of the method name");

                TrimTrailingNull(result);
                return result;
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"MetadataImport::GetMethodName failed";
                throw;
            }
        }

        /// Gets the name of the method with IID_IMetaDataImport2::GetMethodProps.
        /// @returns method's name or std::nullopt in case of error.
        /// @param methodMetadataToken : points to the method
        std::optional<std::wstring> TryGetMethodName(const mdToken methodMetadataToken) const
        {
            if (ULONG actualLength;
                TryCallCom(
                [this, methodMetadataToken, &actualLength]()
                {
                    return m_metaDataImport->GetMethodProps(
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
                },
                L"IMetadataImport2::GetMethodName, 1-st call: Failed to retrieve the length of the method name"))
            {

                if (actualLength == 0)
                {
                    return std::wstring{};
                }

                if (std::wstring result(actualLength, L'\0');
                    TryCallCom(
                    [this, methodMetadataToken, &result, actualLength]()
                    {
                        ULONG dummy;
                        return m_metaDataImport->GetMethodProps(
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
                    },
                    L"IMetadataImport2::GetMethodName, 2-nd call: Failed to retrieve the characters of the method name"))
                {

                    TrimTrailingNull(result);
                    return result;
                }
            }
            m_logger.Log() << L"MetadataImport::TryGetMethodName failed";
            return std::nullopt;
        }

        /// Gets the name of the given class/type with IMetaDataImport2::GetTypeDefProps.
        /// @param typeDefToken : token of the class or type
        /// @returns : name of the class/type or std::nullopt in case of error.
        std::optional<std::wstring> TryGetTypeName(const mdTypeDef typeDefToken) const
        {
            if (ULONG requiredLength;
                TryCallCom(
                [this, typeDefToken, &requiredLength]()
                {
                    return m_metaDataImport->GetTypeDefProps(
                        typeDefToken,
                        nullptr,
                        0,
                        &requiredLength,
                        nullptr,
                        nullptr);
                },
                L"Calling IMetadataImport2::GetTypeDefProps, 1-st try."))
            {
                if (0 == requiredLength)
                {
                    return std::wstring{};
                }
                if (std::wstring result(requiredLength, L'\0');
                    TryCallCom(
                    [this, typeDefToken, requiredLength, &result]()
                    {
                        ULONG cchDummy;
                        return m_metaDataImport->GetTypeDefProps(
                            typeDefToken,
                            result.data(),
                            requiredLength,
                            &cchDummy,
                            nullptr,
                            nullptr);
                    },
                    L"Calling IMetadataImport2::GetTypeDefProps, 2-nd try."))
                {
                    TrimTrailingNull(result);
                    return result;
                }
            }
            return std::nullopt;
        }
    };
}
