#pragma once

#include <winerror.h>
#include <winnt.h>
#include <comdef.h>

#include "OutputUtils.h"
#include <concepts>

namespace Drill4dotNet
{
    // Provides error checking for a single COM call.
    // Returns false in case of an error.
    // callable : parameterless lambda returning HRESULT.
    // errorHandler : lambda accepting _com_error.
    template <typename TCallable, typename TErrorHandler>
    bool TryCallCom(TCallable callable, const TErrorHandler& errorHandler)
    {
        HRESULT result;
        if (SUCCEEDED(result = callable()))
        {
            return true;
        }

        errorHandler(_com_error(result));
        return false;
    }

    // Provides error handling for a single COM call.
    // Throws _com_error in case of an error.
    // callable : parameterless lambda returning HRESULT.
    template <typename TCallable>
    void CallCom(TCallable callable)
    {
        TryCallCom(callable, [](const _com_error error) { throw error; });
    }

    // Checks the given type is suitable for usage as
    // a logger in classes derived from ComWrapperBase.
    // The type must have methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename TLogger>
    concept Logger = requires (const TLogger& logger)
    {
        { logger.IsLogEnabled() } -> std::same_as<bool>;
        { logger.Log() };
    };

    // Simple logger satisfying the Logger requirements.
    // Will discard all log entries.
    class TrivialLogger
    {
    private:
        class LogWriter
        {
            template <typename T>
            constexpr friend LogWriter& operator<<(LogWriter& target, const T& value) noexcept
            {
                return target;
            }
        };
    public:
        // Creates a new instance.
        // Ignores all arguments.
        template <typename ... Ts>
        constexpr TrivialLogger(const Ts& ... args) noexcept
        {
        }

        // Determines when log is enabled.
        // This implementation will always return false.
        constexpr bool IsLogEnabled() const noexcept
        {
            return false;
        }

        // Returns the object which can accept log entries.
        // The returned object will ignore all log entries.
        constexpr auto Log() const noexcept
        {
            return LogWriter{};
        }
    };

    static_assert(Logger<TrivialLogger>);

    // Provides error handling and logging capabilities
    // to classes wrapping COM objects. Derived classes get
    // per-instance logging context. Their user must provide
    // the logging object upon construction of such objects.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <Logger TLogger>
    class ComWrapperBase
    {
    private:
        template <typename TChar>
        void LogComError(const _com_error& exception, const TChar* detailsFromCaller) const
        {
            if (m_logger.IsLogEnabled())
            {
                m_logger.Log()
                    << detailsFromCaller
                    << " Exception from HRESULT: "
                    << HexOutput(exception.Error())
                    << " (" << exception.ErrorMessage() << ")";
            }
        }

    protected:
        // Logger for this instance, each instance has
        // its own logger, allowing to have per-object context.
        TLogger m_logger;

        ComWrapperBase(const TLogger logger)
            : m_logger(logger)
        {
        }

        // Makes an error handler for the TrycallCom method.
        // The handler logs the COM error and throws _com_error.
        template <typename TChar, size_t length>
        auto LogAndThrowHandler(const TChar(&detailsFromCaller)[length]) const
        {
            return [this, detailsFromCaller](const _com_error exception)
            {
                LogComError(exception, detailsFromCaller);
                throw exception;
            };
        }

        // Makes an error handler for the TrycallCom method.
        // The handler logs the COM error.
        template <typename TChar, size_t length>
        auto LogHandler(const TChar(&detailsFromCaller)[length]) const
        {
            return [this, detailsFromCaller](const _com_error& exception)
            {
                LogComError(exception, detailsFromCaller);
            };
        }

        // Provides error handling for a single COM call.
        // In case of error, logs it and throws _com_error.
        // callable: lambda without parameters, returning HRESULT.
        // detailsFromCaller: added to message logged in case of an error.
        template <typename TCallable, typename TChar, size_t length>
        void CallComOrThrow(TCallable callable, const TChar(&detailsFromCaller)[length])
        {
            Drill4dotNet::TryCallCom(callable, LogAndThrowHandler(detailsFromCaller));
        }

        // Provides error handling for a single COM call.
        // In case of error, logs it and throws _com_error.
        // callable: lambda without parameters, returning HRESULT.
        // detailsFromCaller: added to message logged in case of an error.
        template <typename TCallable, typename TChar, size_t length>
        void CallComOrThrow(TCallable callable, const TChar(&detailsFromCaller)[length]) const
        {
            Drill4dotNet::TryCallCom(callable, LogAndThrowHandler(detailsFromCaller));
        }

        // Provides error handling for a single COM call.
        // If the call is successful, returns true.
        // In case of error, logs it and returns false.
        // callable: lambda without parameters, returning HRESULT.
        // detailsFromCaller: added to message logged in case of an error.
        template <typename TCallable, typename TChar, size_t length>
        bool TryCallCom(TCallable callable, const TChar(&detailsFromCaller)[length])
        {
            return Drill4dotNet::TryCallCom(callable, LogHandler(detailsFromCaller));
        }

        // Provides error handling for a single COM call.
        // If the call is successful, returns true.
        // In case of error, logs it and returns false.
        // callable: lambda without parameters, returning HRESULT.
        // detailsFromCaller: added to message logged in case of an error.
        template <typename TCallable, typename TChar, size_t length>
        bool TryCallCom(TCallable callable, const TChar(&detailsFromCaller)[length]) const
        {
            return Drill4dotNet::TryCallCom(callable, LogHandler(detailsFromCaller));
        }
    };
}
