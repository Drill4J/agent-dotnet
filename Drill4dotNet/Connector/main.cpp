#include "pch.h"

#include "Connector.h"
#include <iostream>
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

    std::shared_ptr<IConnector> connector = IConnector::CreateInstance();
    connector->InitializeAgent();
    connector->SendMessage1("hello");
    int i = 0;
    do
    {
        std::optional<std::string> s;
        do
        {
            s = connector->GetNextMessage();
            if (s)
            {
                std::cout << s.value() << std::endl;
            }
        } while (s);
        connector->WaitForNextMessage(1000);
    } while (!isKbHit);

    connector->SendMessage1("bye");
    keyThread.join();
    return 0;
}
