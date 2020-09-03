#pragma once

#include <string>
#include <sstream>
#include "LogBuffer.h"
#include "InfoHandler.h"
#include "IConnector.h"

namespace Drill4dotNet
{
    // Determines whether the given functor can be
    // used as a source of classes tree.
    template <typename F>
    concept IsTreeProvider = requires (const F & f)
    {
        { f() } -> std::same_as<std::vector<AstEntity>>;
    };

    // Determines whether the given functor can be
    // used as a handler of assembly filter.
    template <typename F>
    concept IsPackagesPrefixesHandler = requires (F f, PackagesPrefixes prefixes)
    {
        { f(prefixes) } -> std::same_as<void>;
    };

    // Determines whether the given functor can be
    // used as a source of coverage data.
    template <typename F>
    concept IsCoverageDataSource = requires (F f)
    {
        { f() } -> std::same_as<std::vector<ExecClassData>>;
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

    static_assert(IsTreeProvider<TrivialTreeProvider>);
    static_assert(IsPackagesPrefixesHandler<TrivialPackagesPrefixesHandler>);

    template <
        IsTreeProvider TreeProvider,
        IsPackagesPrefixesHandler PackagesPrefixesHandler,
        IsCoverageDataSource CoverageDataSource,
        IsConnector TConnector,
        IsLogger Logger>
    class ProClient
    {
    private:
        InfoHandler<Logger> m_infoHandler { Logger {} };
        TConnector m_connector;
        TreeProvider m_treeProvider;
        PackagesPrefixesHandler m_packagesPrefixesHandler;
        CoverageDataSource m_coverageDataSource;

        inline static ProClient* s_this;

        // is called by Kotlin native connector to transfer a message.
        // It pushes a message to the queue and signals the event.
        static void ReceiveMessage(const char* destination, const char* message)
        {
            if (s_this)
            {
                std::wcout << "ReceiveMessage: "
                    << "destination: " << destination
                    << " message: " << message
                    << std::endl;
                if (std::string{ "/agent/load" } == destination)
                {
                    std::vector<AstEntity> classes { s_this->m_treeProvider() };
                    const std::string pluginName{ "test2code" };

                    // send INIT
                    nlohmann::json initMessage = InitInfo{ static_cast<uint32_t>(classes.size()) };
                    s_this->m_connector.SendPluginMessage(
                        pluginName,
                        initMessage.dump());

                    // send INIT_DATA_PART
                    nlohmann::json initDataPartMessage = InitDataPart(classes);
                    s_this->m_connector.SendPluginMessage(
                        pluginName,
                        initDataPartMessage.dump());

                    // send INITIALIZED
                    nlohmann::json initializedMessage = Initialized{};
                    s_this->m_connector.SendPluginMessage(
                        pluginName,
                        initializedMessage.dump());
                }
                else if (std::string{ "/agent/set-packages-prefixes" } == destination)
                {
                    s_this->m_packagesPrefixesHandler(nlohmann::json::parse(message).get<PackagesPrefixes>());
                }
                else if (std::string{ "/plugin/action" } == destination)
                {
                    PluginAction wrapper{ nlohmann::json::parse(message).get<PluginAction>() };
                    nlohmann::json messageText{ nlohmann::json::parse(wrapper.message) };
                    std::string discriminator{ messageText.at("type").get<std::string>() };
                    if (discriminator == StartSession::Discriminator)
                    {
                        StartSession startSession{ messageText.get<StartSession>() };
                        nlohmann::json startMessage = SessionStarted{
                            startSession.payload.startPayload.sessionId,
                            startSession.payload.startPayload.testType,
                            GetCurrentTimeMillis() };
                        s_this->m_connector.SendPluginMessage(
                            "test2code",
                            startMessage.dump());
                    }
                    else if (discriminator == StopSession::Discriminator)
                    {
                        StopSession stopSession{ messageText.get<StopSession>() };
                        nlohmann::json coverageDataPart = CoverDataPart{
                            stopSession.payload.sessionId,
                            s_this->m_coverageDataSource()
                        };

                        s_this->m_connector.SendPluginMessage(
                            "test2code",
                            coverageDataPart.dump());

                        nlohmann::json stopMessage = SessionFinished{
                            stopSession.payload.sessionId,
                            GetCurrentTimeMillis() };

                        s_this->m_connector.SendPluginMessage(
                            "test2code",
                            stopMessage.dump());
                    }
                    else if (discriminator == InitActiveScope::Discriminator)
                    {
                        InitActiveScope init{ messageText.get<decltype(init)>() };
                        nlohmann::json initMessage = ScopeInitialized{
                            init.payload.id,
                            init.payload.name,
                            init.payload.prevId,
                            GetCurrentTimeMillis() };

                        s_this->m_connector.SendPluginMessage(
                            "test2code",
                            initMessage.dump());
                    }
                }
            }
        }

    public:
        ProClient(
            TreeProvider treeProvider,
            PackagesPrefixesHandler packagesPrefixesHandler,
            CoverageDataSource coverageDataSource)
            : m_connector(ReceiveMessage),
            m_treeProvider { treeProvider },
            m_packagesPrefixesHandler { packagesPrefixesHandler },
            m_coverageDataSource { coverageDataSource }
        {
            s_this = this;
        }

        InfoHandler<Logger>& GetInfoHandler() &
        {
            return m_infoHandler;
        }

        TConnector& GetConnector() &
        {
            return m_connector;
        }

        TreeProvider& TreeProvider() &
        {
            return m_treeProvider;
        }

        PackagesPrefixesHandler PackagesPrefixesHandler() &
        {
            return m_packagesPrefixesHandler;
        }
    };
}
