#pragma once

#include <cstddef>
#include <optional>
#include <ostream>
#include <variant>
#include <vector>
#include "OutputUtils.h"

namespace Drill4dotNet
{
    // The definitions in this file follow
    // ECMA-335, Common Language Infrastructure,
    // part II.23.2 Blobs and signatures
    // https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf

    // The minimum signed value allowed to use with CompressSignatureInteger.
    inline static const int32_t MinCompressedSignatureInteger { -(1 << 28) };

    // The maximum signed value allowed to use with CompressSignatureInteger.
    inline static const int32_t MaxCompressedSignatureInteger { (1 << 28) - 1 };

    // The maximum unsigned value allowed to use with CompressSignatureInteger.
    inline static const uint32_t MaxCompressedSignatureUnsignedInteger { 0x1FFF'FFFF };

    // Compresses the given signed value to the form
    // that can be used in method signature blobs.
    // @param signedValue : the value to compress,
    //     should be between MinCompressedSignatureInteger
    //     and MaxCompressedSignatureInteger.
    std::vector<std::byte> CompressSignatureInteger(const int32_t signedValue);

    // Compresses the given unsigned value to the form
    // that can be used in method signature blobs.
    // @param unsignedValue : the value to compress,
    //     should less than MaxCompressedSignatureUnsignedInteger.
    std::vector<std::byte> CompressSignatureInteger(const uint32_t unsignedValue);

    // Stores the output produced by Parse methods.
    template <typename T>
    class ParseResult
    {
    public:
        // The amount of bytes from the input raw bytes
        // stream, which the Parse method has consumed.
        size_t BytesTaken;

        // The value formed by the Parse method.
        T ParsedValue;
    };

    // Compares output produced by the Parse method.
    // The outputs considered equal if both amount of used
    // bytes and the extracted value are the same.
    // Returns true, if the given values are the same.
    // @param left : the first item to compare.
    // @param right : the second item to compare.
    template <typename T>
    constexpr bool operator==(
        const ParseResult<T> left,
        const ParseResult<T> right)
    {
        return left.BytesTaken == right.BytesTaken
            && left.ParsedValue == right.ParsedValue;
    }

    // Compares output produced by the Parse method.
    // The outputs considered equal if both amount of used
    // bytes and the extracted value are the same.
    // Returns true, if the given values are not the same.
    // @param left : the first item to compare.
    // @param right : the second item to compare.
    template <typename T>
    constexpr bool operator!=(
        const ParseResult<T> left,
        const ParseResult<T> right)
    {
        return !(left == other);
    }

    // Parses a signed integer from the given position in
    // a signature blob.
    // Throws std::runtime_error if the input stream ends
    // unexpectedly or the input data is invalid.
    // @param integerPosition : iterator pointing to the position
    //     in the signature blob, where the compressed value starts.
    // @param sourceEnd : iterator pointing immediately after the
    //     last byte of the method signature.
    ParseResult<int32_t> DecompressSignatureSignedInteger(
        const std::vector<std::byte>::const_iterator integerPosition,
        const std::vector<std::byte>::const_iterator sourceEnd);

    // Parses an unsigned integer from the given position in
    // a signature blob.
    // Throws std::runtime_error if the input stream ends
    // unexpectedly or the input data is invalid.
    // @param integerPosition : iterator pointing to the position
    //     in the signature blob, where the compressed value starts.
    // @param sourceEnd : iterator pointing immediately after the
    //     last byte of the method signature.
    ParseResult<uint32_t> DecompressSignatureUnsignedInteger(
        const std::vector<std::byte>::const_iterator integerPosition,
        const std::vector<std::byte>::const_iterator sourceEnd);

    // Defines possible options for method calling convention in .net.
    enum class MethodCallingConvention
    {
        // Managed code convention.
        Default,

        // Standard C unmanaged call convention.
        C,

        // Standard C++ unmanaged call convention.
        StdCall,

        // Unmanaged C++ call that passes a this pointer.
        ThisCall,

        // Optimized unmanaged C++ call convention.
        FastCall
    };

    // Defines possible options how a .net method uses "this" object.
    enum class MethodThisUsage
    {
        // The method is static, and not allowed to use "this".
        NoThis,

        // The method is a usual instance method, uses "this".
        This,

        // For unmanaged calls only.
        ExplicitThis
    };

    // Defines possible variations of method signature syntax.
    enum class MethodSignatureKind
    {
        // The signature describes a managed call, and
        // if it is a vararg signature, information about
        // optional parameters is not provided.
        MethodInSameAssembly,

        // The signature describes a managed call, and
        // if it is a vararg signature, information about
        // optional parameters is provided.
        MethodInDifferentAssembly,

        // Managed or unmanaged call.
        FreeStandingSignature
    };

    // Represents a method signature, which is a
    // combination of the return type, the parameter types,
    // and the calling convention.
    class MethodSignature;

    // Represents an integral, floating point,
    // object, or string type.
    class PrimitiveType;

    // Represents an array type as a combination of
    // the element type and shape describing array dimensions.
    class ArrayType;

    // Represents a type which is a class.
    class ClassType;

    // Represents a type which is a struct.
    class StructType;

    // Represents a type of an unmanaged
    // function pointer.
    class FunctionPointerType;

    // Represents a closed generic type.
    class GenericInstanceType;

    // Represents a type, which is specified as
    // a generic method argument.
    class MethodGenericArgument;

    // Represents a type, which is specified as
    // a generic type argument.
    class TypeGenericArgument;

    // Represents a type of an unmanaged pointer type.
    class PointerType;

    // Represents a single dimension zero-based array type.
    class ZeroBasedArrayType;

    // Represents a variable type in .net.
    using Type = std::variant<
        PrimitiveType,
        ArrayType,
        ClassType,
        StructType,
        FunctionPointerType,
        GenericInstanceType,
        MethodGenericArgument,
        TypeGenericArgument,
        PointerType,
        ZeroBasedArrayType>;

    // Describes array dimensions.
    class ArrayShape
    {
    public:
        // The number of the dimensions.
        uint32_t Rank;

        // Each element in this vector specifies
        // length of the corresponding dimension.
        // There could be less elements than Rank,
        // in this case other dimensions are considered
        // to be of unknown length. For example, if
        // Rank is 2, and Sizes has 1 element equal to 4,
        // the array will have two dimensions, the first
        // will keep 4 elements, and the second will contain
        // any amount of elements.
        std::vector<uint32_t> Sizes;

        // Each element in this vector specifies
        // lower bound of the corresponding dimension.
        // There could be less elements than Rank,
        // in this case other dimensions are considered
        // to be zero based. For example, if Rank is 2,
        // and LowerBounds has 1 element equal to 1,
        // the array will have two dimensions, the indexes
        // of the first will start with 1, the indexes of
        // the second one will start with 0.
        std::vector<int32_t> LowerBounds;

        // Parses an ArrayShape from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the ArrayShape value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<ArrayShape> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Parses a type token from a method signature blob.
    // Returns a metadata token of type mdtTypeDef, mdtTypeRef, or mdtTypeSpec.
    // @param position : iterator pointing to the position
    //     in the signature blob, where the token value starts.
    // @param end : iterator pointing immediately after the
    //     last byte of the method signature.
    ParseResult<mdToken> DecompressTypeDefOrRefOrSpecEncoded(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end);

    // Compresses a type token to a form that can be used in method signature blobs.
    // @param typeToken : a metadata token of type mdtTypeDef, mdtTypeRef, or mdtTypeSpec.
    // @param target : the vector to store the serialized value in.
    void CompressTypeDefOrRefOrSpecEncoded(
        const mdToken typeToken,
        std::vector<std::byte>& target);

    // Represents a custom modifier.
    // Custom modifier is a reference to some
    // tag type, which gives additional information
    // about how the parameter is passed.
    // Such tag types are typically located in the
    // System.Runtime.CompilerServices namespace,
    // for example, System.Runtime.CompilerServices.IsConst.
    class CustomMod
    {
    public:
        // The value indicating whether the caller
        // must take the semantics of the custom modifier
        // into account to make a successful call.
        bool Required;

        // The reference to the tag type.
        mdToken TypeToken;

        // Parses a custom modifier from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // If a custom modifier starts at the position, the
        // modifier is extracted and returned. If there is no
        // custom modifier at the position, std::nullopt is returned.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the custom modifier value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static std::optional<ParseResult<CustomMod>> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;

        // Parses as many adjacent custom modifiers as possible
        // from the given position in a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // If one or more custom modifiers start at the position, they
        // are extracted and returned. If there is no
        // custom modifier at the position, an empty vector is returned.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the custom modifier value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<std::vector<CustomMod>> ParseSeveral(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes several custom modifiers to the raw bytes form.
        // @param customMods : the collection of custom modifiers, can be empty.
        // @param target : the vector to store the serialized value in.
        static void AppendSeveral(
            const std::vector<CustomMod> customMods,
            std::vector<std::byte>& target);
    };

    // Represents an integral, floating point,
    // object, or string type.
    class PrimitiveType
    {
    private:
        // The selected primitive type option.
        CorElementType m_type;

    public:
        // Creates a primitive type with the given option.
        // Throws std::logic_error if the given option is not primitive.
        // Method PrimitiveType::IsPrimitiveType can be used to
        // determine whether a type is primitive.
        // @param primitiveType : one of primitive type options.
        constexpr PrimitiveType(const CorElementType primitiveType)
            : m_type{ primitiveType }
        {
            if (!IsPrimitiveType(primitiveType))
            {
                throw std::logic_error("The given type is not primitive");
            }
        }

        // The selected primitive type option.
        constexpr CorElementType Type() const noexcept
        {
            return m_type;
        }

        // Gets the value determining whether the given type option is primitive.
        // Returns true, if the given option is an integral type, floating point
        // type, object, or string.
        // @param type : one of possible CorElementType options.
        static constexpr bool IsPrimitiveType(const CorElementType type) noexcept
        {
            switch (type)
            {
            case ELEMENT_TYPE_BOOLEAN:
            case ELEMENT_TYPE_CHAR:
            case ELEMENT_TYPE_I1:
            case ELEMENT_TYPE_U1:
            case ELEMENT_TYPE_I2:
            case ELEMENT_TYPE_U2:
            case ELEMENT_TYPE_I4:
            case ELEMENT_TYPE_U4:
            case ELEMENT_TYPE_I8:
            case ELEMENT_TYPE_U8:
            case ELEMENT_TYPE_R4:
            case ELEMENT_TYPE_R8:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_U:
                return true;
            default:
                return false;
            }
        }
    };

    // Represents an array type as a combination of
    // the element type and shape describing array dimensions.
    class ArrayType
    {
    private:
        // Copies the value from the given
        // ArrayShape to this object.
        // @param other : the source object.
        void Assign(const ArrayType& other);

        // Stores the type of array element.
        // The std::unique_ptr is used, because
        // otherwise, Type, which could be ArrayType,
        // should have included other Type as
        // a field, this is not allowed in C++ type system.
        // sizeof(std::unique_ptr<Type>) does not depend on
        // sizeof(Type), this allows to break the recursion.
        // The pointer will never be nullptr.
        std::unique_ptr<Type> m_elementType;

    public:
        // Describes array dimensions.
        ArrayShape Shape;

        // Creates ArrayType with the given parameters.
        // @param type : the element type.
        // @param shape : the description of array dimensions.
        ArrayType(
            const Type& type,
            const ArrayShape& shape);

        // Creates ArrayType with the given parameters.
        // Both parameters will be in moved-from state after this constructor.
        // @param type : the element type.
        // @param shape : the description of array dimensions.
        ArrayType(
            Type&& type,
            ArrayShape&& shape);

        // Copy constructor.
        ArrayType(const ArrayType& other)
            : Shape(other.Shape)
        {
            Assign(other);
        }

        // Move constructor.
        ArrayType(ArrayType&& other) = default;
        // Move assignment operator.
        ArrayType& operator=(ArrayType&& other) & = default;

        // Copy assignment operator.
        ArrayType& operator=(const ArrayType& other) &
        {
            Assign(other);
            return *this;
        }

        // The type of an element in the array.
        const Type& ElementType() const;

        // The type of an element in the array.
        Type& ElementType();

        // Parses an ArrayType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the ArrayType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<ArrayType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a type which is a class.
    class ClassType
    {
    public:
        // The metadata token of a class.
        mdToken Class;

        // Parses a ClassType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the ClassType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<ClassType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a type which is a struct.
    class StructType
    {
    public:
        // The metadata token of a struct.
        mdToken Struct;

        // Parses a StructType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the StructType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<StructType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a type of an unmanaged
    // function pointer.
    class FunctionPointerType
    {
    private:
        // Copies the value from the given
        // FunctionPointerType to this object.
        // @param other : the source object.
        void Assign(const FunctionPointerType& other);

        // Stores the signature of the target function.
        // The std::unique_ptr is used, because
        // otherwise, Type, which could be FunctionPointerType,
        // should have included other Type as
        // a parameter type, this is not allowed in C++ type system.
        // sizeof(std::unique_ptr<Type>) does not depend on
        // sizeof(Type), this allows to break the recursion.
        // The pointer will never be nullptr.
        std::unique_ptr<MethodSignature> m_targetSignature;

    public:

        // Creates FunctionPointerType with the given target signature.
        // @param signature : the signature of the target function.
        FunctionPointerType(const MethodSignature& signature);

        // Creates FunctionPointerType with the given target signature.
        // @param signature : the signature of the target function.
        //     Will be in moved-from state after this constructor.
        FunctionPointerType(MethodSignature&& signature);

        // Copy constructor.
        FunctionPointerType(const FunctionPointerType& other)
        {
            Assign(other);
        }

        // Move constructor.
        FunctionPointerType(FunctionPointerType&& other) = default;

        // Move assignment operator.
        FunctionPointerType& operator=(FunctionPointerType&& other) & = default;

        // Copy assignment operator.
        FunctionPointerType& operator=(const FunctionPointerType& other) &
        {
            Assign(other);
            return *this;
        }

        // The signature of the target function.
        const MethodSignature& TargetSignature() const;

        // The signature of the target function.
        MethodSignature& TargetSignature();

        // Parses a FunctionPointerType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the FunctionPointerType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<FunctionPointerType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a closed generic type.
    class GenericInstanceType
    {
    private:
        // Copies the value from the given
        // GenericInstanceType to this object.
        // @param other : the source object.
        void Assign(const GenericInstanceType& other);

    public:
        // Value indicating whether
        // GenericType is a class or a struct.
        // True corresponds to struct.
        bool IsValueType;

        // The metadata token of open generic type.
        mdToken GenericType;

        // The types used as parameters to construct
        // a closed generic type from GenericType.
        std::vector<Type> GenericParameters;

        // Creates GenericInstanceType with the given parameters.
        // @param isValueType : value indicating whether
        //     genericType is a class or a struct.
        //     True corresponds to struct.
        // @param genericType : the metadata token of
        //     an open generic type.
        // @param genericParameters : The types used as
        //     parameters to construct a closed generic type
        //     from genericType.
        GenericInstanceType(
            const bool isValueType,
            const mdToken genericType,
            const std::vector<Type>& genericParameters);

        // Creates GenericInstanceType with the given parameters.
        // @param isValueType : value indicating whether
        //     genericType is a class or a struct.
        //     True corresponds to struct.
        // @param genericType : the metadata token of
        //     an open generic type.
        // @param genericParameters : The types used as
        //     parameters to construct a closed generic type
        //     from genericType. Will be in moved-from
        //     state after this constructor.
        GenericInstanceType(
            const bool isValueType,
            const mdToken genericType,
            std::vector<Type>&& genericParameters);

        // Copy constructor.
        GenericInstanceType(const GenericInstanceType& other)
        {
            Assign(other);
        }

        // Move constructor.
        GenericInstanceType(GenericInstanceType&& other) = default;

        // Move assignment operator.
        GenericInstanceType& operator=(GenericInstanceType&& other) & = default;

        // Copy assignment operator.
        GenericInstanceType& operator=(const GenericInstanceType& other) &
        {
            Assign(other);
            return *this;
        }

        // Parses a GenericInstanceType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the GenericInstanceType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<GenericInstanceType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a type, which is specified as
    // a generic method argument.
    class MethodGenericArgument
    {
    public:
        // Number of generic parameter of this method,
        // which this type equal to.
        // Example 1 [C#]:
        // class MyClass
        // {
        //     public void MyFunction<TFirst, TSecond>(TFirst first, TSecond second)
        //     {
        //     }
        // }
        // In this case, in signature blob, MyFunction will be represented as
        // void`2(MethodGenericArguments[0] first, MethodGenericArguments[1] second)
        // For the first parameter, type will be MethodGenericArgument with Index == 0,
        // and for the second parameter, type will be MethodGenericArgument with Index == 1.
        //
        // Example 2 [C#]:
        // class MyClass
        // {
        //     public List<T> Merge<T>(List<T> first, List<T> second)
        //     {
        //         // ...
        //     }
        // }
        // In this case, in signature blob, MyFunction will be represented as
        // GenericInstanceType { ClassType { List`1 }, MethodGenericArguments[0] }`1(
        //     GenericInstanceType { ClassType { List`1 }, MethodGenericArguments[0] } first,
        //     GenericInstanceType { ClassType { List`1 }, MethodGenericArguments[0] } second)
        // In this case, return type and both parameters types will be represented as a closed
        // generic type, constructed from generic type System.Collections.Generic.List, and
        // and the element type of the List will be the same as the generic parameter
        // of the Merge method.
        uint32_t Index;

        // Parses a MethodGenericArgument from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the MethodGenericArgument value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<MethodGenericArgument> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a type, which is specified as
    // a generic type argument.
    class TypeGenericArgument
    {
    public:
        // Number of generic parameter of the type this method belongs to,
        // which this type equal to.
        // Example 1 [C#]:
        // class MyClass<TFirst, TSecond>
        // {
        //     public void MyFunction(TFirst first, TSecond second)
        //     {
        //     }
        // }
        // In this case, in signature blob, MyFunction will be represented as
        // void(TypeGenericArguments[0] first, TypeGenericArguments[1] second)
        // For the first parameter, type will be TypeGenericArgument with Index == 0,
        // and for the second parameter, type will be TypeGenericArgument with Index == 1.
        //
        // Example 2 [C#]:
        // class MyClass<T>
        // {
        //     public List<T> Merge(List<T> first, List<T> second)
        //     {
        //         // ...
        //     }
        // }
        // In this case, in signature blob, MyFunction will be represented as
        // GenericInstanceType { ClassType { List`1 }, TypeGenericArguments[0] }`1(
        //     GenericInstanceType { ClassType { List`1 }, TypeGenericArguments[0] } first,
        //     GenericInstanceType { ClassType { List`1 }, TypeGenericArguments[0] } second)
        // In this case, return type and both parameters types will be represented as a closed
        // generic type, constructed from generic type System.Collections.Generic.List, and
        // and the element type of the List will be the same as the generic parameter
        // of the myClass class.
        uint32_t Index;

        // Parses a TypeGenericArgument from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the TypeGenericArgument value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<TypeGenericArgument> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a type of an unmanaged pointer type.
    class PointerType
    {
    private:
        // Copies the value from the given
        // PointerType to this object.
        // @param other : the source object.
        void Assign(const PointerType& other);

    public:
        // If the pointer is typed, this should
        // hold the description of the value type.
        // If the pointer is not typed, i. e. void*,
        // this is equal to nullptr.
        std::unique_ptr<Type> ValueType;

        // Custom modifiers providing additional
        // details on the semantics of the pointer.
        std::vector<CustomMod> CustomMods;

        // Creates the PointerType with the given parameters.
        // @param valueType : if the pointer is typed, this
        //     should hold the description of the value type.
        //     If the pointer is not typed, i. e. void*,
        //     this is equal to nullptr.
        // @param customMods : custom modifiers providing
        //     additional details on the semantics of the pointer.
        PointerType(
            const std::unique_ptr<Type>& valueType,
            const std::vector<CustomMod>& customMods);

        // Creates the PointerType with the given parameters.
        // Both parameters will be in moved-from state after this constructor.
        // @param valueType : if the pointer is typed, this
        //     should hold the description of the value type.
        //     If the pointer is not typed, i. e. void*,
        //     this is equal to nullptr.
        // @param customMods : custom modifiers providing
        //     additional details on the semantics of the pointer.
        PointerType(
            std::unique_ptr<Type>&& valueType,
            std::vector<CustomMod>&& customMods);

        // Copy constructor.
        PointerType(const PointerType& other)
        {
            Assign(other);
        }

        // Move constructor.
        PointerType(PointerType&& other) = default;

        // Move assignment operator.
        PointerType& operator=(PointerType&& other) & = default;

        // Copy assignment operator.
        PointerType& operator=(const PointerType& other) &
        {
            Assign(other);
            return *this;
        }

        // Parses a PointerType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the PointerType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<PointerType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Represents a single dimension zero-based array type.
    class ZeroBasedArrayType
    {
    private:
        // Copies the value from the given
        // ArrayShape to this object.
        // @param other : the source object.
        void Assign(const ZeroBasedArrayType& other);

        // Stores the type of array element.
        // The std::unique_ptr is used, because
        // otherwise, Type, which could be ZeroBasedArrayType,
        // should have included other Type as
        // a field, this is not allowed in C++ type system.
        // sizeof(std::unique_ptr<Type>) does not depend on
        // sizeof(Type), this allows to break the recursion.
        // The pointer will never be nullptr.
        std::unique_ptr<Type> m_elementType;

    public:
        // Custom modifiers providing additional
        // details on the semantics of array.
        std::vector<CustomMod> CustomMods;

        // Creates ZeroBasedArrayType with the given parameters.
        // @param type : the element type.
        // @param customMods : custom modifiers providing
        //     additional details on the semantics of array.
        ZeroBasedArrayType(
            const Type& elementType,
            const std::vector<CustomMod>& customMods);

        // Creates ZeroBasedArrayType with the given parameters.
        // Both parameters will be in moved-from state after this constructor.
        // @param type : the element type.
        // @param customMods : custom modifiers providing
        //     additional details on the semantics of array.
        ZeroBasedArrayType(
            Type&& elementType,
            std::vector<CustomMod>&& customMods);

        // Copy constructor.
        ZeroBasedArrayType(const ZeroBasedArrayType& other)
        {
            Assign(other);
        }

        // Move constructor.
        ZeroBasedArrayType(ZeroBasedArrayType&& other) = default;

        // Move assignment operator.
        ZeroBasedArrayType& operator=(ZeroBasedArrayType&& other) & = default;

        // Copy assignment operator.
        ZeroBasedArrayType& operator=(const ZeroBasedArrayType& other) &
        {
            Assign(other);
            return *this;
        }

        // The type of an element in the array.
        const Type& ElementType() const;

        // The type of an element in the array.
        Type& ElementType();

        // Parses a ZeroBasedArrayType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the ZeroBasedArrayType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<ZeroBasedArrayType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Parses a Type from the given position in
    // a signature blob.
    // Throws std::runtime_error if the input stream ends
    // unexpectedly or the input data is invalid.
    // @param position : iterator pointing to the position
    //     in the signature blob, where the Type value starts.
    // @param end : iterator pointing immediately after the
    //     last byte of the method signature.
    static ParseResult<Type> ParseType(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end);

    // Serializes the Type value to the raw bytes form.
    // @param type : the Type value to serialize.
    // @param target : the vector to store the serialized value in.
    void AppendTypeToBytes(const Type& type, std::vector<std::byte>& target);

    // Represents parameter of type System.TypedReference.
    class ObsoleteParameterPassed
    {
    };

    // Represents parameter of type other than System.TypedReference.
    class ModernParameterPassed
    {
    public:
        // The type of the parameter.
        Type ParameterType;

        // True, if the parameter is passed by reference.
        bool IsPassedByReference;
    };

    // Represents parameter passed in a method.
    class ParameterType
    {
    public:
        // Custom modifiers providing additional information
        // on the parameter pass semantics.
        std::vector<CustomMod> CustomMods;

        // The type of the parameter, and whether it is
        // passed by reference.
        std::variant<ModernParameterPassed, ObsoleteParameterPassed> PassDescription;

        // Parses a ParameterType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the ParameterType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<ParameterType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // Description of the value returned from a method.
    class ReturnType
    {
    public:
        // Custom modifiers providing additional information
        // on the value pass semantics.
        std::vector<CustomMod> CustomMods;

        // If this equal to std::nullopt, the method does not
        // return any value, i. e. void.
        // Otherwise, describes the return type and pass semantics.
        std::optional<std::variant<ModernParameterPassed, ObsoleteParameterPassed>> PassDescription;

        // Parses a ReturnType from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the ReturnType value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<ReturnType> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;
    };

    // For methods accepting variable amount of
    // parameters, describes how many required and
    // optional parameters are passed.
    class VarArgDescription
    {
    public:
        // How many required parameters were passed.
        size_t RequiredParametersCount;

        // How many additional parameters were passed.
        size_t OptionalParametersCount;
    };

    // Represents a method signature, which is a
    // combination of the return type, the parameter types,
    // and the calling convention.
    class MethodSignature
    {
    private:
        // The syntax variation of this signature.
        MethodSignatureKind m_kind;

        // How this method uses "this" object.
        MethodThisUsage m_thisUsage;

        // Calling convention of the method.
        MethodCallingConvention m_callingConvention;

        // Types of the method parameters.
        std::vector<ParameterType> m_parameterTypes;

        // The type of the method's return value.
        ReturnType m_returnType;

        // For generic methods, the number of generic parameters.
        std::optional<size_t> m_genericParameters;

        // For methods accepting variable amount of
        // parameters, describes how many required and
        // optional parameters are passed.
        std::optional<VarArgDescription> m_varArg;

        // Object capable of outputting method calling convention
        // and return type to standard streams.
        class WritePreambleHolder
        {
        private:
            // The method signature which details should be outputted.
            const MethodSignature& m_value;

        public:
            // Creates a new instance.
            // @param value : the method signature which
            //     details should be outputted.
            WritePreambleHolder(const MethodSignature& value)
                : m_value(value)
            {
            }

            // Outputs method calling convention
            // and return type to standard streams.
            // @param target : the strem to output to.
            // @param holder : stores the value to output.
            template <typename TChar>
            friend std::basic_ostream<TChar>& operator<<(
                std::basic_ostream<TChar>& target,
                const WritePreambleHolder& holder)
            {
                const MethodSignature& self { holder.m_value };
                return target
                    << self.CallingConvention()
                    << " "
                    << self.ThisUsage()
                    << " "
                    << self.ReturnType();
            }

        };

        // Object capable of outputting method
        // parameters to standard streams.
        class WriteParametersHolder
        {
        private:
            // The method signature which details should be outputted.
            const MethodSignature& m_value;

        public:
            // Creates a new instance.
            // @param value : the method signature which
            //     details should be outputted.
            WriteParametersHolder(const MethodSignature& value)
                : m_value(value)
            {
            }

            // Outputs method parameters types to standard streams.
            // @param target : the strem to output to.
            // @param holder : stores the value to output.
            template <typename TChar>
            friend std::basic_ostream<TChar>& operator<<(
                std::basic_ostream<TChar>& target,
                const WriteParametersHolder& holder)
            {
                const MethodSignature& self { holder.m_value };
                if (self.GenericParameters().has_value())
                {
                    target << '`' << *self.GenericParameters();
                }

                target << '(';
                bool first { true };
                for (size_t i { 0 }; i != self.ParameterTypes().size(); ++i)
                {
                    if (self.VarArg().has_value() && i == self.VarArg()->RequiredParametersCount)
                    {
                        if (!first)
                        {
                            target << ", ";
                        }
                        else
                        {
                            first = false;
                        }

                        target << "... ";
                    }

                    if (!first)
                    {
                        target << ", ";
                    }

                    target << self.ParameterTypes()[i];
                    first = false;
                };

                if (self.VarArg().has_value() && self.VarArg()->OptionalParametersCount == 0)
                {
                    target << (first ? "..." : ", ...");
                }

                return target << ')';
            }
        };

    public:
        // Creates an instance of MethodSignature with the given values.
        // @param kind : the syntax variation of this signature.
        // @param thisUsage : how this method uses "this" object.
        // @param callingConvention : calling convention of the method.
        // @param parameterTypes : types of the method parameters.
        // @param returnType : the type of the method's return value.
        // @param genericParameters : for generic methods, the number of
        //     generic parameters.
        // @param varArg : for methods accepting variable amount of
        // parameters, describes how many required and
        // optional parameters are passed.
        MethodSignature(
            const MethodSignatureKind kind,
            const MethodThisUsage thisUsage,
            const MethodCallingConvention callingConvention,
            std::vector<ParameterType> parameterTypes,
            ReturnType returnType,
            std::optional<size_t> genericParameters,
            std::optional<VarArgDescription> varArg);

        // Parses a MethodSignature from the given position in
        // a signature blob.
        // Throws std::runtime_error if the input stream ends
        // unexpectedly or the input data is invalid.
        // @param position : iterator pointing to the position
        //     in the signature blob, where the MethodSignature value starts.
        // @param end : iterator pointing immediately after the
        //     last byte of the method signature.
        static ParseResult<MethodSignature> Parse(
            const std::vector<std::byte>::const_iterator position,
            const std::vector<std::byte>::const_iterator end);

        // Serializes the value to the raw bytes form.
        // @param target : the vector to store the serialized value in.
        void AppendToBytes(std::vector<std::byte>& target) const;

        // How this method uses "this" object.
        MethodThisUsage ThisUsage() const
        {
            return m_thisUsage;
        }

        // Calling convention of the method.
        MethodCallingConvention CallingConvention() const
        {
            return m_callingConvention;
        }

        // Types of the method parameters.
        const std::vector<ParameterType>& ParameterTypes() const&
        {
            return m_parameterTypes;
        }

        // The type of the method's return value.
        const ReturnType& ReturnType() const&
        {
            return m_returnType;
        }

        // For generic methods, the number of generic parameters.
        std::optional<size_t> GenericParameters() const
        {
            return m_genericParameters;
        }

        // The syntax variation of this signature.
        MethodSignatureKind Kind() const
        {
            return m_kind;
        }

        // For methods accepting variable amount of
        // parameters, describes how many required and
        // optional parameters are passed.
        std::optional<VarArgDescription> VarArg() const
        {
            return m_varArg;
        }

        // Returns an object capable of outputting method calling
        // convention and return type to standard streams.
        auto WritePreamble() const
        {
            return WritePreambleHolder(*this);
        }

        // Returns an object capable of outputting
        // method parameters to standard streams.
        auto WriteParameters() const
        {
            return WriteParametersHolder(*this);
        }
    };

    // Provides ability to output method calling convention
    // to standard streams.
    // @param target : the stream to output to.
    // @param callingConvention : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const MethodCallingConvention callingConvention)
    {
        switch (callingConvention)
        {
        case MethodCallingConvention::C:
            target << "c";
            break;
        case MethodCallingConvention::FastCall:
            target << "fastcall";
            break;
        case MethodCallingConvention::StdCall:
            target << "stdcall";
            break;
        case MethodCallingConvention::ThisCall:
            target << "thiscall";
        }

        return target;
    }

    // Provides ability to output usage of "this"
    // by a method to standard streams.
    // @param target : the stream to output to.
    // @param thisUsage : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const MethodThisUsage thisUsage)
    {
        switch (thisUsage)
        {
        case MethodThisUsage::ExplicitThis:
            target << "explicit this";
            break;
        case MethodThisUsage::NoThis:
            target << "static";
            break;
        }

        return target;
    }

    // Provides ability to output one custom modifier
    // to standard streams.
    // @param target : the stream to output to.
    // @param customMod : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const CustomMod& customMod)
    {
        target
            << "["
            << (customMod.Required ? "required" : "optional")
            << " custom modifier: "
            << HexOutput(customMod.TypeToken)
            << "]";
        return target;
    }

    // Provides ability to output several custom modifiers
    // to standard streams.
    // @param target : the stream to output to.
    // @param customMods : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const std::vector<CustomMod>& customMods)
    {
        for (const auto& customMod : customMods)
        {
            target << customMod;
        }

        if (!customMods.empty())
        {
            target << " ";
        }

        return target;
    }

    // Provides ability to output PrimitiveType
    // to standard streams.
    // @param target : the stream to output to.
    // @param type : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const PrimitiveType& type)
    {
        switch (type.Type())
        {
        case CorElementType::ELEMENT_TYPE_BOOLEAN:
            target << "bool";
            break;
        case CorElementType::ELEMENT_TYPE_CHAR:
            target << "char";
            break;
        case CorElementType::ELEMENT_TYPE_I1:
            target << "sbyte";
            break;
        case CorElementType::ELEMENT_TYPE_U1:
            target << "byte";
            break;
        case CorElementType::ELEMENT_TYPE_I2:
            target << "short";
            break;
        case CorElementType::ELEMENT_TYPE_U2:
            target << "ushort";
            break;
        case CorElementType::ELEMENT_TYPE_I4:
            target << "int";
            break;
        case CorElementType::ELEMENT_TYPE_U4:
            target << "uint";
            break;
        case CorElementType::ELEMENT_TYPE_I8:
            target << "long";
            break;
        case CorElementType::ELEMENT_TYPE_U8:
            target << "ulong";
            break;
        case CorElementType::ELEMENT_TYPE_R4:
            target << "float";
            break;
        case CorElementType::ELEMENT_TYPE_R8:
            target << "double";
            break;
        case CorElementType::ELEMENT_TYPE_STRING:
            target << "string";
            break;
        case CorElementType::ELEMENT_TYPE_OBJECT:
            target << "object";
            break;
        case CorElementType::ELEMENT_TYPE_I:
            target << "System.IntPtr";
            break;
        case CorElementType::ELEMENT_TYPE_U:
            target << "System.UIntPtr";
            break;
        }

        return target;
    }

    // Provides ability to output ArrayShape
    // to standard streams.
    // @param target : the stream to output to.
    // @param shape : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ArrayShape& shape)
    {
        target << "[";
        bool first { true };
        for (decltype(shape.Rank) i { 0 }; i != shape.Rank; ++i)
        {
            if (!first)
            {
                target << ", ";
            }

            const bool hasLowerBound { i < shape.LowerBounds.size() };
            const bool hasSize { i < shape.Sizes.size() };

            if (hasLowerBound)
            {
                const auto lowerBound { shape.LowerBounds[i] };
                target << lowerBound << " ...";
                if (hasSize)
                {
                    target << ' ' << (lowerBound + static_cast<int32_t>(shape.Sizes[i]));
                }
            }
            else if (hasSize)
            {
                target << shape.Sizes[i];
            }

            first = false;
        }

        target << "]";

        return target;
    }

    // Provides ability to output ArrayType
    // to standard streams.
    // @param target : the stream to output to.
    // @param arrayType : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ArrayType& arrayType)
    {
        return target << arrayType.ElementType() << arrayType.Shape;
    }

    // Provides ability to output ClassType
    // to standard streams.
    // @param target : the stream to output to.
    // @param classType : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ClassType classType)
    {
        return target
            << "{ class "
            << HexOutput(classType.Class)
            << " }";
    }

    // Provides ability to output StructType
    // to standard streams.
    // @param target : the stream to output to.
    // @param structType : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const StructType structType)
    {
        return target
            << "{ struct "
            << HexOutput(structType.Struct)
            << " }";
    }

    // Provides ability to output FunctionPointerType
    // to standard streams.
    // @param target : the stream to output to.
    // @param functionPointer : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const FunctionPointerType& functionPointer)
    {
        return target
            << InRoundBrackets(functionPointer.TargetSignature())
            << "*";
    }

    // Provides ability to output GenericInstanceType
    // to standard streams.
    // @param target : the stream to output to.
    // @param closedGenericType : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const GenericInstanceType& closedGenericType)
    {
        if (closedGenericType.IsValueType)
        {
            target << StructType { closedGenericType.GenericType };
        }
        else
        {
            target << ClassType { closedGenericType.GenericType };
        }

        return target << '<' << Delimitered(closedGenericType.GenericParameters, ", ") << '>';
    }

    // Provides ability to output MethodGenericArgument
    // to standard streams.
    // @param target : the stream to output to.
    // @param genericArgument : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const MethodGenericArgument genericArgument)
    {
        return target << "MethodGenericArguments" << InSquareBrackets(genericArgument.Index);
    }

    // Provides ability to output TypeGenericArgument
    // to standard streams.
    // @param target : the stream to output to.
    // @param genericArgument : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const TypeGenericArgument genericArgument)
    {
        return target << "TypeGenericArguments" << InSquareBrackets(genericArgument.Index);
    }

    // Provides ability to output PointerType
    // to standard streams.
    // @param target : the stream to output to.
    // @param pointer : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const PointerType& pointer)
    {
        target << pointer.CustomMods;
        if (pointer.ValueType == nullptr)
        {
            target << "void";
        }
        else
        {
            target << *pointer.ValueType;
        }

        return target << "*";
    }

    // Provides ability to output ZeroBasedArrayType
    // to standard streams.
    // @param target : the stream to output to.
    // @param zeroBasedArray : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ZeroBasedArrayType& zeroBasedArray)
    {
        return target
            << zeroBasedArray.CustomMods
            << zeroBasedArray.ElementType()
            << "[]";
    }

    // Provides ability to output variable Type
    // to standard streams.
    // @param target : the stream to output to.
    // @param type : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const Type& type)
    {
        std::visit(
            [&target](const auto& typeOption)
            {
                target << typeOption;
            },
            type);

        return target;
    }

    // Provides ability to output ObsoleteParameterPassed
    // to standard streams.
    // @param target : the stream to output to.
    // @param obsoleteParameter : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ObsoleteParameterPassed obsoleteParameter)
    {
        return target << "System.TypedReference";
    }

    // Provides ability to output ModernParameterPassed
    // to standard streams.
    // @param target : the stream to output to.
    // @param modernParameter : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ModernParameterPassed& modernParameter)
    {
        if (modernParameter.IsPassedByReference)
        {
            target << "ref ";
        }

        return target << modernParameter.ParameterType;
    }

    // Provides ability to output ParameterType
    // to standard streams.
    // @param target : the stream to output to.
    // @param parameter : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ParameterType& parameter)
    {
        target << parameter.CustomMods;

        std::visit(
            [&target](const auto& pass)
            {
                target << pass;
            },
            parameter.PassDescription);

        return target;
    }

    // Provides ability to output ReturnType
    // to standard streams.
    // @param target : the stream to output to.
    // @param returnType : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const ReturnType& returnType)
    {
        target << returnType.CustomMods;
        if (!returnType.PassDescription.has_value())
        {
            return target << "void";
        }

        std::visit(
            [&target](const auto& pass)
            {
                target << pass;
            },
            *returnType.PassDescription);

        return target;
    }

    // Provides ability to output MethodSignature
    // to standard streams.
    // @param target : the stream to output to.
    // @param signature : the value to output.
    template <typename TChar>
    std::basic_ostream<TChar>& operator<<(
        std::basic_ostream<TChar>& target,
        const MethodSignature& signature)
    {
        return target
            << signature.WritePreamble()
            << signature.WriteParameters();
    }
}
