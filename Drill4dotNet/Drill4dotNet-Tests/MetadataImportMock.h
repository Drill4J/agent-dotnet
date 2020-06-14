#pragma once

#include <string>

#include <gmock/gmock.h>

#include "IMetadataImport.h"

namespace Drill4dotNet
{
    class MetadataImportMock
    {
    private:
        inline static std::function<void(MetadataImportMock&)> m_onCreation{};

        class CreationRaii
        {
        public:
            CreationRaii(std::function<void(MetadataImportMock&)> onCreate)
            {
                MetadataImportMock::m_onCreation = onCreate;
            }

            ~CreationRaii()
            {
                MetadataImportMock::m_onCreation = decltype(MetadataImportMock::m_onCreation){};
            }

            CreationRaii(const CreationRaii&) = delete;
            CreationRaii& operator=(const CreationRaii&) & = delete;

            CreationRaii(CreationRaii&&) = default;
            CreationRaii& operator=(CreationRaii&&) & = default;
        };

    public:
        static auto SetOnCreate(std::function<void(MetadataImportMock&)> onCreate)
        {
            return CreationRaii(onCreate);
        }

        MetadataImportMock(TrivialLogger)
        {
            if (m_onCreation)
            {
                m_onCreation(*this);
            }
        }

        MOCK_METHOD(MethodProps, GetMethodProps, (const mdToken token), (const));
        MOCK_METHOD(std::optional<MethodProps>, TryGetMethodProps, (const mdToken token), (const));
        MOCK_METHOD(TypeDefProps, GetTypeDefProps, (const mdTypeDef token), (const));
        MOCK_METHOD(std::optional<TypeDefProps>, TryGetTypeDefProps, (const mdTypeDef token), (const));
        MOCK_METHOD(std::vector<mdMethodDef>, EnumMethodsWithName, (const mdTypeDef enclosingType, const std::wstring& name), (const));
        MOCK_METHOD(std::optional<std::vector<mdMethodDef>>, TryEnumMethodsWithName, (const mdTypeDef enclosingType, const std::wstring& name), (const));
        MOCK_METHOD(mdTypeDef, FindTypeDefByName, (const std::wstring& name, const mdToken enclosingClass), (const));
        MOCK_METHOD(std::optional<mdTypeDef>, TryFindTypeDefByName, (const std::wstring& name, const mdTypeDef enclosingType), (const));
        MOCK_METHOD(MemberReferenceProps, GetMemberReferenceProps, (const mdMemberRef memberToken), (const));
        MOCK_METHOD(std::optional<MemberReferenceProps>, TryGetMemberReferenceProps, (const mdMemberRef memberToken), (const));
        MOCK_METHOD(std::vector<std::byte>, GetSignatureBlob, (const mdSignature signatureToken), (const));
        MOCK_METHOD(std::optional<std::vector<std::byte>>, TryGetSignatureBlob, (const mdSignature signatureToken), (const));
        MOCK_METHOD(std::vector<mdTypeDef>, EnumTypeDefinitions, (), (const));
        MOCK_METHOD(std::optional<std::vector<mdTypeDef>>, TryEnumTypeDefinitions, (), (const));
        MOCK_METHOD(std::vector<mdMethodDef>, EnumMethods, (const mdTypeDef enclosingType), (const));
        MOCK_METHOD(std::optional<std::vector<mdMethodDef>>, TryEnumMethods, (const mdTypeDef enclosingType), (const));
    };

    static_assert(IMetadataImport<MetadataImportMock>);
}
