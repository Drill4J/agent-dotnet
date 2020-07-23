#pragma once

#include <nlohmann/json.hpp>
#include <concepts>

namespace Drill4dotNet
{
    // Determines whether the given type can be converted
    // to and from JSON using the nlohmann::json library.
    template <typename T>
    concept IsJsonConvertible = requires()
    {
        { to_json(std::declval<nlohmann::json&>(), std::declval<const T&>()) } -> std::same_as<void>;
        { from_json(std::declval<const nlohmann::json&>(), std::declval<T&>()) } -> std::same_as<void>;
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

    // Sent to Drill admin to notify that classes data will be sent.
    class InitInfo
    {
    public:
        std::string type { "INIT" };
        uint32_t classesCount;
        std::string message { "" };
        bool init { true };

        InitInfo(uint32_t classesCount)
            : classesCount { classesCount }
        {
        }
    };

    // Sent to Drill admin to provide information about classes.
    class InitDataPart
    {
    public:
        std::string type { "INIT_DATA_PART" };
        std::vector<AstEntity> astEntities;

        InitDataPart(std::vector<AstEntity> astEntities)
            : astEntities{ std::move(astEntities) }
        {
        }
    };

    // Sent to Drill admin to notify that all information about classes was sent.
    class Initialized
    {
    public:
        std::string type { "INITIALIZED" };
        std::string msg { "" };
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

    // Converts an AstMethod object to json format.
    void to_json(nlohmann::json& target, const AstMethod& data);

    // Gets an AstMethod object from json.
    void from_json(const nlohmann::json& source, AstMethod& target);

    // Converts an AstEntity object to json format.
    void to_json(nlohmann::json& target, const AstEntity& data);

    // Gets an AstEntity object from json.
    void from_json(const nlohmann::json& source, AstEntity& target);

    // Converts an InitInfo object to json format.
    void to_json(nlohmann::json& target, const InitInfo& data);

    // Gets an InitInfo object from json.
    void from_json(const nlohmann::json& source, InitInfo& target);

    // Converts an InitDataPart object to json format.
    void to_json(nlohmann::json& target, const InitDataPart& data);

    // Gets an InitDataPart object from json.
    void from_json(const nlohmann::json& source, InitDataPart& target);

    // Converts an Initialized object to json format.
    void to_json(nlohmann::json& target, const Initialized& data);

    // Gets an Initialized object from json.
    void from_json(const nlohmann::json& source, Initialized& target);

    // Converts a PackagesPrefixes object to json format.
    void to_json(nlohmann::json& target, const PackagesPrefixes& data);

    // Gets a PackagesPrefixes object from json.
    void from_json(const nlohmann::json& source, PackagesPrefixes& target);

    // Converts a PluginAction object to json format.
    void to_json(nlohmann::json& target, const PluginAction& data);

    // Gets a PluginAction object from json.
    void from_json(const nlohmann::json& source, PluginAction& target);

    // Converts a SessionPayload object to json format.
    void to_json(nlohmann::json& target, const SessionPayload& data);

    // Gets a SessionPayload object from json.
    void from_json(const nlohmann::json& source, SessionPayload& target);

    // Converts a StopSession object to json format.
    void to_json(nlohmann::json& target, const StopSession& data);

    // Gets a StopSession object from json.
    void from_json(const nlohmann::json& source, StopSession& target);

    // Converts a StartPayload object to json format.
    void to_json(nlohmann::json& target, const StartPayload& data);

    // Gets a StartPayload object from json.
    void from_json(const nlohmann::json& source, StartPayload& target);

    // Converts a StartSessionHttpRequest object to json format.
    void to_json(nlohmann::json& target, const StartSessionHttpRequest& data);

    // Gets a StartSessionHttpRequest object from json.
    void from_json(const nlohmann::json& source, StartSessionHttpRequest& target);

    // Converts a StartSessionPayload object to json format.
    void to_json(nlohmann::json& target, const StartSessionPayload& data);

    // Gets a StartSessionPayload object from json.
    void from_json(const nlohmann::json& source, StartSessionPayload& target);

    // Converts a StartSession object to json format.
    void to_json(nlohmann::json& target, const StartSession& data);

    // Gets a StartSession object from json.
    void from_json(const nlohmann::json& source, StartSession& target);

    // Converts a SessionStarted object to json format.
    void to_json(nlohmann::json& target, const SessionStarted& data);

    // Gets a SessionStarted object from json.
    void from_json(const nlohmann::json& source, SessionStarted& target);

    // Converts a SessionCancelled object to json format.
    void to_json(nlohmann::json& target, const SessionCancelled& data);

    // Gets a SessionCancelled object from json.
    void from_json(const nlohmann::json& source, SessionCancelled& target);

    // Converts a AllSessionsCancelled object to json format.
    void to_json(nlohmann::json& target, const AllSessionsCancelled& data);

    // Gets a AllSessionsCancelled object from json.
    void from_json(const nlohmann::json& source, AllSessionsCancelled& target);

    // Converts a ExecClassData object to json format.
    void to_json(nlohmann::json& target, const ExecClassData& data);

    // Gets a ExecClassData object from json.
    void from_json(const nlohmann::json& source, ExecClassData& target);

    // Converts a CoverDataPart object to json format.
    void to_json(nlohmann::json& target, const CoverDataPart& data);

    // Gets a CoverDataPart object from json.
    void from_json(const nlohmann::json& source, CoverDataPart& target);

    // Converts a SessionChanged object to json format.
    void to_json(nlohmann::json& target, const SessionChanged& data);

    // Gets a SessionChanged object from json.
    void from_json(const nlohmann::json& source, SessionChanged& target);

    // Converts a SessionFinished object to json format.
    void to_json(nlohmann::json& target, const SessionFinished& data);

    // Gets a SessionFinished object from json.
    void from_json(const nlohmann::json& source, SessionFinished& target);

    // Converts a ScopeInitialized object to json format.
    void to_json(nlohmann::json& target, const ScopeInitialized& data);

    // Gets a ScopeInitialized object from json.
    void from_json(const nlohmann::json& source, ScopeInitialized& target);

    // Converts a InitScopePayload object to json format.
    void to_json(nlohmann::json& target, const InitScopePayload& data);

    // Gets a InitScopePayload object from json.
    void from_json(const nlohmann::json& source, InitScopePayload& target);

    // Converts a InitActiveScope object to json format.
    void to_json(nlohmann::json& target, const InitActiveScope& data);

    // Gets a InitActiveScope object from json.
    void from_json(const nlohmann::json& source, InitActiveScope& target);

    static_assert(IsJsonConvertible<AstMethod>);
    static_assert(IsJsonConvertible<AstEntity>);
    static_assert(IsJsonConvertible<InitInfo>);
    static_assert(IsJsonConvertible<InitDataPart>);
    static_assert(IsJsonConvertible<Initialized>);
    static_assert(IsJsonConvertible<PackagesPrefixes>);
    static_assert(IsJsonConvertible<PluginAction>);
    static_assert(IsJsonConvertible<SessionPayload>);
    static_assert(IsJsonConvertible<StopSession>);
    static_assert(IsJsonConvertible<StartPayload>);
    static_assert(IsJsonConvertible<StartSessionHttpRequest>);
    static_assert(IsJsonConvertible<StartSessionPayload>);
    static_assert(IsJsonConvertible<StartSession>);
    static_assert(IsJsonConvertible<SessionStarted>);
    static_assert(IsJsonConvertible<SessionCancelled>);
    static_assert(IsJsonConvertible<AllSessionsCancelled>);
    static_assert(IsJsonConvertible<ExecClassData>);
    static_assert(IsJsonConvertible<CoverDataPart>);
    static_assert(IsJsonConvertible<SessionChanged>);
    static_assert(IsJsonConvertible<SessionFinished>);
    static_assert(IsJsonConvertible<ScopeInitialized>);
    static_assert(IsJsonConvertible<InitScopePayload>);
    static_assert(IsJsonConvertible<InitActiveScope>);
}
