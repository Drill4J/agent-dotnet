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

        // Gets the properties of the referenced member.
        // Throws _com_error on errors.
        { x.GetMemberReferenceProps(std::declval<const mdMemberRef>()) } -> std::same_as<MemberReferenceProps>;

        // Gets the properties of the referenced member.
        // Returns std::nullopt on errors.
        { x.TryGetMemberReferenceProps(std::declval<const mdMemberRef>()) } -> std::same_as<std::optional<MemberReferenceProps>>;

        // Gets the raw bytes of the signature.
        // Throws _com_error on errors.
        { x.GetSignatureBlob(std::declval<const mdSignature>()) } -> std::same_as<std::vector<std::byte>>;

        // Gets the raw bytes of the signature.
        // Returns std::nullopt in case of errors.
        { x.TryGetSignatureBlob(std::declval<const mdSignature>()) } -> std::same_as<std::optional<std::vector<std::byte>>>;

        // Gets the tokens of the types in the module. Throws in case of an error.
        { x.EnumTypeDefinitions() } -> std::same_as<std::vector<mdTypeDef>>;

        // Gets the tokens of the types in the module. Returns std::nullopt in case of an error.
        { x.TryEnumTypeDefinitions() } -> std::same_as<std::optional<std::vector<mdTypeDef>>>;

        // Gets the tokens of the methods of the given type.
        // Throws in case of an error.
        { x.EnumMethods(std::declval<const mdTypeDef>()) } -> std::same_as<std::vector<mdMethodDef>>;

        // Gets the tokens of the methods in the given type.
        // Returns std::nullopt in case of an error.
        { x.TryEnumMethods(std::declval<const mdTypeDef>()) } -> std::same_as<std::optional<std::vector<mdMethodDef>>>;
    };
}
