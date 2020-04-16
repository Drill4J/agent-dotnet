#pragma once

#include <memory>

#include "framework.h"
#include "OutputUtils.h"
#include "CorDataStructures.h"
#include "ComWrapperBase.h"
#include "IMetadataImport.h"

namespace Drill4dotNet
{
    // Provides logging and error handling capabilities for IMetaDataImport2.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <Logger TLogger>
    class MetaDataImport : protected ComWrapperBase<TLogger>
    {
    private:
        ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> m_metaDataImport{};

        // Wraps IMetaDataImport::FindTypeDefByName
        auto FindTypeDefByNameCallable(
            const std::wstring& name,
            const mdToken enclosingClass,
            mdTypeDef& result) const
        {
            return [this, name, enclosingClass, &result]()
            {
                return m_metaDataImport->FindTypeDefByName(
                    name.c_str(),
                    enclosingClass,
                    &result);
            };
        }

        class MetaDataEnum
        {
        private:
            const MetaDataImport& m_self;
        public:
            HCORENUM Value { nullptr };

            MetaDataEnum(const MetaDataImport& self)
                : m_self { self }
            {
            }

            ~MetaDataEnum() noexcept
            {
                if (Value != nullptr)
                {
                    m_self.m_metaDataImport->CloseEnum(Value);
                }
            }

            MetaDataEnum(const MetaDataEnum& other) = delete;
            MetaDataEnum& operator=(const MetaDataEnum& other) & = delete;

            MetaDataEnum(MetaDataEnum&& other)
                : Value { std::exchange(other.Value, nullptr) }
            {
            }

            MetaDataEnum& operator=(MetaDataEnum&& other) &
            {
                if (Value != nullptr)
                {
                    m_self.m_metaDataImport->CloseEnum(Value);
                }

                Value = std::exchange(other.Value, nullptr);
                return *this;
            }
        };

        // Wraps IMetaDataImport::EnumMethodsWithName.
        auto EnumMethodsWithNameCallable(
            const mdTypeDef enclosingType,
            const std::wstring& name,
            mdMethodDef* methods,
            const ULONG methodsLength,
            ULONG& methodsRetrieved,
            HCORENUM& enumeration,
            HRESULT& result
        ) const
        {
            return [
                this,
                enclosingType,
                name,
                methods,
                methodsLength,
                &methodsRetrieved,
                &enumeration,
                &result]()
            {
                return result = m_metaDataImport->EnumMethodsWithName(
                    &enumeration,
                    enclosingType,
                    name.c_str(),
                    methods,
                    methodsLength,
                    &methodsRetrieved);
            };
        };

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
        MethodProps GetMethodProps(const mdMethodDef methodMetadataToken) const
        {
            try
            {
                MethodProps result;
                ULONG actualLength;
                CallComOrThrow([this, methodMetadataToken, &result, &actualLength]()
                {
                    return m_metaDataImport->GetMethodProps(
                        methodMetadataToken,
                        &result.EnclosingClass,
                        nullptr,
                        0,
                        &actualLength,
                        &result.Attributes,
                        nullptr,
                        nullptr,
                        &result.CodeRelativeVirtualAddress,
                        &result.ImplementationFlags);
                }, L"IMetadataImport2::GetMethodName, 1-st call: Failed to retrieve the length of the method name");

                if (actualLength == 0)
                {
                    return result;
                }

                result.Name = std::wstring(actualLength, L'\0');
                CallComOrThrow([this, methodMetadataToken, &result, actualLength]()
                {
                    ULONG dummy;
                    return m_metaDataImport->GetMethodProps(
                        methodMetadataToken,
                        nullptr,
                        result.Name.data(),
                        actualLength,
                        &dummy,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr);
                }, L"IMetadataImport2::GetMethodName, 2-nd call: Failed to retrieve the characters of the method name");

                TrimTrailingNull(result.Name);
                return result;
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"MetadataImport::GetMethodName failed";
                throw;
            }
        }

