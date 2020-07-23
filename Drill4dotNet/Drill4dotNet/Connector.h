#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <concepts>

namespace Drill4dotNet
{
    // Represents a message from Drill admin.
    class ConnectorQueueItem
    {
    public:
        // Type of the message.
        std::wstring Destination;

        // The message payload.
        std::wstring Message;
    };

    // Data about a method sent to Drill admin.
    class AstMethod
    {
    public:
        // Name of the method.
        std::wstring name;

        // Lists the parameters.
        std::vector<std::wstring> params;

        // The return type.
        std::wstring returnType;

        // The count of probes added to the method.
        uint32_t count;

        // The identifiers of the probes added to the method.
        std::vector<uint32_t> probes;
    };

    // Data about a class sent to Drill admin.
    class AstEntity
    {
    public:
        // The assembly where the class is located.
        std::wstring path;

        // The name of the class, with namespace.
        std::wstring name;

        // Information about methods in the class.
        std::vector<AstMethod> methods;
    };

    class PackagesPrefixes
    {
    public:
        std::vector<std::wstring> packagesPrefixes;
    };

    class StartPayload
    {
    public:
        std::wstring testType { L"AUTO" };

        std::wstring sessionId;
    };

    class StartSessionHttpRequest
    {
    public:
        std::string type { "START" };

        StartPayload payload;

        StartSessionHttpRequest() = default;

        StartSessionHttpRequest(StartPayload payload)
            : payload { payload }
        {
        }
    };

    class SessionPayload
    {
    public:
        std::wstring sessionId;
    };

    class StopSession
    {
    public:
        inline static const std::string Discriminator { "STOP" };

        std::string type { Discriminator };

        SessionPayload payload;

        StopSession() = default;

        StopSession(SessionPayload payload)
            : payload { payload }
        {
        }
    };

    class SessionStarted
    {
    public:
        std::string type { "SESSION_STARTED" };

        std::wstring sessionId;

        std::wstring testType;

        int64_t ts;

        SessionStarted() = default;

        SessionStarted(
            std::wstring sessionId,
            std::wstring testType,
            int64_t ts)
            : sessionId { sessionId },
            testType { testType },
            ts { ts }
        {
        }
    };

    class SessionCancelled
    {
    public:
        std::string type { "SESSION_CANCELLED" };

        std::wstring sessionId;

        int64_t ts;

        SessionCancelled() = default;

        SessionCancelled(
            std::wstring sessionId,
            int64_t ts)
            : sessionId { sessionId },
            ts { ts }
        {
        }
    };

    class AllSessionsCancelled
    {
    public:
        std::string type { "ALL_SESSIONS_CANCELLED" };

        std::vector<std::wstring> ids;

        int64_t ts;

        AllSessionsCancelled() = default;

        AllSessionsCancelled(
            std::vector<std::wstring> ids,
            int64_t ts)
            : ids { ids },
            ts { ts }
        {
        }
    };

    class ExecClassData
    {
    public:
        int64_t id { 0 };

        std::wstring className;

        std::vector<bool> probes;

        std::wstring testName { L"" };
    };

    class CoverDataPart
    {
    public:
        std::string type { "COVERAGE_DATA_PART" };

        std::wstring sessionId;

        std::vector<ExecClassData> data;

        CoverDataPart() = default;

        CoverDataPart(
            std::wstring sessionId,
            std::vector<ExecClassData> data)
            : sessionId { sessionId },
            data { data }
        {
        }
    };

    class SessionChanged
    {
    public:
        std::string type { "SESSION_CHANGED" };

        std::wstring sessionId;

        int32_t probeCount;

        SessionChanged() = default;

        SessionChanged(
            std::wstring sessionId,
            int32_t probeCount)
            : sessionId { sessionId },
            probeCount { probeCount }
        {
        }
    };

    class SessionFinished
    {
    public:
        std::string type { "SESSION_FINISHED" };

        std::wstring sessionId;

        int64_t ts;

        SessionFinished() = default;

        SessionFinished(
            std::wstring sessionId,
            int64_t ts)
            : sessionId { sessionId },
            ts { ts }
        {
        }
    };

    class StartSessionPayload
    {
    public:
        std::wstring sessionId;

        StartPayload startPayload;
    };

    class StartSession
    {
    public:
        inline static const std::string Discriminator { "START_AGENT_SESSION" };

        std::string type { Discriminator };

        StartSessionPayload payload;

        StartSession() = default;

        StartSession(StartSessionPayload payload)
            : payload { payload }
        {
        }
    };

    class PluginAction
    {
    public:
        std::wstring id;
        std::wstring message;
    };

    class ScopeInitialized
    {
    public:
        std::string type { "SCOPE_INITIALIZED" };

        std::wstring id;

        std::wstring name;

        std::wstring prevId;

        int64_t ts;

        ScopeInitialized() = default;

        ScopeInitialized(
            std::wstring id,
            std::wstring name,
            std::wstring prevId,
            int64_t ts)
            : id { id },
            name { name },
            prevId { prevId },
            ts { ts }
        {
        }
    };

    class InitScopePayload
    {
    public:
        std::wstring id;
        std::wstring name;
        std::wstring prevId;
    };

    class InitActiveScope
    {
    public:
        inline static const std::string Discriminator { "INIT_ACTIVE_SCOPE" };

        std::string type { Discriminator };

        InitScopePayload payload;

        InitActiveScope() = default;

        InitActiveScope(InitScopePayload payload)
            : payload { payload }
        {
        }
    };

    // Determines whether the given functor can be
    // used as a source of classes tree.
    template <typename F>
    concept IsTreeProvider = requires (const F& f)
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

    // Determines whether the given type can be used for
    // communication between Drill admin and the profiler.
    template <typename T>
    concept IsConnector = requires (T x)
    {
        { x.InitializeAgent() } -> std::same_as<void>;
        { x.SendAgentMessage(
            std::declval<const std::string&>(),
            std::declval<const std::string&>(),
            std::declval<const std::string&>()) } -> std::same_as<void>;
        { x.SendPluginMessage(
            std::declval<const std::string&>(),
            std::declval<const std::string&>()) } -> std::same_as<void>;

        // gets (and pops) the next message from the queue
        // @returns - next message, if available, std::nullopt otherwise
        { x.GetNextMessage() } -> std::same_as<std::optional<ConnectorQueueItem>>;

        // waits for availability of a new message in the given timeout
        // returns when the event signaled.
        { x.WaitForNextMessage() } -> std::same_as<void>;
        { x.WaitForNextMessage(std::declval<const DWORD>()) } -> std::same_as<void>;

        { x.TreeProvider() } -> IsTreeProvider;
        { x.PackagesPrefixesHandler() } -> IsPackagesPrefixesHandler;
    };
}
