#pragma once

#include "IHttpPost.h"
#include "AdminDataStructures.h"

#include <optional>

namespace Drill4dotNet
{
    template <IsHttpPost HttpPost>
    class SessionControl
    {
    private:
        std::wstring m_sessionId;
        std::optional<HttpPost> m_httpPost{};

    public:
        SessionControl(
            std::string apiEndPoint,
            std::wstring agentName,
            std::wstring sessionId,
            std::wstring testType)
            : m_sessionId { sessionId }
        {
            m_httpPost.emplace();
            const HttpResponse loginResponse { m_httpPost
                ->Url(apiEndPoint + "/login")
                .Execute() };

            if (loginResponse.HttpCode != 200)
            {
                throw std::runtime_error("Request to /api/login was not successful");
            }

            const char authorizationKey[] { "Authorization" };
            const auto authorizationHeader { std::find_if(
                loginResponse.Headers.cbegin(),
                loginResponse.Headers.cend(),
                [&authorizationKey](const auto& value)
                {
                    return value.first == authorizationKey;
                }) };

            if (authorizationHeader == loginResponse.Headers.cend())
            {
                throw std::runtime_error("Request to /api/login has not return Authorization header");
            }

            std::string authorization{ "Bearer " + authorizationHeader->second };

            const nlohmann::json startBody = StartSessionHttpRequest {
                StartPayload {
                    .testType = testType,
                    .sessionId = sessionId,
                }
            };

            const HttpResponse startResponse { m_httpPost
                ->Url(apiEndPoint + "/agents/" + m_httpPost->UrlEncode(agentName) + "/plugins/test2code/dispatch-action")
                .Header(authorizationKey, authorization)
                .Header("Content-Type", "application/json")
                .Body(startBody.dump())
                .Execute() };

            if (startResponse.HttpCode != 200)
            {
                throw std::runtime_error("Request to /dispatch-action was not successful");
            }
        }

        void Stop()
        {
            const nlohmann::json stopBody = StopSession {
                SessionPayload { m_sessionId } };

            const HttpResponse stopResponse { m_httpPost
                ->Body(stopBody.dump())
                .Execute() };

            if (stopResponse.HttpCode != 200)
            {
                throw std::runtime_error("Request to /dispatch-action was not successful");
            }

            m_httpPost.reset();
        }
    };
}
