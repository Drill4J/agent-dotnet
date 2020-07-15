#include "pch.h"

#include "ConnectorImplementation.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>

#include <curl/curl.h>

size_t curlWriteFunc(
    char* data,
    size_t size,
    size_t nmemb,
    void* buffer)
{
    const size_t result { size * nmemb };
    if (std::string* const output { static_cast<decltype(output)>(buffer) }
        ; output != nullptr)
    {
        output->append(data, result);
    }

    return result;
}

int main(int argc, char** argv)
{
    using namespace Drill4dotNet;
    std::wcout << "Test connector sends \"hello\" to and waits for messages from admin." << std::endl;
    std::wcout << "=====================================================" << std::endl;
    std::wcout << "========== PRESS ENTER KEY TO STOP WAITING ==========" << std::endl;
    std::wcout << "=====================================================" << std::endl;

    std::atomic<bool> isKbHit = false;
    std::thread keyThread([&isKbHit]
        {
            isKbHit = false;
            ::getwchar();
            isKbHit = true;
        });

    try
    {
        Connector connector {
            []()
            {
                return std::vector {
                    AstEntity {
                        L"my_path",
                        L"my_name",
                        std::vector {
                            AstMethod {
                                std::wstring { L"my_method" },
                                std::vector { std::wstring { L"my_param" } },
                                L"my_return_type",
                                1,
                                std::vector { uint32_t { 42 } } } } } };
            },
            [](const PackagesPrefixes& prefixes)
            {
                std::wcout << L"Received packages prefixes settings:" << std::endl;
                for (const auto& item : prefixes.packagesPrefixes)
                {
                    std::wcout << L'\t' << item << std::endl;
                }
            }
        };

        connector.InitializeAgent();
        std::this_thread::sleep_for(std::chrono::seconds(16));

        {
            curl_global_init(CURL_GLOBAL_ALL);

            {
                if (CURL* curl { curl_easy_init() }
                    ; curl != nullptr)
                {
                    std::array<char, CURL_ERROR_SIZE> errorBuffer{};
                    curl_easy_setopt(
                        curl,
                        CURLOPT_ERRORBUFFER,
                        errorBuffer.data());

                    curl_easy_setopt(
                        curl,
                        CURLOPT_USERAGENT,
                        "Drill");

                    curl_easy_setopt(
                        curl,
                        CURLOPT_FOLLOWLOCATION,
                        true);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_NOPROGRESS,
                        true);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_MAXREDIRS,
                        50L);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_COOKIEFILE,
                        "");

                    curl_easy_setopt(
                        curl,
                        CURLOPT_NOSIGNAL,
                        true);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_TCP_KEEPALIVE,
                        true);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_ACCEPT_ENCODING,
                        "");

                    curl_easy_setopt(
                        curl,
                        CURLOPT_POST,
                        true);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_URL,
                        "http://localhost:8090/api/login");

                    curl_easy_setopt(
                        curl,
                        CURLOPT_POSTFIELDS,
                        "");

                    curl_easy_setopt(
                        curl,
                        CURLOPT_WRITEDATA,
                        nullptr);

                    std::string header;
                    curl_easy_setopt(
                        curl,
                        CURLOPT_HEADERDATA,
                        &header);

                    curl_easy_setopt(
                        curl,
                        CURLOPT_WRITEFUNCTION,
                        curlWriteFunc);

                    CURLcode curlResult = curl_easy_perform(curl);

                    if (curlResult == CURLcode::CURLE_OK)
                    {
                        long responseCode;
                        curl_easy_getinfo(
                            curl,
                            CURLINFO_RESPONSE_CODE,
                            &responseCode);

                        std::wcout
                            << responseCode
                            << std::endl;

                        std::optional<std::string> authentication;
                        if (responseCode == 200)
                        {
                            std::istringstream headerData(std::move(header));
                            std::string line{};
                            const char authorizationKey[] { "Authorization: " };
                            while (std::getline(headerData, line))
                            {
                                if (line.starts_with(authorizationKey))
                                {
                                    authentication.emplace(std::move(line));
                                    authentication->insert(
                                        std::extent_v<decltype(authorizationKey)> - 1,
                                        "Bearer ");
                                    break;
                                }
                            }

                            std::cout << *authentication;
                            if (authentication.has_value())
                            {
                                curl_easy_setopt(
                                    curl,
                                    CURLOPT_URL,
                                    "http://localhost:8090/api/agents/mysuperAgent/plugins/test2code/dispatch-action");

                                curl_slist* headerList { curl_slist_append(
                                    curl_slist_append(nullptr, authentication->c_str()),
                                    "Content-Type: application/json") };

                                curl_easy_setopt(
                                    curl,
                                    CURLOPT_HTTPHEADER,
                                    headerList);

                                const nlohmann::json body = StartSessionHttpRequest {
                                    StartPayload {
                                        .testType = L"AUTO",
                                        .sessionId = L"{6D2831ED-6A9D-42FB-9375-1ABFAAF81933}",
                                    }
                                };

                                const std::string bodyMock { body.dump() };

                                curl_easy_setopt(
                                    curl,
                                    CURLOPT_POSTFIELDSIZE_LARGE,
                                    bodyMock.size());

                                curl_easy_setopt(
                                    curl,
                                    CURLOPT_COPYPOSTFIELDS,
                                    bodyMock.c_str());

                                CURLcode curlResult2 = curl_easy_perform(curl);

                                if (curlResult2 == CURLcode::CURLE_OK)
                                {
                                    long responseCode2;
                                    curl_easy_getinfo(
                                        curl,
                                        CURLINFO_RESPONSE_CODE,
                                        &responseCode2);

                                    std::wcout
                                        << responseCode2
                                        << std::endl;

                                    std::this_thread::sleep_for(std::chrono::seconds(32));

                                    const nlohmann::json stopBody = StopSession {
                                        SessionPayload {
                                            L"{6D2831ED-6A9D-42FB-9375-1ABFAAF81933}" } };
                                    const std::string stopBodyMock { stopBody.dump() };

                                    curl_easy_setopt(
                                        curl,
                                        CURLOPT_POSTFIELDSIZE_LARGE,
                                        stopBodyMock.size());

                                    curl_easy_setopt(
                                        curl,
                                        CURLOPT_COPYPOSTFIELDS,
                                        stopBodyMock.c_str());

                                    CURLcode curlResult3 = curl_easy_perform(curl);

                                    long responseCode3;
                                    curl_easy_getinfo(
                                        curl,
                                        CURLINFO_RESPONSE_CODE,
                                        &responseCode3);

                                    std::wcout
                                        << responseCode3
                                        << std::endl;


                                    curl_slist_free_all(headerList);
                                }
                            }
                        }
                    }

                    curl_easy_cleanup(curl);
                }
            }

            curl_global_cleanup();
        }

        std::this_thread::sleep_for(std::chrono::seconds(16));
        int x;
        std::cin >> x;
    }
    catch(const std::exception &ex)
    {
        std::wcout
            << "Exception caught: "
            << ex.what()
            << std::endl;
        keyThread.detach();
    }

    return 0;
}
