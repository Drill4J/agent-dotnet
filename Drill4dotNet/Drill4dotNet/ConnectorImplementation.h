#pragma once

#include "Connector.h"

// A hack to solve GNU-specific artifact generated by Kotlin native 1.3.70
#define __attribute__(x) 
#include <agent_connector_api.h>
#undef  __attribute__

#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <queue>
#include <mutex>
#include <concepts>
#include "../Drill4dotNet/OutputUtils.h"
#include <nlohmann/json.hpp>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Drill4dotNet
{
    class DllLoader
    {
    private:
        class Deleter
        {
        private:
            const std::filesystem::path m_fileName;

        public:
            Deleter(const std::filesystem::path& fileName)
                : m_fileName(fileName)
            {
            }

            void operator()(const HMODULE library) const noexcept
            {
                if (library == nullptr)
                {
                    return;
                }

                if (::FreeLibrary(library) == FALSE)
                {
                    std::wcout
                        << L"Failed to unload library "
                        << m_fileName.wstring()
                        << std::endl;
                }
            }
        };

        using UniquePtr = std::unique_ptr<std::remove_pointer_t<HMODULE>, Deleter>;

        static UniquePtr Create(const std::filesystem::path& fileName)
        {
            Deleter deleter { fileName };

            const HMODULE nakedPointer { ::LoadLibraryW(fileName.wstring().c_str()) };
            if (nakedPointer == nullptr)
            {
                throw std::runtime_error(
                    std::string { "Cannot load " } + fileName.string());
            }

            return UniquePtr(nakedPointer, std::move(deleter));
        }

        UniquePtr m_handle;

        static const std::filesystem::path ResolveAbsolutePath(const std::filesystem::path& name)
        {
            const HMODULE hCurrentModule = reinterpret_cast<HMODULE>(&__ImageBase);
            DWORD rSize = _MAX_PATH;
            std::wstring currentModuleFileName(_MAX_PATH, L'\0');
            do
            {
                currentModuleFileName.resize(rSize, L'\0');
                rSize = ::GetModuleFileName(hCurrentModule, currentModuleFileName.data(), static_cast<unsigned long>(currentModuleFileName.size()));
                rSize *= 2;
            } while (::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
            TrimTrailingNulls(currentModuleFileName);

            const std::filesystem::path absolutePath = std::filesystem::path(currentModuleFileName).parent_path() / name;
            std::wcout
                << "ResolveAbsolutePath: "
                << absolutePath.wstring()
                << std::endl;
            return absolutePath;
        }

    public:
        explicit DllLoader(const std::filesystem::path& fileName)
            : m_handle{ Create(ResolveAbsolutePath(fileName)) }
        {
        }

        HMODULE Handle() const noexcept
        {
            return m_handle.get();
        }
    };

    class AgentConnectorDllLoader
        : public DllLoader
    {
    private:
        inline static const std::filesystem::path CONNECTOR_DLL_FILE{ L"agent_connector.dll" };

        template <typename T>
        T ImportFunction(LPCSTR const name)
        {
            void* result = ::GetProcAddress(Handle(), name);
            if (result == nullptr)
            {
                throw std::runtime_error(
                    std::string("Cannot get address of procedure: ")
                    + name
                    + " in "
                    + CONNECTOR_DLL_FILE.string());
            }

            return reinterpret_cast<T>(result);
        }
    public:
        decltype(::agent_connector_symbols)* const agent_connector_symbols;
        decltype(::initialize_agent)* const initialize_agent;
        decltype(::sendMessage)* const sendMessage;
        decltype(::sendPluginMessage)* const sendPluginMessage;

        AgentConnectorDllLoader()
            : DllLoader{ CONNECTOR_DLL_FILE },
            agent_connector_symbols { ImportFunction<decltype(agent_connector_symbols)>("agent_connector_symbols") },
            initialize_agent { ImportFunction<decltype(initialize_agent)>("initialize_agent") },
            sendMessage { ImportFunction<decltype(sendMessage)>("sendMessage") },
            sendPluginMessage{ ImportFunction<decltype(sendPluginMessage)>("sendPluginMessage") }
        {
        }
    };

    class Event
    {
    private:
        class Deleter
        {
        public:
            void operator()(const HANDLE event) const noexcept
            {
                if (event != nullptr)
                {
                    if (::CloseHandle(event) == FALSE)
                    {
                        std::wcout
                            << L"Failed to close event handle."
                            << std::endl;
                    }
                }
            }
        };

        using UniquePtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, Deleter>;
        UniquePtr m_handle;

    public:
        Event(
            LPSECURITY_ATTRIBUTES securityAttributes,
            BOOL manualReset,
            BOOL initialState,
            LPCWSTR const name)
            : m_handle { ::CreateEventW(
                    securityAttributes,
                    manualReset,
                    initialState,
                    name) }
        {
            if (m_handle.get() == nullptr)
            {
                throw std::runtime_error("Cannot create an event in Connector");
            }
        }

        HANDLE Handle() const noexcept
        {
            return m_handle.get();
        }
    };

    static int64_t GetCurrentTimeMillis()
    {
        const std::chrono::system_clock::time_point now {
            std::chrono::system_clock::now() };

        std::tm zero {
           .tm_sec = 0,
           .tm_min = 0,
           .tm_hour = 0,
           .tm_mday = 1,
           .tm_mon = 0,
           .tm_year = 1970 - 1900 };

        std::chrono::system_clock::time_point zeroPoint {
            std::chrono::system_clock::from_time_t(
                mktime(&zero)) };
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - zeroPoint).count();
    }

    template <
        IsTreeProvider TreeProvider,
        IsPackagesPrefixesHandler PackagesPrefixesHandler>
    class Connector
    {
    protected:
        const AgentConnectorDllLoader m_agentLibrary{};

        TreeProvider m_treeProvider;
        PackagesPrefixesHandler m_packagesPrefixesHandler;
        std::queue<ConnectorQueueItem> m_messages;
        std::mutex m_mutex;
        Event m_event { NULL, TRUE, FALSE, NULL };

        // a hack for static callback; it should be replaced by context parameter of callback
        inline static Connector* s_connector;
    protected:
        // is called by Kotlin native connector to transfer a message.
        // It pushes a message to the queue and signals the event.
        static void ReceiveMessage(const char* destination, const char* message)
        {
            if (s_connector)
            {
                std::wcout << "ReceiveMessage: "
                    << "destination: " << destination
                    << " message: " << message
                    << std::endl;
                if (std::string { "/agent/load" } == destination)
                {
                    std::vector<AstEntity> classes { s_connector->m_treeProvider() };
                    const std::string pluginName { "test2code" };

                    // send INIT
                    nlohmann::json initMessage = InitInfo { static_cast<uint32_t>(classes.size()) };
                    s_connector->SendPluginMessage(
                        pluginName,
                        initMessage.dump());

                    // send INIT_DATA_PART
                    nlohmann::json initDataPartMessage = InitDataPart(classes);
                    s_connector->SendPluginMessage(
                        pluginName,
                        initDataPartMessage.dump());

                    // send INITIALIZED
                    nlohmann::json initializedMessage = Initialized{};
                    s_connector->SendPluginMessage(
                        pluginName,
                        initializedMessage.dump());
                }
                else if (std::string { "/agent/set-packages-prefixes" } == destination)
                {
                    s_connector->m_packagesPrefixesHandler(nlohmann::json::parse(message).get<PackagesPrefixes>());
                }
                else if (std::string { "/plugin/action" } == destination)
                {
                    PluginAction wrapper { nlohmann::json::parse(message).get<PluginAction>() };
                    nlohmann::json messageText { nlohmann::json::parse(wrapper.message) };
                    std::string discriminator { messageText.at("type").get<std::string>() };
                    if (discriminator == StartSession::Discriminator)
                    {
                        StartSession startSession { messageText.get<StartSession>() };
                        nlohmann::json startMessage = SessionStarted {
                            startSession.payload.startPayload.sessionId,
                            startSession.payload.startPayload.testType,
                            GetCurrentTimeMillis() };
                        s_connector->SendPluginMessage(
                            "test2code",
                            startMessage.dump());
                    }
                    else if (discriminator == StopSession::Discriminator)
                    {
                        StopSession stopSession{ messageText.get<StopSession>() };
                        nlohmann::json coverageDataPart = CoverDataPart{
                            stopSession.payload.sessionId,
                            std::vector{
                                ExecClassData{
                                    .id = 0,
                                    .className = L"my_path/my_name",
                                    .probes = { true },
                                    .testName = L"my_test"
                                }
                            }
                        };

                        s_connector->SendPluginMessage(
                            "test2code",
                            coverageDataPart.dump());

                        nlohmann::json stopMessage = SessionFinished {
                            stopSession.payload.sessionId,
                            GetCurrentTimeMillis() };

                        s_connector->SendPluginMessage(
                            "test2code",
                            stopMessage.dump());
                    }
                    else if (discriminator == InitActiveScope::Discriminator)
                    {
                        InitActiveScope init { messageText.get<decltype(init)>() };
                        nlohmann::json initMessage = ScopeInitialized {
                            init.payload.id,
                            init.payload.name,
                            init.payload.prevId,
                            GetCurrentTimeMillis() };

                        s_connector->SendPluginMessage(
                            "test2code",
                            initMessage.dump());
                    }
                }
            }
        }

    public:
        Connector(
            TreeProvider treeProvider,
            PackagesPrefixesHandler packagesPrefixesHandler)
            : m_treeProvider { std::move(treeProvider) },
            m_packagesPrefixesHandler { std::move(packagesPrefixesHandler) }
        {
            s_connector = this;
        }

        ~Connector()
        {
            ::SetEvent(m_event.Handle()); // to finish all waits
            ::WaitForSingleObject(m_event.Handle(), 0);
        }

        TreeProvider& TreeProvider() &
        {
            return m_treeProvider;
        }

        PackagesPrefixesHandler& PackagesPrefixesHandler() &
        {
            return m_packagesPrefixesHandler;
        }

        void InitializeAgent()
        {
            std::wcout << "Connector::InitializeAgent start." << std::endl;
            agent_connector_ExportedSymbols* ptr = m_agentLibrary.agent_connector_symbols();
            void (*fun)(const char*, const char*) = ReceiveMessage;
            void* function = (void*)(fun);
            m_agentLibrary.initialize_agent(
                "mysuperAgent",
                "localhost:8090",
                "1.0.0",
                "",
                "fail",
                function);
            std::wcout << "Connector::InitializeAgent end." << std::endl;
        }

        void SendAgentMessage(
            const std::string& messageType,
            const std::string& destination,
            const std::string& content)
        {
            std::cout
                << "Connector::SendAgentMessage: { MessageType = "
                << messageType
                << ", Destination = "
                << destination
                << ", Content = "
                << content
                << "}"
                << std::endl;

            m_agentLibrary.sendMessage(
                messageType.c_str(),
                destination.c_str(),
                content.c_str());
        }

        void SendPluginMessage(
            const std::string& pluginId,
            const std::string& content)
        {
            std::cout
                << "Connector::SendPluginMessage: { PluginId = "
                << pluginId
                << ", Content = "
                << content
                << "}"
                << std::endl;

            m_agentLibrary.sendPluginMessage(
                pluginId.c_str(),
                content.c_str());
        }

        std::optional<ConnectorQueueItem> GetNextMessage()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            if (!m_messages.empty())
            {
                const ConnectorQueueItem result { m_messages.front() };
                m_messages.pop();
                return result;
            }

            return std::nullopt;
        }

        void WaitForNextMessage(DWORD timeout = INFINITE)
        {
            DWORD waitResult = ::WaitForSingleObject(m_event.Handle(), timeout);
            switch (waitResult)
            {
            case WAIT_OBJECT_0:
            case WAIT_TIMEOUT:
                return;
            case WAIT_ABANDONED:
            case WAIT_FAILED:
            default:
                throw std::runtime_error("WaitForSingleObject failed.");
            }
        }
    };

    class TrivialTreeProvider
    {
    public:
        std::vector<AstEntity> operator()() const
        {
            return {};
        }
    };

    class TrivialPackagesPrefixesHandler
    {
    public:
        void operator()(const PackagesPrefixes prefixes) const
        {
        }
    };

    static_assert(IsConnector<Connector<TrivialTreeProvider, TrivialPackagesPrefixesHandler>>);
}
