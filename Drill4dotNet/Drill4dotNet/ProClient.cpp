#include "pch.h"
#include "ProClient.h"

using namespace std;

namespace Drill4dotNet
{
    ProClient::ProClient()
        : m_ostream(wcout)
        , m_istream(wcin)
    {
    }

    ProClient::~ProClient()
    {
    }

    LogBuffer<std::wostream> ProClient::Log()
    {
        return LogBuffer<std::wostream>(m_ostream);
    }

    wistream& ProClient::Key()
    {
        return m_istream;
    }
}
