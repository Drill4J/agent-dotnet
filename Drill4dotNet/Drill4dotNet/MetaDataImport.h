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
    template <IsLogger Logger>
    class MetaDataImport : protected ComWrapperBase<Logger>
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

        // Wraps IMetaDataImport::EnumTypeDefs.
        auto EnumTypeDefsCallable(
            HCORENUM& enumeration,
            mdTypeDef* types,
            const ULONG typesLength,
            ULONG& typesRetrieved,
            HRESULT& result) const
        {
            return [
                this,
                &enumeration,
                types,
                typesLength,
                &typesRetrieved,
                &result]()
            {
                return result = m_metaDataImport->EnumTypeDefs(
                    &enumeration,
                    types,
                    typesLength,
                    &typesRetrieved);
            };
        }

        // Wraps IMetaDataImport::EnumMethods.
        auto EnumMethodsCallable(
            HCORENUM& enumeration,
            const mdTypeDef enclosingClass,
            mdMethodDef* methods,
            const ULONG methodsLength,
            ULONG& methodsRetrieved,
            HRESULT& result) const
        {
            return [
                this,
                &enumeration,
                enclosingClass,
                methods,
                methodsLength,
                &methodsRetrieved,
                &result]()
            {
                return result = m_metaDataImport->EnumMethods(
                    &enumeration,
                    enclosingClass,
                    methods,
                    methodsLength,
                    &methodsRetrieved);
            };
        }

        // Gets the call to IMetaDataImport2::GetMemberRefProps,
        // which gets the properties and the length of the name.
        // @param memberToken : the token of the reference to
        //     the method or field to retrieve the properties of.
        // @param enclosingClass : will be set to reference the
        //     class containing the member.
        // @param nameActualLength : will be set to the size of
        //     the member name.
        // @param signatureBytes : will be set to point to the
        //     member's signature.
        // @param signatureSize : will be set to the size of
        //     the signature, in bytes.
        auto GetMemberReferenceSignatureFirstStepCallable(
            const mdMemberRef memberToken,
            mdToken& enclosingClass,
            ULONG& nameActualLength,
            const BYTE*& signatureBytes,
            ULONG& signatureSize) const
        {
            return [this, memberToken, &enclosingClass, &nameActualLength, &signatureBytes, &signatureSize]()
            {
                return m_metaDataImport->GetMemberRefProps(
                    memberToken,
                    &enclosingClass,
                    nullptr,
                    0,
                    &nameActualLength,
                    &signatureBytes,
                    &signatureSize);
            };
        }

        // Gets the call to IMetaDataImport2::GetMemberRefProps,
        // which gets the actual name of the member.
        // @param memberToken : the token of the reference to
        //     the method or field to retrieve the name of.
        // @param name : the name will be written to this buffer.
        // @param length : the length of the buffer.
        auto GetMemberReferenceSignatureSecondStepCallable(
            const mdMemberRef memberToken,
            LPWSTR name,
            const ULONG length) const
        {
            return [this, memberToken, &name, length]()
            {
                return m_metaDataImport->GetMemberRefProps(
                    memberToken,
                    nullptr,
                    name,
                    length,
                    nullptr,
                    nullptr,
                    nullptr);
            };
        }

        // Gets the call to IMetaDataImport2::GetSigFromToken,
        // which sets pointers to the signature blob.
        // @param signatureToken : the token of the signature to retrieve.
        // @param signatureBytes : will be set to point to the
        //     signature blob.
        // @param signatureSize : will be set to the size of
        //     the signature, in bytes.
        auto GetSignatureBlobCallable(
            const mdSignature signatureToken,
            const BYTE*& signatureBytes,
            ULONG& signatureSize) const
        {
            return [this, signatureToken, &signatureBytes, &signatureSize]()
            {
                return m_metaDataImport->GetSigFromToken(
                    signatureToken,
                    &signatureBytes,
                    &signatureSize);
            };
        }

        // Returns copy of a function's signature.
        // @param signatureBytes : must point to the
        //     function's signature.
        // @param signatureSize : the size of
        //     the signature, in bytes.
        static std::vector<std::byte> CopySignature(
            const BYTE* signatureBytes,
            ULONG signatureSize)
        {
            return std::vector<std::byte>(
                reinterpret_cast<const std::byte*>(signatureBytes),
                reinterpret_cast<const std::byte*>(signatureBytes + signatureSize));
        }

    public:
        // Captures IID_IMetaDataImport2 object allowing safe access to its methods.
        // metaDataImport: the IID_IMetaDataImport2 object, which allows accessing
        //     information on classes and methods
        // logger: tool to log the exceptions.
        MetaDataImport(
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport,
            Logger logger)
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
                const BYTE* signatureBytes { nullptr };
                ULONG signatureSize { 0 };
                CallComOrThrow([this, methodMetadataToken, &result, &actualLength, &signatureBytes, &signatureSize]()
                {
                    return m_metaDataImport->GetMethodProps(
                        methodMetadataToken,
                        &result.EnclosingClass,
                        nullptr,
                        0,
                        &actualLength,
                        &result.Attributes,
                        &signatureBytes,
                        &signatureSize,
                        &result.CodeRelativeVirtualAddress,
                        &result.ImplementationFlags);
                }, L"IMetadataImport2::GetMethodProps, 1-st call: Failed to retrieve the length of the method name");

                result.SignatureBlob = CopySignature(signatureBytes, signatureSize);

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
                }, L"IMetadataImport2::GetMethodProps, 2-nd call: Failed to retrieve the characters of the method name");

                TrimTrailingNull(result.Name);
                return result;
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"MetadataImport::GetMethodProps failed";
                throw;
            }
        }

        // Gets the name of the method with IID_IMetaDataImport2::GetMethodProps.
        // @returns method's name or std::nullopt in case of error.
        // @param methodMetadataToken : points to the method
        std::optional<MethodProps> TryGetMethodProps(const mdMethodDef methodMetadataToken) const
        {
            ULONG actualLength;
            const BYTE* signatureBytes { nullptr };
            ULONG signatureSize { 0 };
            if (MethodProps result
                ; TryCallCom(
                [this, methodMetadataToken, &result, &actualLength, &signatureBytes, &signatureSize]()
                {
                    return m_metaDataImport->GetMethodProps(
                        methodMetadataToken,
                        &result.EnclosingClass,
                        nullptr,
                        0,
                        &actualLength,
                        &result.Attributes,
                        &signatureBytes,
                        &signatureSize,
                        &result.CodeRelativeVirtualAddress,
                        &result.ImplementationFlags);
                },
                L"IMetadataImport2::GetMethodProps, 1-st call: Failed to retrieve the length of the method name"))
            {
                result.SignatureBlob = CopySignature(signatureBytes, signatureSize);

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
                    L"IMetadataImport2::GetMethodProps, 2-nd call: Failed to retrieve the characters of the method name"))
                {

                    TrimTrailingNull(result.Name);
                    return result;
                }
            }

            m_logger.Log() << L"MetadataImport::TryGetMethodProps failed";
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
            constexpr ULONG chunkLength { 16 };
            std::array<mdMethodDef, chunkLength> chunk;
            std::vector<mdMethodDef> result{};
            while (true)
            {
                ULONG methodsRetrieved;
                HRESULT peekResult;
                this->CallComOrThrow(
                    EnumMethodsWithNameCallable(
                        enclosingType,
                        name,
                        chunk.data(),
                        chunkLength,
                        methodsRetrieved,
                        enumeration.Value,
                        peekResult),
                    L"Call to MetaDataImport::EnumMethodsWithName failed.");

                if (peekResult == S_FALSE)
                {
                    break;
                }

                result.insert(
                    result.cend(),
                    chunk.cbegin(),
                    chunk.cbegin() + methodsRetrieved);

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
            constexpr ULONG chunkLength{ 16 };
            std::array<mdMethodDef, chunkLength> chunk;
            std::vector<mdMethodDef> result{};
            while (true)
            {
                HRESULT peekResult;
                if (ULONG methodsRetrieved
                    ; this->TryCallCom(
                        EnumMethodsWithNameCallable(
                            enclosingType,
                            name,
                            chunk.data(),
                            chunkLength,
                            methodsRetrieved,
                            enumeration.Value,
                            peekResult),
                        L"Call to MetaDataImport::TryEnumMethodsWithName failed."))
                {
                    if (peekResult == S_FALSE)
                    {
                        break;
                    }

                    result.insert(
                        result.cend(),
                        chunk.cbegin(),
                        chunk.cend() + methodsRetrieved);

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

        // Gets the properties of the referenced member.
        // Throws _com_error on errors.
        // @param memberToken : the token of the reference to
        //     the method or field to retrieve the properties for.
        MemberReferenceProps GetMemberReferenceProps(const mdMemberRef memberToken) const
        {
            try
            {
                MemberReferenceProps result;

                ULONG nameActualLength { 0 };
                const BYTE* signatureBytes { nullptr };
                ULONG signatureSize { 0 };

                CallComOrThrow(
                    GetMemberReferenceSignatureFirstStepCallable(
                        memberToken,
                        result.EnclosingClass,
                        nameActualLength,
                        signatureBytes,
                        signatureSize),
                    L"MetadataImport::GetMemberReferenceProps: first call to IMetadataImport2::GetMemberRefProps failed");

                result.SignatureBlob = CopySignature(signatureBytes, signatureSize);

                if (nameActualLength == 0)
                {
                    return result;
                }

                result.Name = std::wstring(nameActualLength, L'\0');
                CallComOrThrow(
                    GetMemberReferenceSignatureSecondStepCallable(
                        memberToken,
                        result.Name.data(),
                        nameActualLength),
                    L"MetadataImport::GetMemberReferenceProps: second call to IMetadataImport2::GetMemberRefProps failed");

                TrimTrailingNull(result.Name);
                return result;
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"MetadataImport::GetMemberReferenceProps failed";
                throw;
            }
        }

        // Gets the properties of the referenced member.
        // Returns std::nullopt in case of errors.
        // @param memberToken : the token of the reference to
        //     the method or field to retrieve the properties for.
        std::optional<MemberReferenceProps> TryGetMemberReferenceProps(const mdMemberRef memberToken) const
        {
            ULONG nameActualLength { 0 };
            const BYTE* signatureBytes { nullptr };
            ULONG signatureSize { 0 };

            if (MemberReferenceProps result
                ; this->TryCallCom(
                    GetMemberReferenceSignatureFirstStepCallable(
                        memberToken,
                        result.EnclosingClass,
                        nameActualLength,
                        signatureBytes,
                        signatureSize),
                    L"MetadataImport::TryGetMemberReferenceProps: first call to IMetadataImport2::GetMemberRefProps failed"))
            {
                result.SignatureBlob = CopySignature(signatureBytes, signatureSize);

                if (nameActualLength == 0)
                {
                    return result;
                }

                result.Name = std::wstring(nameActualLength, L'\0');
                if (this->TryCallCom(
                        GetMemberReferenceSignatureSecondStepCallable(
                            memberToken,
                            result.Name.data(),
                            nameActualLength),
                        L"MetadataImport::TryGetMemberReferenceProps: second call to IMetadataImport2::GetMemberRefProps failed"))
                {
                    TrimTrailingNull(result.Name);
                    return result;
                }
            }

            m_logger.Log() << L"MetadataImport::TryGetMemberReferenceProps failed";
            return std::nullopt;
        }

        // Gets the raw bytes of the signature.
        // Throws _com_error on errors.
        // @param signatureToken : the token of the signature to retrieve.
        std::vector<std::byte> GetSignatureBlob(const mdSignature signatureToken) const
        {
            const BYTE* signatureBytes { nullptr };
            ULONG signatureSize { 0 };
            CallComOrThrow(
                GetSignatureBlobCallable(
                    signatureToken,
                    signatureBytes,
                    signatureSize),
                L"MetadataImport::GetSignatureBlob: call to IMetadataImport2::GetSigFromToken failed.");
            return CopySignature(signatureBytes, signatureSize);
        }

        // Gets the raw bytes of the signature of the referenced member.
        // Returns std::nullopt in case of errors.
        // @param signatureToken : the token of the signature to retrieve.
        std::optional<std::vector<std::byte>> TryGetSignatureBlob(const mdSignature signatureToken) const
        {
            const BYTE* signatureBytes { nullptr };
            ULONG signatureSize { 0 };
            if (!this->TryCallCom(GetSignatureBlobCallable(
                    signatureToken,
                    signatureBytes,
                    signatureSize),
                L"MetadataImport::TryGetSignatureBlob: call to IMetadataImport2::GetSigFromToken failed."))
            {
                return std::nullopt;
            }

            return CopySignature(signatureBytes, signatureSize);
        }

        // Gets the tokens of the types in the module.
        // Throws _com_error in case of an error.
        std::vector<mdTypeDef> EnumTypeDefinitions() const
        {
            MetaDataEnum enumeration { *this };
            constexpr ULONG chunkLength { 16 };
            std::array<mdTypeDef, chunkLength> chunk;
            std::vector<mdTypeDef> result {};
            while (true)
            {
                ULONG typesRetrieved;
                HRESULT peekResult;
                this->CallComOrThrow(
                    EnumTypeDefsCallable(
                        enumeration.Value,
                        chunk.data(),
                        chunkLength,
                        typesRetrieved,
                        peekResult),
                    L"Call to MetaDataImport::EnumTypeDefinitions failed.");

                if (peekResult == S_FALSE)
                {
                    break;
                }

                result.insert(
                    result.cend(),
                    chunk.cbegin(),
                    chunk.cbegin() + typesRetrieved);

                if (typesRetrieved < chunkLength)
                {
                    break;
                }
            }

            return result;
        }

        // Gets the tokens of the types in the module.
        // Returns std::nullopt in case of an error.
        std::optional<std::vector<mdTypeDef>> TryEnumTypeDefinitions() const
        {
            MetaDataEnum enumeration { *this };
            constexpr ULONG chunkLength { 16 };
            std::array<mdTypeDef, chunkLength> chunk;
            std::vector<mdTypeDef> result {};
            while (true)
            {
                HRESULT peekResult;
                if (ULONG typesRetrieved
                    ; this->TryCallCom(
                        EnumTypeDefsCallable(
                            enumeration.Value,
                            chunk.data(),
                            chunkLength,
                            typesRetrieved,
                            peekResult),
                        L"Call to MetaDataImport::TryEnumTypeDefinitions failed."))
                {
                    if (peekResult == S_FALSE)
                    {
                        break;
                    }

                    result.insert(
                        result.cend(),
                        chunk.cbegin(),
                        chunk.cend() + typesRetrieved);

                    if (typesRetrieved < chunkLength)
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

        // Gets the tokens of the methods of the given type.
        // Throws _com_error in case of an error.
        std::vector<mdMethodDef> EnumMethods(const mdTypeDef enclosingType) const
        {
            MetaDataEnum enumeration { *this };
            constexpr ULONG chunkLength { 16 };
            std::array<mdMethodDef, chunkLength> chunk;
            std::vector<mdMethodDef> result {};
            while (true)
            {
                ULONG methodsRetrieved;
                HRESULT peekResult;
                this->CallComOrThrow(
                    EnumMethodsCallable(
                        enumeration.Value,
                        enclosingType,
                        chunk.data(),
                        chunkLength,
                        methodsRetrieved,
                        peekResult),
                    L"Call to MetaDataImport::EnumMethods failed.");

                if (peekResult == S_FALSE)
                {
                    break;
                }

                result.insert(
                    result.cend(),
                    chunk.cbegin(),
                    chunk.cbegin() + methodsRetrieved);

                if (methodsRetrieved < chunkLength)
                {
                    break;
                }
            }

            return result;
        }

        // Gets the tokens of the methods of the given type.
        // Returns std::nullopt in case of an error.
        std::optional<std::vector<mdMethodDef>> TryEnumMethods(const mdTypeDef enclosingClass) const
        {
            MetaDataEnum enumeration { *this };
            constexpr ULONG chunkLength { 16 };
            std::array<mdMethodDef, chunkLength> chunk;
            std::vector<mdMethodDef> result {};
            while (true)
            {
                HRESULT peekResult;
                if (ULONG methodsRetrieved
                    ; this->TryCallCom(
                        EnumMethodsCallable(
                            enumeration.Value,
                            enclosingClass,
                            chunk.data(),
                            chunkLength,
                            methodsRetrieved,
                            peekResult),
                        L"Call to MetaDataImport::TryEnumMethods failed."))
                {
                    if (peekResult == S_FALSE)
                    {
                        break;
                    }

                    result.insert(
                        result.cend(),
                        chunk.cbegin(),
                        chunk.cend() + methodsRetrieved);

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
