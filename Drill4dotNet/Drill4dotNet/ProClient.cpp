#include "pch.h"
#include "ProClient.h"
#include "InfoHandler.h"

namespace Drill4dotNet
{
    ProClient::ProClient()
        : m_ostream(std::wcout)
        , m_istream(std::wcin)
        , m_infoHandler(std::make_shared<InfoHandler>(m_ostream))
    {
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
