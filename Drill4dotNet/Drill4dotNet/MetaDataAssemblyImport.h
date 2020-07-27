#pragma once

#include "IMetaDataAssemblyImport.h"
#include "ComWrapperBase.h"
#include "framework.h"

namespace Drill4dotNet
{
    // Uses IMetaDataAssemblyImport to get information about
    // an assembly from .net Profiling API.
    template <IsLogger Logger>
    class MetaDataAssemblyImport : protected ComWrapperBase<Logger>
    {
    private:
        ATL::CComQIPtr<IMetaDataAssemblyImport, &IID_IMetaDataAssemblyImport> m_metaDataAssemblyImport{};

        // Wraps IMetaDataAssemblyImport::GetAssemblyFromScope.
        auto GetAssemblyFromScopeCallable(mdAssembly& result) const
        {
            return [this, &result]()
            {
                return m_metaDataAssemblyImport->GetAssemblyFromScope(&result);
            };
        }

        // Wraps IMetaDataAssemblyImport::GetAssemblyProps.
        auto GetAssemblyPropsFirstPassCallable(
            const mdAssembly assembly,
            ULONG& nameLength,
            DWORD& flags) const
        {
            return [this, assembly, &nameLength, &flags]()
            {
                return m_metaDataAssemblyImport->GetAssemblyProps(
                    assembly,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    0,
                    &nameLength,
                    nullptr,
                    &flags);
            };
        }

        // Wraps IMetaDataAssemblyImport::GetAssemblyProps.
        auto GetAssemblyPropsSecondPassCallable(
            const mdAssembly assembly,
            const ULONG nameLength,
            const LPWSTR result) const
        {
            return [this, assembly, nameLength, result]()
            {
                return m_metaDataAssemblyImport->GetAssemblyProps(
                    assembly,
                    nullptr,
                    nullptr,
                    nullptr,
                    result,
                    nameLength,
                    nullptr,
                    nullptr,
                    nullptr);
            };
        }

    public:
        // Creates a new instance with the given values.
        MetaDataAssemblyImport(
            ATL::CComQIPtr<IMetaDataAssemblyImport, &IID_IMetaDataAssemblyImport> metaDataAssemblyImport,
            Logger logger)
            : ComWrapperBase(logger),
            m_metaDataAssemblyImport { metaDataAssemblyImport }
        {
        }

        // Calls IMetaDataAssemblyImport::GetAssemblyFromScope.
        // Throws _com_error in case of an error.
        mdAssembly GetAssemblyFromScope() const
        {
            mdAssembly result;
            this->CallComOrThrow(
                GetAssemblyFromScopeCallable(result),
                L"Failed to call MetaDataAssemblyImport::GetAssemblyFromScope.");
            return result;
        }

        // Calls IMetaDataAssemblyImport::GetAssemblyFromScope.
        // Returns an empty optional in case of an error.
        std::optional<mdAssembly> TryGetAssemblyFromScope() const
        {
            if (mdAssembly result
                ; this->TryCallCom(
                    GetAssemblyFromScopeCallable(result),
                    L"Failed to call MetaDataAssemblyImport::TryGetAssemblyFromScope."))
            {
                return result;
            }

            return std::nullopt;
        }

        // Calls IMetaDataAssemblyImport::GetAssemblyProps. Throws on error.
        AssemblyProps GetAssemblyProps(const mdAssembly assembly) const
        {
            try
            {
                ULONG nameLength;
                AssemblyProps result;
                this->CallComOrThrow(
                    GetAssemblyPropsFirstPassCallable(
                        assembly,
                        nameLength,
                        result.Flags),
                    L"Failed to call IMetaDataAssemblyImport::GetAssemblyProps, first pass.");

                if (nameLength == 0)
                {
                    return result;
                }

                result.Name = std::wstring(nameLength, L'\0');
                this->CallComOrThrow(
                    GetAssemblyPropsSecondPassCallable(
                        assembly,
                        nameLength,
                        result.Name.data()),
                    L"Failed to call IMetaDataAssemblyImport::GetAssemblyProps, second pass.");

                TrimTrailingNull(result.Name);
                return result;
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"Failed to call MetaDataAssemblyImport::GetAssemblyProps.";
                throw;
            }
        }

        // Calls IMetaDataAssemblyImport::GetAssemblyProps. Returns std::nullopt on error.
        std::optional<AssemblyProps> TryGetAssemblyProps(const mdAssembly assembly) const
        {
            ULONG nameLength;
            AssemblyProps result;
            if (this->TryCallCom(
                GetAssemblyPropsFirstPassCallable(
                    assembly,
                    nameLength,
                    result.Flags),
                L"Failed to call IMetaDataAssemblyImport::GetAssemblyProps, first pass."))
            {
                result.Name = std::wstring(actualLength, L'\0');

                if (this->TryCallCom(
                    GetAssemblyPropsSecondPassCallable(
                        assembly,
                        nameLength,
                        result.Name.data()),
                    L"Failed to call IMetaDataAssemblyImport::GetAssemblyProps, second pass."))
                {
                    TrimTrailingNull(result.Name);
                    return result;
                }
            }

            m_logger.Log() << L"Failed to call MetaDataAssemblyImport::TryGetAssemblyProps.";
            return std::nullopt;
        }

    };

    static_assert(IsMetadataAssemblyImport<MetaDataAssemblyImport<TrivialLogger>>);
}
