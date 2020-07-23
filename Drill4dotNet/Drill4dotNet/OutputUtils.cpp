#include "pch.h"

#include "OutputUtils.h"

namespace Drill4dotNet
{
    // Converts a UTF-8 string to a std::wstring.
    // Throws std::runtime_error in case of an error.
    std::wstring DecodeUtf8(const std::string& string)
    {
        if (string == "")
        {
            return std::wstring{};
        }

        std::wstring result(
            MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                string.c_str(),
                string.size(),
                nullptr,
                0),
            L'\0');

        if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            string.c_str(),
            string.size(),
            result.data(),
            result.size()) == 0)
        {
            throw std::runtime_error("Could no decode UTF-8: Invalid UTF-8 string.");
        }

        return result;
    }

    // Converts the given string to UTF-8.
    // Throws std::runtime_error in case of an error.
    std::string EncodeUtf8(const std::wstring& source)
    {
        if (source == L"")
        {
            return {};
        }


        std::string result(
            WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                source.c_str(),
                source.size(),
                nullptr,
                0,
                nullptr,
                nullptr),
            '\0');

        if (WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            source.c_str(),
            source.size(),
            result.data(),
            result.size(),
            nullptr,
            nullptr) == 0)
        {
            throw std::runtime_error("Could no encode UTF-8: Invalid UTF-16 string.");
        }

        return result;
    }
}
