#include "pch.h"
#include "ProClient.h"
#include "InfoHandler.h"

namespace Drill4dotNet
{
    ProClient::ProClient(const std::shared_ptr<IConnector>& connector)
        : m_ostream(std::wcout)
        , m_istream(std::wcin)
        , m_infoHandler(m_ostream)
        , m_connector(connector)
    {
        m_connector->InitializeAgent();
    }

    ProClient::~ProClient()
    {
    }

    LogBuffer<std::wostream> ProClient::Log() const
    {
        return LogBuffer<std::wostream>(m_ostream);
    }

    std::wistream& ProClient::Key()
    {
        return m_istream;
    }
}
