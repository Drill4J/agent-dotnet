#include "pch.h"

#include "AdminDataStructures.h"
#include "OutputUtils.h"

namespace Drill4dotNet
{
    // Converts an AstMethod object to json format.
    void to_json(nlohmann::json& target, const AstMethod& data)
    {
        std::vector<std::string> params{};
        params.reserve(data.params.size());
        for (const auto& param : data.params)
        {
            params.push_back(EncodeUtf8(param));
        }

        target = nlohmann::json{
            { "name", EncodeUtf8(data.name) },
            { "params", params },
            { "returnType", EncodeUtf8(data.returnType) },
            { "count", data.count },
            { "probes", data.probes }
        };
    }

    // Gets an AstMethod object from json.
    void from_json(const nlohmann::json& source, AstMethod& target)
    {
        target.name = DecodeUtf8(source.at("name").get<std::string>());

        std::vector<std::string> params { source.at("params").get<std::vector<std::string>>() };
        target.params.clear();
        target.params.reserve(params.size());
        for (const auto& param : params)
        {
            target.params.push_back(DecodeUtf8(param));
        }

        target.returnType = DecodeUtf8(source.at("returnType").get<std::string>());

        source.at("count").get_to(target.count);
        source.at("probes").get_to(target.probes);
    }

    // Converts an AstEntity object to json format.
    void to_json(nlohmann::json& target, const AstEntity& data)
    {
        target = nlohmann::json{
            { "path", EncodeUtf8(data.path) },
            { "name", EncodeUtf8(data.name) },
            { "methods", data.methods }
        };
    }

    // Gets an AstEntity object from json.
    void from_json(const nlohmann::json& source, AstEntity& target)
    {
        target.path = DecodeUtf8(source.at("path").get<std::string>());
        target.name = DecodeUtf8(source.at("name").get<std::string>());
        source.at("methods").get_to(target.methods);
    }

    // Converts an InitInfo object to json format.
    void to_json(nlohmann::json& target, const InitInfo& data)
    {
        target = nlohmann::json{
            { "type", data.type },
            { "classesCount", data.classesCount },
            { "message", data.message },
            { "init", data.init }
        };
    }

    // Gets an InitInfo object from json.
    void from_json(const nlohmann::json& source, InitInfo& target)
    {
        source.at("type").get_to(target.type);
        source.at("classesCount").get_to(target.classesCount);
        source.at("message").get_to(target.message);
        source.at("init").get_to(target.init);
    }

    // Converts an InitDataPart object to json format.
    void to_json(nlohmann::json& target, const InitDataPart& data)
    {
        target = nlohmann::json{
            { "type", data.type },
            { "astEntities", data.astEntities }
        };
    }

    // Gets an InitDataPart object from json.
    void from_json(const nlohmann::json& source, InitDataPart& target)
    {
        source.at("type").get_to(target.type);
        source.at("astEntities").get_to(target.astEntities);
    }

    // Converts an Initialized object to json format.
    void to_json(nlohmann::json& target, const Initialized& data)
    {
        target = nlohmann::json{
            { "type", data.type },
            { "msg", data.msg }
        };
    }

    // Gets an Initialized object from json.
    void from_json(const nlohmann::json& source, Initialized& target)
    {
        source.at("type").get_to(target.type);
        source.at("msg").get_to(target.msg);
    }

    // Converts a PackagesPrefixes object to json format.
    void to_json(nlohmann::json& target, const PackagesPrefixes& data)
    {
        std::vector<std::string> packagesPrefixes{};
        packagesPrefixes.reserve(data.packagesPrefixes.size());
        for (const auto& item : data.packagesPrefixes)
        {
            packagesPrefixes.push_back(EncodeUtf8(item));
        }

        target = nlohmann::json { { "packagesPrefixes", packagesPrefixes } };
    }

    // Gets a PackagesPrefixes object from json.
    void from_json(const nlohmann::json& source, PackagesPrefixes& target)
    {
        std::vector<std::string> packagesPrefixes { source.at("packagesPrefixes").get<std::vector<std::string>>() };
        target.packagesPrefixes.clear();
        target.packagesPrefixes.reserve(packagesPrefixes.size());
        for (const auto& item : packagesPrefixes)
        {
            target.packagesPrefixes.push_back(DecodeUtf8(item));
        }
    }

