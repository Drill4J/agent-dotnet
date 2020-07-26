#include "pch.h"

#include <ConnectorImplementation.h>
#include <HttpPost.h>
#include <SessionControl.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>

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

        SessionControl<HttpPost> sessionControl(
            "http://localhost:8090/api",
            L"mysuperAgent",
            L"{6D2831ED-6A9D-42FB-9375-1ABFAAF81933}",
            L"AUTO");

        std::this_thread::sleep_for(std::chrono::seconds(32));

        sessionControl.Stop();

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
