#include "pch.h"

#include "HttpPost.h"
#include "OutputUtils.h"

#include <mutex>

namespace Drill4dotNet
{
    static std::mutex CurlGlobalInit{};
    static size_t HttpPostInstances { 0 };

    static void ChangeHttpPost(int delta)
    {
        std::lock_guard guard { CurlGlobalInit };
        HttpPostInstances += delta;
        if (HttpPostInstances == 1)
        {
            curl_global_init(CURL_GLOBAL_ALL);
        }
        else if (HttpPostInstances == 0)
        {
            curl_global_cleanup();
        }
    }

    static size_t CurlWriteFunc(
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

    static std::vector<std::pair<std::string, std::string>> ParseHeaders(std::string&& headers)
    {
        std::istringstream headerData(headers);
        std::string line{};
        std::vector<std::pair<std::string, std::string>> result{};
        std::getline(headerData, line); // Skip HTTP status line
        while (std::getline(headerData, line))
        {
            TrimSpaces(line);
            if (line.empty())
            {
                continue;
            }

            if (line.starts_with(' ') || line.starts_with('\t'))
            {
                if (result.size() == 0)
                {
                    throw std::runtime_error("HttpPost received invalid http headers");
                }

                TrimSpaces(line);
                result.back().second.append(line);
            }
            else
            {
                const size_t delimiter { line.find_first_of(':') };
                if (delimiter == std::string::npos)
                {
                    throw std::runtime_error("HttpPost received invalid http headers");
                }

                std::string key { line.substr(0, delimiter) };
                std::string value { line.substr(delimiter + 1) };
                TrimSpaces(value);
                result.emplace_back(key, value);
            }
        }

        return result;
    }

    HttpPost::HttpPost()
    {
        ChangeHttpPost(+1);
        m_errorBuffer.resize(CURL_ERROR_SIZE);
    }

    HttpPost::~HttpPost()
    {
        FreeInstance();
        ChangeHttpPost(-1);
    }

    HttpPost::HttpPost(HttpPost&& other)
        : m_curl { std::exchange(other.m_curl, nullptr) },
        m_headerList { std::exchange(other.m_headerList, nullptr) },
        m_errorBuffer { std::move(other.m_errorBuffer) }
    {
    }

    HttpPost& HttpPost::operator=(HttpPost&& other) &
    {
        FreeInstance();
        m_headerList = std::exchange(other.m_headerList, nullptr);
        m_curl = std::exchange(other.m_curl, nullptr);
        m_errorBuffer = std::move(other.m_errorBuffer);
        return *this;
    }

    void HttpPost::InitCurlIfRequired()
    {
        if (m_curl == nullptr)
        {
            m_curl = curl_easy_init();
            if (m_curl == nullptr)
            {
                throw std::runtime_error("curl_easy_init failed");
            }

            curl_easy_setopt(
                m_curl,
                CURLOPT_ERRORBUFFER,
                m_errorBuffer.data());

            curl_easy_setopt(
                m_curl,
                CURLOPT_USERAGENT,
                "Drill");

            curl_easy_setopt(
                m_curl,
                CURLOPT_FOLLOWLOCATION,
                true);

            curl_easy_setopt(
                m_curl,
                CURLOPT_NOPROGRESS,
                true);

            curl_easy_setopt(
                m_curl,
                CURLOPT_MAXREDIRS,
                50L);

            curl_easy_setopt(
                m_curl,
                CURLOPT_COOKIEFILE,
                "");

            curl_easy_setopt(
                m_curl,
                CURLOPT_NOSIGNAL,
                true);

            curl_easy_setopt(
                m_curl,
                CURLOPT_TCP_KEEPALIVE,
                true);

            curl_easy_setopt(
                m_curl,
                CURLOPT_ACCEPT_ENCODING,
                "");

            curl_easy_setopt(
                m_curl,
                CURLOPT_POST,
                true);

            curl_easy_setopt(
                m_curl,
                CURLOPT_POSTFIELDS,
                "");

            curl_easy_setopt(
                m_curl,
                CURLOPT_WRITEDATA,
                nullptr);


            curl_easy_setopt(
                m_curl,
                CURLOPT_WRITEFUNCTION,
                CurlWriteFunc);
        }
    }

    void HttpPost::FreeInstance()
    {
        if (m_headerList != nullptr)
        {
            curl_slist_free_all(m_headerList);
        }

        if (m_curl != nullptr)
        {
            curl_easy_cleanup(m_curl);
        }
    }

    HttpPost& HttpPost::Url(const std::string& url) &
    {
        InitCurlIfRequired();

        curl_easy_setopt(
            m_curl,
            CURLOPT_URL,
            url.c_str());

        return *this;
    }


    HttpPost& HttpPost::Header(const std::string& key, const std::string& value) &
    {
        InitCurlIfRequired();

        const char delimiter[] { ':', ' ' };
        std::string header{};
        header.reserve(key.size() + value.size() + std::extent_v<decltype(delimiter)>);
        header.append(key);
        header.append(delimiter, std::extent_v<decltype(delimiter)>);
        header.append(value);
        m_headerList = curl_slist_append(m_headerList, header.c_str());

        return *this;
    }

    HttpPost& HttpPost::Body(const std::string& content) &
    {
        InitCurlIfRequired();

        curl_easy_setopt(
            m_curl,
            CURLOPT_POSTFIELDSIZE_LARGE,
            content.size());

        curl_easy_setopt(
            m_curl,
            CURLOPT_COPYPOSTFIELDS,
            content.c_str());

        return *this;
    }

    HttpResponse HttpPost::Execute() const
    {
        if (m_curl == nullptr)
        {
            throw std::runtime_error("HttpPost has not been properly initialized. Use Url, Header, and Body methods");
        }

        if (m_headerList != nullptr)
        {
            curl_easy_setopt(
                m_curl,
                CURLOPT_HTTPHEADER,
                m_headerList);
        }

        std::string headers;
        curl_easy_setopt(
            m_curl,
            CURLOPT_HEADERDATA,
            &headers);

        const CURLcode curlResult = curl_easy_perform(m_curl);
        HttpResponse result;
        if (curlResult == CURLcode::CURLE_OK)
        {
            curl_easy_getinfo(
                m_curl,
                CURLINFO_RESPONSE_CODE,
                &result.HttpCode);

            result.Headers = ParseHeaders(std::move(headers));
        }
        else
        {
            throw std::runtime_error("General error during curl_easy_perform");
        }

        return result;
    }
}
