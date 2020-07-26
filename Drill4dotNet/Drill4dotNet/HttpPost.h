#pragma once

#include <curl/curl.h>
#include "IHttpPost.h"

namespace Drill4dotNet
{
    class HttpPost
    {
    private:
        CURL* m_curl { nullptr };
        curl_slist* m_headerList { nullptr };
        std::string m_errorBuffer;

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

        std::string UrlEncode(const std::wstring& data) const;
    };

    static_assert(IsHttpPost<HttpPost>);
}
