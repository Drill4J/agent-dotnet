#pragma once

#include "ComWrapperBase.h"

namespace Drill4dotNet
{
    // RAII-style wrapper for CoInitialize / CoUninitialize.
    // with logging and error handling capability.
    template <Logger TLogger>
    class ComInitializer : protected ComWrapperBase<TLogger>
    {
    private:
        bool m_initialized{ false };

        void UnInitialize() const noexcept
        {
            CoUninitialize();
        }

        auto CoInitializeCallable() const
        {
            return []()
            {
                return ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
            };
        }

    public:
        // Creates a new instance.
        // @param logger : the tool to log errors.
        ComInitializer(TLogger logger)
            : ComWrapperBase(logger)
        {
        }

        // Wraps CoInitialize. Throws _com_error in case of an error.
        void Initialize()
        {
            CallComOrThrow(
                CoInitializeCallable(),
                L"ComInitializer::Initialize: failed to call CoInitializeEx.");
            m_initialized = true;
        }

        // Wraps CoInitialize. Returns false in case of an error.
        bool TryInitialize()
        {
            m_initialized = this->TryCallCom(
                CoInitializeCallable(),
                L"ComInitializer::TryInitialize: failed to call CoInitializeEx.");
            return m_initialized;
        }

        // Wraps CoUninitialize.
        ~ComInitializer()
        {
            if (m_initialized)
            {
                UnInitialize();
            }
        }

        ComInitializer(const ComInitializer&) = delete;
        ComInitializer& operator=(const ComInitializer&) & = delete;

        // Move constructor.
        ComInitializer(ComInitializer&& other)
            : m_initialized{ std::exchange(other.m_initialized, false) }
        {
        }

        // Move assignment operator.
        ComInitializer& operator=(ComInitializer&& other) &
        {
            if (m_initialized)
            {
                UnInitialize();
            }

            m_initialized = std::exchange(other.m_initialized, false);
            return *this;
        }
    };
}
