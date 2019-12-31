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

    wostream& ProClient::Log()
    {
        return m_ostream;
    }

    wistream& ProClient::Key()
    {
        return m_istream;
    }

    wostream& ProClient::operator << (const wstring& s)
    {
        return m_ostream << s;
    }

    wostream& ProClient::operator << (wstring&& s)
    {
        return m_ostream << s;
    }

    wostream& ProClient::operator << (int16_t i)
    {
        return m_ostream << i;
    }

    wostream& ProClient::operator << (int32_t l)
    {
        return m_ostream << l;
    }
}
