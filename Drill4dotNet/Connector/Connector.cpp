#include "pch.h"

#include "Connector.h"

// A hack to solve GNU-specific artifact generated by Kotlin native 1.3.70
#define __attribute__(x) 
#include "agent_connector_api.h"
#undef  __attribute__

#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <queue>
#include <mutex>

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
            if (nakedPointer == NULL)
            {
                throw std::runtime_error(
                    std::string { "Cannot load " } + fileName.string());
            }

            return UniquePtr(nakedPointer, std::move(deleter));
        }

        UniquePtr m_handle;

    public:
        DllLoader(const std::filesystem::path& fileName)
            : m_handle { Create(fileName)}
        {
        }

        HMODULE Handle() const noexcept
        {
            return m_handle.get();
        }
    };

    class AgentConnectorDllLoader
    {
    private:
        inline static const std::filesystem::path CONNECTOR_DLL_FILE { L"agent_connector.dll" };

        const DllLoader m_library { CONNECTOR_DLL_FILE };

        template <typename T>
        T ImportFunction(LPCSTR const name)
        {
            void* result = ::GetProcAddress(m_library.Handle(), name);
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

        AgentConnectorDllLoader()
            : agent_connector_symbols { ImportFunction<decltype(agent_connector_symbols)>("agent_connector_symbols") },
            initialize_agent { ImportFunction<decltype(initialize_agent)>("initialize_agent") },
            sendMessage { ImportFunction<decltype(sendMessage)>("sendMessage") }
        {
        }
    };

    class Connector : public IConnector
    {
    protected:
        inline static const AgentConnectorDllLoader m_agentConnector{};

        static std::queue<std::string> m_messages;
        static std::mutex m_mutex;
        static HANDLE m_event;

    protected:
        // is called by Kotlin native connector to transfer a message.
        // It pushes a message to the queue and signals the event.
        static void ReceiveMessage(const char* destination, const char* message)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_messages.push(message);
            std::wcout << "ReceiveMessage: "
                << "destination: " << destination
                << "message: " << message << std::endl;
            ::SetEvent(m_event);
        }

    public:
        Connector()
        {
            if (m_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
                !m_event)
            {
                throw std::runtime_error("Cannot create an event in Connector.");
            }
        }

        ~Connector() override
        {
            ::SetEvent(m_event); // to finish all waits
            ::WaitForSingleObject(m_event, 0);
            ::CloseHandle(m_event);
        }

        void InitializeAgent() override
        {
            std::wcout << "Connector::InitializeAgent start." << std::endl;
            agent_connector_ExportedSymbols* ptr = m_agentConnector.agent_connector_symbols();
            void (*fun)(const char*, const char*) = ReceiveMessage;
            void* function = (void*)(fun);
            m_agentConnector.initialize_agent(
                "mysuperAgent",
                "localhost:8090",
                "1.0.0",
                "group",
                "fail",
                function);
            std::wcout << "Connector::InitializeAgent end." << std::endl;
        }

        void SendMessage1(const std::string& content) override
        {
            std::wcout << "Connector::SendMessage1: '" << content.c_str() << "'" << std::endl;
            m_agentConnector.sendMessage("10", content.c_str());
        }

        std::optional<std::string> GetNextMessage() override
        {
            std::string result;
            std::lock_guard<std::mutex> locker(m_mutex);
            if (!m_messages.empty())
            {
                result = m_messages.front();
                m_messages.pop();
                return result;
            }
            return std::nullopt;
        }

        void WaitForNextMessage(DWORD timeout) override
        {
            DWORD waitResult = ::WaitForSingleObject(m_event, timeout);
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

    std::queue<std::string> Connector::m_messages;
    std::mutex Connector::m_mutex;
    HANDLE Connector::m_event{ nullptr };

    std::shared_ptr<IConnector> IConnector::CreateInstance()
    {
        return std::make_shared<Connector>();
    }
}