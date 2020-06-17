#include "pch.h"

#include "ConnectorImplementation.h"
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
        Connector connector{ []() {
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
                            std::vector { uint32_t { 42 } } } } } }; } };

        connector.InitializeAgent();
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
