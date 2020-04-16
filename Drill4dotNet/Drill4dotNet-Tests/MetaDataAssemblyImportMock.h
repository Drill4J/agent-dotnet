#pragma once

#include <string>

#include <gmock/gmock.h>

#include "IMetadataAssemblyImport.h"

namespace Drill4dotNet
{
    class MetaDataAssemblyImportMock
    {
    private:
        inline static std::function<void(MetaDataAssemblyImportMock&)> m_onCreation{};

        class CreationRaii
        {
        public:
            CreationRaii(std::function<void(MetaDataAssemblyImportMock&)> onCreate)
            {
                MetaDataAssemblyImportMock::m_onCreation = onCreate;
            }

            ~CreationRaii()
            {
                MetaDataAssemblyImportMock::m_onCreation = decltype(MetaDataAssemblyImportMock::m_onCreation){};
            }

            CreationRaii(const CreationRaii&) = delete;
            CreationRaii& operator=(const CreationRaii&) & = delete;

            CreationRaii(CreationRaii&&) = default;
            CreationRaii& operator=(CreationRaii&&) & = default;
        };

    public:
        static auto SetOnCreate(std::function<void(MetaDataAssemblyImportMock&)> onCreate)
        {
            return CreationRaii(onCreate);
        }

        MetaDataAssemblyImportMock(TrivialLogger)
        {
            if (m_onCreation)
            {
                m_onCreation(*this);
            }
        }

        MOCK_METHOD(mdAssembly, GetAssemblyFromScope, (), (const));
        MOCK_METHOD(std::optional<mdAssembly>, TryGetAssemblyFromScope, (), (const));
        MOCK_METHOD(AssemblyProps, GetAssemblyProps, (const mdAssembly assembly), (const));
        MOCK_METHOD(std::optional<AssemblyProps>, TryGetAssemblyProps, (const mdAssembly assembly), (const));
    };

    static_assert(IsMetadataAssemblyImport<MetaDataAssemblyImportMock>);
}