        // Gets the name of the method with IID_IMetaDataImport2::GetMethodProps.
        // @returns method's name or std::nullopt in case of error.
        // @param methodMetadataToken : points to the method
        std::optional<MethodProps> TryGetMethodProps(const mdMethodDef methodMetadataToken) const
        {
            ULONG actualLength;
            if (MethodProps result
                ; TryCallCom(
                [this, methodMetadataToken, &result, &actualLength]()
                {
                    return m_metaDataImport->GetMethodProps(
                        methodMetadataToken,
                        &result.EnclosingClass,
                        nullptr,
                        0,
                        &actualLength,
                        &result.Attributes,
                        nullptr,
                        nullptr,
                        &result.CodeRelativeVirtualAddress,
                        &result.ImplementationFlags);
                },
                L"IMetadataImport2::GetMethodName, 1-st call: Failed to retrieve the length of the method name"))
            {

                if (actualLength == 0)
                {
                    return result;
                }

                result.Name = std::wstring(actualLength, L'\0');
                if (TryCallCom(
                    [this, methodMetadataToken, &result, actualLength]()
                    {
                        ULONG dummy;
                        return m_metaDataImport->GetMethodProps(
                            methodMetadataToken,
                            nullptr,
                            result.Name.data(),
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

                    TrimTrailingNull(result.Name);
                    return result;
                }
            }

            m_logger.Log() << L"MetadataImport::TryGetMethodName failed";
            return std::nullopt;
        }

        // Gets the name of the given class/type with IMetaDataImport2::GetTypeDefProps.
        // @param typeDefToken : metadata token of the class or type
        // @returns : name of the class/type or std::nullopt in case of error.
        std::optional<TypeDefProps> TryGetTypeDefProps(const mdTypeDef typeDefToken) const
        {
            ULONG requiredLength;
            if (TypeDefProps result
                ; TryCallCom(
                [this, typeDefToken, &requiredLength, &result]()
                {
                    return m_metaDataImport->GetTypeDefProps(
                        typeDefToken,
                        nullptr,
                        0,
                        &requiredLength,
                        &result.Flags,
                        &result.Extends);
                },
                L"Calling IMetadataImport2::GetTypeDefProps, 1-st try."))
            {
                if (0 == requiredLength)
                {
                    return result;
                }

                result.Name = std::wstring(requiredLength, L'\0');
                if (TryCallCom(
                    [this, typeDefToken, requiredLength, &result]()
                    {
                        ULONG cchDummy;
                        return m_metaDataImport->GetTypeDefProps(
                            typeDefToken,
                            result.Name.data(),
                            requiredLength,
                            &cchDummy,
                            nullptr,
                            nullptr);
                    },
                    L"Calling IMetadataImport2::GetTypeDefProps, 2-nd try."))
                {
                    TrimTrailingNull(result.Name);
                    return result;
                }
            }

            return std::nullopt;
        }

        // Gets the name of the given class/type with IMetaDataImport2::GetTypeDefProps.
        // @param typeDefToken : metadata token of the class or type
        // @returns : name of the class/type.
        // Throws _com_error in case of an error.
        TypeDefProps GetTypeDefProps(const mdTypeDef typeDefToken) const
        {
            TypeDefProps result;
            ULONG requiredLength;
            CallComOrThrow(
                [this, typeDefToken, &requiredLength, &result]()
                {
                    return m_metaDataImport->GetTypeDefProps(
                        typeDefToken,
                        nullptr,
                        0,
                        &requiredLength,
                        &result.Flags,
                        &result.Extends);
                },
                L"Calling IMetadataImport2::GetTypeDefProps, 1-st try.");

            if (0 == requiredLength)
            {
                return result;
            }

            result.Name = std::wstring(requiredLength, L'\0');
            CallComOrThrow(
                [this, typeDefToken, requiredLength, &result]()
                {
                    ULONG cchDummy;
                    return m_metaDataImport->GetTypeDefProps(
                        typeDefToken,
                        result.Name.data(),
                        requiredLength,
                        &cchDummy,
                        nullptr,
                        nullptr);
                },
                L"Calling IMetadataImport2::GetTypeDefProps, 2-nd try.");
            TrimTrailingNull(result.Name);
            return result;
        }

        // Gets the token of the type with the given name in the
        // given class. Throws in case of an error.
        mdTypeDef FindTypeDefByName(
            const std::wstring& name,
            const mdToken enclosingClass) const
        {
            mdTypeDef result;
            this->CallComOrThrow(
                FindTypeDefByNameCallable(
                    name,
                    enclosingClass,
                    result),
                L"Failed to call MetaDataImport::FindTypeDefByName");
            return result;
        }

        // Gets the token of the type with the given name in the
        // given class. Returns std::nullopt in case of an error.
        std::optional<mdTypeDef> TryFindTypeDefByName(
            const std::wstring& name,
            const mdToken enclosingClass) const
        {
            if (mdTypeDef result
                ; this->TryCallCom(
                    FindTypeDefByNameCallable(
                        name,
                        enclosingClass,
                        result),
                    L"Failed to call MetaDataImport::TryFindTypeDefByName"))
            {
                return result;
            }

            return std::nullopt;
        }

        // Gets the tokens of the methods with the given name in the
        // given class. Throws _com_error in case of an error.
        std::vector<mdMethodDef> EnumMethodsWithName(
            const mdTypeDef enclosingType,
            const std::wstring& name) const
        {
            MetaDataEnum enumeration { *this };
            mdMethodDef chunk[16];
            std::vector<mdMethodDef> result{};
            constexpr ULONG chunkLength { std::extent_v<decltype(chunk)> };
            while (true)
            {
                ULONG methodsRetrieved;
                HRESULT peekResult;
                this->CallComOrThrow(
                    EnumMethodsWithNameCallable(
                        enclosingType,
                        name,
                        chunk,
                        chunkLength,
                        methodsRetrieved,
                        enumeration.Value,
                        peekResult),
                    L"Call to metaDataImport::EnumMethodsWithName failed.");

                if (peekResult == S_FALSE)
                {
                    break;
                }

                result.insert(
                    result.cend(),
                    std::cbegin(chunk),
                    std::cend(chunk) + methodsRetrieved);

                if (methodsRetrieved < chunkLength)
                {
                    break;
                }
            }

            return result;
        }

        // Gets the tokens of the methods with the given name in the
        // given class. Returns std::nullopt in case of an error.
        std::optional<std::vector<mdMethodDef>> TryEnumMethodsWithName(
            const mdTypeDef enclosingType,
            const std::wstring& name) const
        {
            MetaDataEnum enumeration { *this };
            mdMethodDef chunk[16];
            std::vector<mdMethodDef> result{};
            constexpr ULONG chunkLength{ std::extent_v<decltype(chunk)> };
            while (true)
            {
                HRESULT peekResult;
                if (ULONG methodsRetrieved
                    ; this->TryCallCom(
                        EnumMethodsWithNameCallable(
                            enclosingType,
                            name,
                            chunk,
                            chunkLength,
                            methodsRetrieved,
                            enumeration.Value,
                            peekResult),
                        L"Call to metaDataImport::EnumMethodsWithName failed."))
                {
                    if (peekResult == S_FALSE)
                    {
                        break;
                    }

                    result.insert(
                        result.cend(),
                        std::cbegin(chunk),
                        std::cend(chunk) + methodsRetrieved);

                    if (methodsRetrieved < chunkLength)
                    {
                        break;
                    }
                }
                else
                {
                    return std::nullopt;
                }
            }

            return result;
        }
    };

    static_assert(IMetadataImport<MetaDataImport<TrivialLogger>>);
}
