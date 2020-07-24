#pragma once

#include <array>
#include <vector>
#include <curl/curl.h>

namespace Drill4dotNet
{
    class HttpResponse
    {
    public:
        long HttpCode;
        std::vector<std::pair<std::string, std::string>> Headers;
    };

    class HttpPost
    {
    private:
        CURL* m_curl { nullptr };
        curl_slist* m_headerList { nullptr };
        std::array<char, CURL_ERROR_SIZE> m_errorBuffer{};

        void InitCurlIfRequired();
        void FreeInstance();

    public:
        HttpPost();

        ~HttpPost();

        HttpPost(const HttpPost&) = delete;

        HttpPost(HttpPost&& other);

        HttpPost& operator=(const HttpPost&) & = delete;

        HttpPost& operator=(HttpPost&& other) &;

        HttpPost& Url(const std::string& url) &;

        HttpPost& Header(const std::string& key, const std::string& value) &;

        HttpPost& Body(const std::string& content) &;

        HttpResponse Execute() const;
    };
}