    // Converts a PluginAction object to json format.
    void to_json(nlohmann::json& target, const PluginAction& data)
    {
        target = nlohmann::json {
            { "id", EncodeUtf8(data.id) },
            { "message", EncodeUtf8(data.message) }
        };
    }

    // Gets a PluginAction object from json.
    void from_json(const nlohmann::json& source, PluginAction& target)
    {
        target.id = DecodeUtf8(source.at("id").get<std::string>());
        target.message = DecodeUtf8(source.at("message").get<std::string>());
    }

    // Converts a SessionPayload object to json format.
    void to_json(nlohmann::json& target, const SessionPayload& data)
    {
        target = nlohmann::json { { "sessionId", EncodeUtf8(data.sessionId) } };
    }

    // Gets a SessionPayload object from json.
    void from_json(const nlohmann::json& source, SessionPayload& target)
    {
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
    }

    // Converts a StopSession object to json format.
    void to_json(nlohmann::json& target, const StopSession& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "payload", data.payload }
        };
    }

    // Gets a StopSession object from json.
    void from_json(const nlohmann::json& source, StopSession& target)
    {
        source.at("type").get_to(target.type);
        source.at("payload").get_to(target.payload);
    }

    // Converts a StartPayload object to json format.
    void to_json(nlohmann::json& target, const StartPayload& data)
    {
        target = nlohmann::json {
            { "testType", EncodeUtf8(data.testType) },
            { "sessionId", EncodeUtf8(data.sessionId) }
        };
    }

    // Gets a StartPayload object from json.
    void from_json(const nlohmann::json& source, StartPayload& target)
    {
        target.testType = DecodeUtf8(source.at("testType").get<std::string>());
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
    }

    // Converts a StartSessionHttpRequest object to json format.
    void to_json(nlohmann::json& target, const StartSessionHttpRequest& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "payload", data.payload }
        };
    }

    // Gets a StartSessionHttpRequest object from json.
    void from_json(const nlohmann::json& source, StartSessionHttpRequest& target)
    {
        source.at("type").get_to(target.type);
        source.at("payload").get_to(target.payload);
    }

    // Converts a StartSessionPayload object to json format.
    void to_json(nlohmann::json& target, const StartSessionPayload& data)
    {
        target = nlohmann::json {
            { "sessionId", EncodeUtf8(data.sessionId) },
            { "startPayload", data.startPayload }
        };
    }

    // Gets a StartSessionPayload object from json.
    void from_json(const nlohmann::json& source, StartSessionPayload& target)
    {
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
        source.at("startPayload").get_to(target.startPayload);
    }

    // Converts a StartSession object to json format.
    void to_json(nlohmann::json& target, const StartSession& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "payload", data.payload }
        };
    }

    // Gets a StartSession object from json.
    void from_json(const nlohmann::json& source, StartSession& target)
    {
        source.at("type").get_to(target.type);
        source.at("payload").get_to(target.payload);
    }

    // Converts a SessionStarted object to json format.
    void to_json(nlohmann::json& target, const SessionStarted& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "sessionId", EncodeUtf8(data.sessionId) },
            { "testType", EncodeUtf8(data.testType) },
            { "ts", data.ts }
        };
    }

    // Gets a SessionStarted object from json.
    void from_json(const nlohmann::json& source, SessionStarted& target)
    {
        source.at("type").get_to(target.type);
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
        target.testType = DecodeUtf8(source.at("testType").get<std::string>());
        source.at("ts").get_to(target.ts);
    }

    // Converts a SessionCancelled object to json format.
    void to_json(nlohmann::json& target, const SessionCancelled& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "sessionId", EncodeUtf8(data.sessionId) },
            { "ts", data.ts}
        };
    }

    // Gets a SessionCancelled object from json.
    void from_json(const nlohmann::json& source, SessionCancelled& target)
    {
        source.at("type").get_to(target.type);
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
        source.at("ts").get_to(target.ts);
    }

    // Converts a AllSessionsCancelled object to json format.
    void to_json(nlohmann::json& target, const AllSessionsCancelled& data)
    {
        std::vector<std::string> ids{};
        ids.reserve(data.ids.size());
        for (const auto& id : data.ids)
        {
            ids.push_back(EncodeUtf8(id));
        }

        target = nlohmann::json {
            { "type", data.type },
            { "ids", ids },
            { "ts", data.ts}
        };
    }

    // Gets a AllSessionsCancelled object from json.
    void from_json(const nlohmann::json& source, AllSessionsCancelled& target)
    {
        source.at("type").get_to(target.type);
        std::vector<std::string> ids { source.at("ids").get<decltype(ids)>() };
        target.ids.clear();
        target.ids.reserve(ids.size());
        for (const auto& id : ids)
        {
            target.ids.push_back(DecodeUtf8(id));
        }

        source.at("ts").get_to(target.ts);
    }

    // Converts a ExecClassData object to json format.
    void to_json(nlohmann::json& target, const ExecClassData& data)
    {
        target = nlohmann::json {
            { "id", data.id },
            { "className", EncodeUtf8(data.className) },
            { "probes", data.probes },
            { "testName", EncodeUtf8(data.testName) }
        };
    }

    // Gets a ExecClassData object from json.
    void from_json(const nlohmann::json& source, ExecClassData& target)
    {
        source.at("id").get_to(target.id);
        target.className = DecodeUtf8(source.at("className").get<std::string>());
        source.at("probes").get_to(target.probes);
        target.testName = DecodeUtf8(source.at("testName").get<std::string>());
    }

    // Converts a CoverDataPart object to json format.
    void to_json(nlohmann::json& target, const CoverDataPart& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "sessionId", EncodeUtf8(data.sessionId) },
            { "data", data.data }
        };
    }

    // Gets a CoverDataPart object from json.
    void from_json(const nlohmann::json& source, CoverDataPart& target)
    {
        source.at("type").get_to(target.type);
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
        source.at("data").get_to(target.data);
    }

    // Converts a SessionChanged object to json format.
    void to_json(nlohmann::json& target, const SessionChanged& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "sessionId", EncodeUtf8(data.sessionId) },
            { "probeCount", data.probeCount }
        };
    }

    // Gets a SessionChanged object from json.
    void from_json(const nlohmann::json& source, SessionChanged& target)
    {
        source.at("type").get_to(target.type);
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
        source.at("probeCount").get_to(target.probeCount);
    }

    // Converts a SessionFinished object to json format.
    void to_json(nlohmann::json& target, const SessionFinished& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "sessionId", EncodeUtf8(data.sessionId) },
            { "ts", data.ts }
        };
    }

    // Gets a SessionFinished object from json.
    void from_json(const nlohmann::json& source, SessionFinished& target)
    {
        source.at("type").get_to(target.type);
        target.sessionId = DecodeUtf8(source.at("sessionId").get<std::string>());
        source.at("ts").get_to(target.ts);
    }

    // Converts a ScopeInitialized object to json format.
    void to_json(nlohmann::json& target, const ScopeInitialized& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "id", EncodeUtf8(data.id) },
            { "name", EncodeUtf8(data.name) },
            { "prevId", EncodeUtf8(data.prevId) },
            { "ts", data.ts }
        };
    }

    // Gets a ScopeInitialized object from json.
    void from_json(const nlohmann::json& source, ScopeInitialized& target)
    {
        source.at("type").get_to(target.type);
        target.id = DecodeUtf8(source.at("id").get<std::string>());
        target.name = DecodeUtf8(source.at("name").get<std::string>());
        target.prevId = DecodeUtf8(source.at("prevId").get<std::string>());
        source.at("ts").get_to(target.ts);
    }

    // Converts a InitScopePayload object to json format.
    void to_json(nlohmann::json& target, const InitScopePayload& data)
    {
        target = nlohmann::json {
            { "id", EncodeUtf8(data.id) },
            { "name", EncodeUtf8(data.name) },
            { "prevId", EncodeUtf8(data.prevId) }
        };
    }

    // Gets a InitScopePayload object from json.
    void from_json(const nlohmann::json& source, InitScopePayload& target)
    {
        target.id = DecodeUtf8(source.at("id").get<std::string>());
        target.name = DecodeUtf8(source.at("name").get<std::string>());
        target.prevId = DecodeUtf8(source.at("prevId").get<std::string>());
    }

    // Converts a InitActiveScope object to json format.
    void to_json(nlohmann::json& target, const InitActiveScope& data)
    {
        target = nlohmann::json {
            { "type", data.type },
            { "payload", data.payload }
        };
    }

    // Gets a InitActiveScope object from json.
    void from_json(const nlohmann::json& source, InitActiveScope& target)
    {
        source.at("type").get_to(target.type);
        source.at("payload").get_to(target.payload);
    }
}
