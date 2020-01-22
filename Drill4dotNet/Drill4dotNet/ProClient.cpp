#include "pch.h"
#include "ProClient.h"
#include "InfoHandler.h"

using namespace std;

namespace Drill4dotNet
{
    ProClient::ProClient()
        : m_ostream(wcout)
        , m_istream(wcin)
        , m_infoHandler(new InfoHandler(m_ostream))
    {
    }

    ProClient::~ProClient()
    {
    }

    LogBuffer<std::wostream> ProClient::Log() const
    {
        return LogBuffer<std::wostream>(m_ostream);
    }

    wistream& ProClient::Key()
    {
        return m_istream;
    }
}
