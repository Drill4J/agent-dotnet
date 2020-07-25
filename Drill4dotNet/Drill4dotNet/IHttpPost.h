#pragma once

#include <concepts>
#include <vector>
#include <string>

namespace Drill4dotNet
{
    class HttpResponse
    {
    public:
        long HttpCode;
        std::vector<std::pair<std::string, std::string>> Headers;
    };

    template <typename T>
    concept IsHttpPost = std::default_initializable<T>
        && std::move_constructible<T>
        && std::assignable_from<T&, T&&>
        && requires(T x)
        {
            { x.Url(std::declval<const std::string&>()) } -> std::same_as<T&>;
            { x.Header(std::declval<const std::string&>(), std::declval<const std::string&>()) } -> std::same_as<T&>;
            { x.Body(std::declval<const std::string&>()) } -> std::same_as<T&>;
            { x.Execute() } -> std::same_as<HttpResponse>;
        };
}
