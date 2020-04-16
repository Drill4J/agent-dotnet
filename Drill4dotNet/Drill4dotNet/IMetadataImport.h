#pragma once

#include <string>
#include <vector>
#include <optional>
#include <concepts>

#include "CorDataStructures.h"

namespace Drill4dotNet
{
    // Interface to get information about a module from CLR.
    template <typename T>
    concept IMetadataImport = requires (const T x)
    {
        // Gets the name of the method.
        // Throws _com_error in case of error.
        // methodMetadataToken : points to the method
        { x.GetMethodProps(std::declval<const mdToken>()) } -> std::same_as<MethodProps>;

        // Gets the name of the method.
        // @returns method's name or std::nullopt in case of error.
        // @param methodMetadataToken : points to the method
        { x.TryGetMethodProps(std::declval<const mdToken>()) } -> std::same_as<std::optional<MethodProps>>;

        // Gets the name of the given class/type.
        // @param typeDefToken : metadata token of the class or type
        // @returns : name of the class/type or std::nullopt in case of error.
        { x.TryGetTypeDefProps(std::declval<const mdTypeDef>()) } -> std::same_as<std::optional<TypeDefProps>>;

        // Gets the name of the given class/type.
        // @param typeDefToken : metadata token of the class or type
        // @returns : name of the class/type.
        // Throws _com_error in case of an error.
        { x.GetTypeDefProps(std::declval<const mdTypeDef>()) } -> std::same_as<TypeDefProps>;

        // Gets the tokens of the methods with the given name in the
        // given class. Throws in case of an error.
        { x.EnumMethodsWithName(
            std::declval<const mdTypeDef>(),
            std::declval<const std::wstring&>()) } -> std::same_as<std::vector<mdMethodDef>>;

        // Gets the tokens of the methods with the given name in the
        // given class. Returns std::nullopt in case of an error.
        { x.TryEnumMethodsWithName(
            std::declval<const mdTypeDef>(),
            std::declval<const std::wstring&>()) } -> std::same_as<std::optional<std::vector<mdMethodDef>>>;

        // Gets the token of the type with the given name in the
        // given class. Throws in case of an error.
        { x.FindTypeDefByName(
            std::declval<const std::wstring&>(),
            std::declval<const mdToken>()) } -> std::same_as<mdTypeDef>;

        // Gets the token of the type with the given name in the
        // given class. Returns std::nullopt in case of an error.
        { x.TryFindTypeDefByName(
            std::declval<const std::wstring&>(),
            std::declval<const mdToken>()) } -> std::same_as<std::optional<mdTypeDef>>;
    };
}
