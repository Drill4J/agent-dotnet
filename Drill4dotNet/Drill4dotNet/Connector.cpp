#include "pch.h"

#include "Connector.h"

namespace Drill4dotNet
{
    Connector::DllLoader::Deleter::Deleter(const std::filesystem::path& fileName)
        : m_fileName(fileName)
    {
    }

    void Connector::DllLoader::Deleter::operator()(const HMODULE library) const noexcept
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

    Connector::DllLoader::UniquePtr Connector::DllLoader::Create(const std::filesystem::path& fileName)
    {
        Deleter deleter{ fileName };

        const HMODULE nakedPointer{ ::LoadLibraryW(fileName.wstring().c_str()) };
        if (nakedPointer == nullptr)
        {
            throw std::runtime_error(
                std::string{ "Cannot load " } + fileName.string());
        }

        return UniquePtr(nakedPointer, std::move(deleter));
    }

    const std::filesystem::path Connector::DllLoader::ResolveAbsolutePath(const std::filesystem::path& name)
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

    Connector::DllLoader::DllLoader(const std::filesystem::path& fileName)
            : m_handle{ Create(ResolveAbsolutePath(fileName)) }
    {
    }

    HMODULE Connector::DllLoader::Handle() const noexcept
    {
        return m_handle.get();
    }

    template <typename T>
    T Connector::AgentConnectorDllLoader::ImportFunction(LPCSTR const name)
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

    Connector::AgentConnectorDllLoader::AgentConnectorDllLoader()
        : DllLoader{ CONNECTOR_DLL_FILE },
        agent_connector_symbols{ ImportFunction<decltype(agent_connector_symbols)>("agent_connector_symbols") },
        initialize_agent{ ImportFunction<decltype(initialize_agent)>("initialize_agent") },
        sendMessage{ ImportFunction<decltype(sendMessage)>("sendMessage") },
        sendPluginMessage{ ImportFunction<decltype(sendPluginMessage)>("sendPluginMessage") }
    {
    }

    Connector::Connector(ConnectorReceiveCallback callback)
        : m_callback { callback }
    {
    }

    void Connector::InitializeAgent()
    {
        std::wcout << "Connector::InitializeAgent start." << std::endl;
        agent_connector_ExportedSymbols* ptr = m_agentLibrary.agent_connector_symbols();
        m_agentLibrary.initialize_agent(
            "mysuperAgent",
            "localhost:8090",
            "1.0.0",
            "",
            "fail",
            m_callback);
        std::wcout << "Connector::InitializeAgent end." << std::endl;
    }

    void Connector::SendAgentMessage(
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

    void Connector::SendPluginMessage(
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


    int64_t GetCurrentTimeMillis()
    {
        const std::chrono::system_clock::time_point now{
            std::chrono::system_clock::now() };

        std::tm zero{
           .tm_sec = 0,
           .tm_min = 0,
           .tm_hour = 0,
           .tm_mday = 1,
           .tm_mon = 0,
           .tm_year = 1970 - 1900 };

        std::chrono::system_clock::time_point zeroPoint{
            std::chrono::system_clock::from_time_t(
                mktime(&zero)) };
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - zeroPoint).count();
    }

}
