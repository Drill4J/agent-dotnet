#pragma once

#include "IMetaDataDispenser.h"
#include "ComWrapperBase.h"
#include "MetaDataAssemblyImport.h"
#include "MetaDataImport.h"
#include "ComInitializer.h"
#include "framework.h"

namespace Drill4dotNet
{
    // Uses an implementation of IMetaDataDispenser to get information about
    // an assembly from .net Profiling API.
    template <IsLogger Logger>
    class MetaDataDispenser : protected ComWrapperBase<Logger>
    {
    private:
        ATL::CComQIPtr<IMetaDataDispenser, &IID_IMetaDataDispenser> m_metaDataDispenser{};

        inline static constexpr int s_NoInit { 0 };
        MetaDataDispenser(Logger logger, int dummy)
            : ComWrapperBase(logger)
        {
        }

        auto InitCallable()
        {
            return [this]()
            {
                return CoCreateInstance(
                    CLSID_CorMetaDataDispenser,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IMetaDataDispenser,
                    (LPVOID*)&m_metaDataDispenser);
            };
        }

        // Fills m_corProfilerInfo. Returns false in case of error.
        bool TryInit()
        {
            ComInitializer<TLogger> initializer(m_logger);
            if (!initializer.TryInitialize())
            {
                m_logger.Log() << L"MetaDataDispenser::TryInit: COM initialization failed.";
                return false;
            }

            return this->TryCallCom(InitCallable(), L"MetaDataDispenser::TryInit: call to CoCreateInstance failed.");
        }

        // Fills m_corProfilerInfo. Throws in case of error.
        void Init()
        {
            try
            {
                ComInitializer<Logger> initializer(m_logger);
                initializer.Initialize();
                this->CallComOrThrow(InitCallable(), L"MetaDataDispenser::Init: call to CoCreateInstance failed.");
            }
            catch (const _com_error&)
            {
                m_logger.Log() << L"MetaDataDispenser::Init: failed.";
                throw;
            }
        }

        // Wraps IMetaDataDispenser::OpenScope.
        auto OpenScopeCallable(
            const std::filesystem::path& path,
            REFIID interfaceId,
            IUnknown** result) const
        {
            return [this, path, interfaceId, &result]()
            {
                return m_metaDataDispenser->OpenScope(
                    path.c_str(),
                    ofRead,
                    interfaceId,
                    result);
            };
        }

    public:

        // Creates a new instance. Throws _com_error in case of an error.
        MetaDataDispenser(Logger logger)
            : MetaDataDispenser(logger, s_NoInit)
        {
            Init();
        }

        // Creates a new instance. Returns std::nullopt in case of an error.
        static std::optional<MetaDataDispenser<Logger>> TryCreate(Logger logger)
        {
            if (MetaDataDispenser<Logger> result(logger, s_NoInit)
                ; result.TryInit())
            {
                return result;
            }

            return std::nullopt;
        }

        // Returns MetaDataAssemblyImport
        // for the given assembly file. Throws on error.
        template <IsLogger TMetaDataLogger = Logger>
        MetaDataAssemblyImport<TMetaDataLogger> OpenScopeMetaDataAssemblyImport(const std::filesystem::path& path, TMetaDataLogger logger) const
        {
            ATL::CComQIPtr<IMetaDataAssemblyImport, &IID_IMetaDataAssemblyImport> metaDataAssemblyImport{};
            this->CallComOrThrow(
                OpenScopeCallable(
                    path,
                    IID_IMetaDataAssemblyImport,
                    (IUnknown**)&metaDataAssemblyImport),
                L"Call to MetaDataDispenser::OpenScope failed.");
            return MetaDataAssemblyImport<TMetaDataLogger>(metaDataAssemblyImport, logger);
        }

        // Returns MetaDataAssemblyImport
        // for the given assembly file. Returns std::nullopt on error.
        template <IsLogger TMetaDataLogger = Logger>
        MetaDataImport<TMetaDataLogger> OpenScopeMetaDataImport(const std::filesystem::path& path, TMetaDataLogger logger) const
        {
            ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport{};
            this->CallComOrThrow(
                OpenScopeCallable(
                    path,
                    IID_IMetaDataImport,
                    (IUnknown**)&metaDataImport),
                L"Call to MetaDataDispenser::OpenScope failed.");
            return MetaDataImport<TMetaDataLogger>(metaDataImport, logger);
        }

        // Returns MetaDataImport for the given assembly file. Throws _com_error on error.
        template <IsLogger TMetaDataLogger = Logger>
        std::optional<MetaDataAssemblyImport<TMetaDataLogger>> TryOpenScopeMetaDataAssemblyImport(const std::filesystem::path& path, TMetaDataLogger logger) const
        {
            if (ATL::CComQIPtr<IMetaDataAssemblyImport, &IID_IMetaDataAssemblyImport> metaDataAssemblyImport{}
                ; this->TryCallCom(
                    OpenScopeCallable(
                        path,
                        IID_IMetaDataAssemblyImport,
                        (IUnknown**)&metaDataAssemblyImport),
                    L"Call to MetaDataDispenser::TryOpenScope failed."))
            {
                return MetaDataAssemblyImport<TMetaDataLogger>(metaDataAssemblyImport, logger);
            }

            return std::nullopt;
        }


        // Returns MetaDataImport for the given assembly file. returns std::nullopt on error.
        template <IsLogger TMetaDataLogger = Logger>
        std::optional<MetaDataImport<TMetaDataLogger>> TryOpenScopeMetaDataImport(const std::filesystem::path& path, TMetaDataLogger logger) const
        {
            if (ATL::CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metaDataImport{}
                ; this->TryCallCom(
                    OpenScopeCallable(
                        path,
                        IID_IMetaDataImport,
                        (IUnknown**)&metaDataImport),
                    L"Call to MetaDataDispenser::TryOpenScope failed."))
            {
                return MetaDataImport<TMetaDataLogger>(metaDataImport, logger);
            }

            return std::nullopt;
        }
    };

    static_assert(IsMetadataDispenser<MetaDataDispenser<TrivialLogger>>);
}
