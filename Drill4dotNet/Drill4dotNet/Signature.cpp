#include "pch.h"
#include "Signature.h"

#include <algorithm>
#include <bitset>

namespace Drill4dotNet
{
    static constexpr int32_t ReEncodeTwosComplement(
        const int32_t value,
        const uint32_t sourceBits,
        const uint32_t resultBits)
    {
        std::bitset<32> valueBits(static_cast<uint32_t>(value));
        const bool isPositive { std::as_const(valueBits)[sourceBits - 1] == false };
        if (isPositive)
        {
            return value;
        }

        const auto [startFlip, endFlip] { std::minmax(sourceBits, resultBits) };
        for (size_t i { startFlip }; i != endFlip; ++i)
        {
            valueBits.flip(i);
        }

        return static_cast<int32_t>(valueBits.to_ulong());
    }

    template <typename TUnsignedInteger>
    static constexpr TUnsignedInteger RotateLeft(
        const TUnsignedInteger value,
        const uint32_t width,
        const uint32_t bits)
    {
        return ~(0xFFFF'FFFF << width) & ((value << bits) | (value >> (width - bits)));
    }

    template <typename TUnsignedInteger>
    static constexpr TUnsignedInteger RotateRight(
        const TUnsignedInteger value,
        const uint32_t width,
        const uint32_t bits)
    {
        return ~(0xFFFF'FFFF << width) & ((value >> bits) | (value << (width - bits)));
    }

    static const std::byte s_TwoByteEncodingMarker { 0x80 };
    static const std::byte s_FourByteEncodingMarker{ 0xC0 };

    std::vector<std::byte> CompressSignatureInteger(const int32_t signedValue)
    {
        if (-(1 << 6) <= signedValue && signedValue <= (1 << 6) - 1)
        {
            const uint8_t value { RotateLeft(
                static_cast<uint8_t>(ReEncodeTwosComplement(
                    signedValue,
                    32,
                    7)),
                7,
                1) };

            return {
                std::byte { value }
            };
        }
        else if (-(1 << 13) <= signedValue && signedValue <= (1 << 13) - 1)
        {
            const uint16_t value { RotateLeft(
                static_cast<uint16_t>(ReEncodeTwosComplement(
                    signedValue,
                    32,
                    14)),
                14,
                1) };

            return {
                s_TwoByteEncodingMarker | std::byte { value >> 8 },
                std::byte { value }
            };
        }
        else if (MinCompressedSignatureInteger <= signedValue && signedValue <= MaxCompressedSignatureInteger)
        {
            const uint32_t value { RotateLeft(
                static_cast<uint32_t>(ReEncodeTwosComplement(
                    signedValue,
                    32,
                    29)),
                29,
                1) };

            return {
                s_FourByteEncodingMarker | std::byte { value >> 24 },
                std::byte { value >> 16 },
                std::byte { value >> 8 },
                std::byte { value }
            };
        }
        else
        {
            throw std::range_error("The given integer cannot be encoded to be stored in a signature.");
        }
    }

    std::vector<std::byte> CompressSignatureInteger(const uint32_t unsignedValue)
    {
        if (unsignedValue <= 0x7F)
        {
            return {
                std::byte { unsignedValue }
            };
        }
        else if (0x80 <= unsignedValue && unsignedValue <= 0x3FFF)
        {
            return {
                s_TwoByteEncodingMarker | std::byte { unsignedValue >> 8 },
                std::byte { unsignedValue }
            };
        }
        else if (unsignedValue <= MaxCompressedSignatureUnsignedInteger)
        {
            return {
                s_FourByteEncodingMarker | std::byte { unsignedValue >> 24 },
                std::byte { unsignedValue >> 16 },
                std::byte { unsignedValue >> 8 },
                std::byte { unsignedValue }
            };
        }
        else
        {
            throw std::range_error("The given integer cannot be encoded to be stored in a signature.");
        }
    }

    class DecomplessIntegerCoreResult
    {
    public:
        uint32_t Value;
        uint32_t MeaningfulBits;
        size_t BytesTaken;
    };

    DecomplessIntegerCoreResult DecomplessIntegerCore(
        const std::vector<std::byte>::const_iterator integerPosition,
        const std::vector<std::byte>::const_iterator sourceEnd)
    {

        if (integerPosition == sourceEnd)
        {
            throw std::runtime_error("Encoded integer value was expected");
        }

        const std::byte firstByte { *integerPosition };
        const size_t size { (firstByte & s_FourByteEncodingMarker) == s_FourByteEncodingMarker
            ? size_t { 4 }
            : ((firstByte & s_TwoByteEncodingMarker) == s_TwoByteEncodingMarker
                ? size_t { 2 }
                : 1) };
        if (sourceEnd - integerPosition < size)
        {
            throw std::runtime_error("Encoded integer value ended expectedly");
        }

        if (size == 1)
        {
            return {
                static_cast<uint32_t>(firstByte),
                7,
                size
            };
        }

        std::vector<std::byte>::const_iterator position { integerPosition };
        ++position;
        const std::byte secondByte { *position };
        if (size == 2)
        {
            return {
                (static_cast<uint32_t>(firstByte & ~s_TwoByteEncodingMarker) << 8)
                    | static_cast<uint32_t>(secondByte),
                14,
                size
            };
        }

        ++position;
        const std::byte thirdByte { *position };

        ++position;
        const std::byte forthByte { *position };

        return {
            (static_cast<uint32_t>(firstByte & ~s_FourByteEncodingMarker) << 24)
                | (static_cast<uint32_t>(secondByte) << 16)
                | (static_cast<uint32_t>(thirdByte) << 8)
                | static_cast<uint32_t>(forthByte),
            29,
            size
        };
    }

    ParseResult<int32_t> DecompressSignatureSignedInteger(
        const std::vector<std::byte>::const_iterator integerPosition,
        const std::vector<std::byte>::const_iterator sourceEnd)
    {
        const auto result = DecomplessIntegerCore(integerPosition, sourceEnd);
        return {
            result.BytesTaken,
            ReEncodeTwosComplement(
                static_cast<int32_t>(
                    RotateRight(
                        result.Value,
                        result.MeaningfulBits,
                        1)),
                result.MeaningfulBits,
                32)
        };
    }

    ParseResult<uint32_t> DecompressSignatureUnsignedInteger(
        const std::vector<std::byte>::const_iterator integerPosition,
        const std::vector<std::byte>::const_iterator sourceEnd)
    {
        const auto result = DecomplessIntegerCore(integerPosition, sourceEnd);
        return {
            result.BytesTaken,
            result.Value
        };
    }

    MethodSignature::MethodSignature(
        const MethodSignatureKind kind,
        const MethodThisUsage thisUsage,
        const MethodCallingConvention callingConvention,
        std::vector<ParameterType> parameterTypes,
        Drill4dotNet::ReturnType returnType,
        std::optional<size_t> genericParameters,
        std::optional<VarArgDescription> varArg)
        : m_kind { kind },
        m_thisUsage { thisUsage },
        m_callingConvention { callingConvention }
    {
        if (kind != MethodSignatureKind::FreeStandingSignature
            && callingConvention != MethodCallingConvention::Default)
        {
            throw std::logic_error("Native calling conventions are only allowed for Free Standing signatures");
        }

        if (callingConvention != MethodCallingConvention::Default
            && genericParameters.has_value())
        {
            throw std::logic_error("Method with unmanaged calling convention cannot be generic");
        }

        if (kind == MethodSignatureKind::MethodInSameAssembly
            && varArg.has_value()
            && varArg->OptionalParametersCount != 0)
        {
            throw std::logic_error("Method in the same assembly cannot have optional parameters info");
        }

        if (m_genericParameters.has_value()
            && *m_genericParameters > MaxCompressedSignatureUnsignedInteger)
        {
            throw std::runtime_error("The signature contains too many generic parameters");
        }

        if (parameterTypes.size() > MaxCompressedSignatureUnsignedInteger)
        {
            throw std::runtime_error("The signature contains too many parameters");
        }

        m_parameterTypes = std::move(parameterTypes);
        m_returnType = std::move(returnType);
        m_genericParameters = std::move(genericParameters);
        m_varArg = std::move(varArg);
    }

    ParseResult<MethodSignature> MethodSignature::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        if (position == end)
        {
            throw std::runtime_error("MethodSignature bytes must not be empty");
        }

        std::optional<MethodSignatureKind> kind {};
        MethodThisUsage thisUsage;
        const CorCallingConvention flags {
            static_cast<CorCallingConvention>(*position) };
        if ((flags & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS) != 0)
        {
            thisUsage = MethodThisUsage::ExplicitThis;
        }
        else if ((flags & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0)
        {
            thisUsage = MethodThisUsage::This;
        }
        else
        {
            thisUsage = MethodThisUsage::NoThis;
        }

        MethodCallingConvention callingConvention;
        const bool isVarArg { (flags & IMAGE_CEE_CS_CALLCONV_VARARG) != 0 };
        const bool isGeneric { (flags & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0 };
        if ((flags & IMAGE_CEE_CS_CALLCONV_C) != 0)
        {
            callingConvention = MethodCallingConvention::C;
            kind = MethodSignatureKind::FreeStandingSignature;
        }
        else if ((flags & IMAGE_CEE_CS_CALLCONV_STDCALL) != 0)
        {
            callingConvention = MethodCallingConvention::StdCall;
            kind = MethodSignatureKind::FreeStandingSignature;
        }
        else if ((flags & IMAGE_CEE_CS_CALLCONV_THISCALL) != 0)
        {
            callingConvention = MethodCallingConvention::ThisCall;
            kind = MethodSignatureKind::FreeStandingSignature;
        }
        else if ((flags & IMAGE_CEE_CS_CALLCONV_FASTCALL) != 0)
        {
            callingConvention = MethodCallingConvention::FastCall;
            kind = MethodSignatureKind::FreeStandingSignature;
        }
        else
        {
            callingConvention = MethodCallingConvention::Default;
        }

        std::optional<size_t> genericParameters{};
        size_t current { 1 };
        if (isGeneric)
        {
            if (kind.has_value())
            {
                throw std::runtime_error("Method with unmanaged calling convention cannot be generic");
            }

            const auto genericParametersCount {
                DecompressSignatureUnsignedInteger(
                    position + current,
                    end) };

            genericParameters = genericParametersCount.ParsedValue;
            current += genericParametersCount.BytesTaken;
        }

        const auto parametersCount{
            DecompressSignatureUnsignedInteger(
                position + current,
                end) };
        current += parametersCount.BytesTaken;

        // parse RetType
        auto returnType { ReturnType::Parse(position + current, end) };
        current += returnType.BytesTaken;

        // parse Parameters
        std::optional<VarArgDescription> varArg{};
        std::vector<ParameterType> parameterTypes{};
        parameterTypes.reserve(parametersCount.ParsedValue);
        for (uint32_t i { 0 }; i != parametersCount.ParsedValue; ++i)
        {
            if (isVarArg
                && position + current != end
                && *(position + current) == std::byte { CorElementType::ELEMENT_TYPE_SENTINEL })
            {
                if (kind.has_value() && *kind == MethodSignatureKind::MethodInSameAssembly)
                {
                    throw std::runtime_error("Method in the same assembly cannot have optional parameters info");
                }

                if (!kind.has_value())
                {
                    kind = MethodSignatureKind::MethodInDifferentAssembly;
                }

                varArg = VarArgDescription {
                    i,
                    parametersCount.ParsedValue - i };
                ++current;
            }

            auto parameterType { ParameterType::Parse(position + current, end) };
            current += parameterType.BytesTaken;
            parameterTypes.push_back(std::move(parameterType.ParsedValue));
        }

        if (isVarArg)
        {
            varArg = VarArgDescription { parametersCount.ParsedValue, 0 };
        }

        if (!kind.has_value())
        {
            kind = MethodSignatureKind::MethodInSameAssembly;
        }

        return { current,
            {
                *kind,
                thisUsage,
                callingConvention,
                std::move(parameterTypes),
                std::move(returnType.ParsedValue),
                genericParameters,
                varArg } };
    }

    void MethodSignature::AppendToBytes(std::vector<std::byte>& target) const
    {
        std::byte firstByte;
        switch (m_thisUsage)
        {
        case MethodThisUsage::NoThis:
            firstByte = std::byte { 0 };
            break;
        case MethodThisUsage::This:
            firstByte = std::byte { IMAGE_CEE_CS_CALLCONV_HASTHIS };
            break;
        case MethodThisUsage::ExplicitThis:
            firstByte = std::byte { IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS };
            break;
        default:
            throw std::logic_error("Must handle this variant of this usage");
        }

        switch (m_callingConvention)
        {
        case MethodCallingConvention::Default:
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_DEFAULT };
            break;
        case MethodCallingConvention::C:
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_C };
            break;
        case MethodCallingConvention::StdCall:
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_STDCALL };
            break;
        case MethodCallingConvention::FastCall:
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_FASTCALL };
            break;
        case MethodCallingConvention::ThisCall:
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_THISCALL };
            break;
        }

        if (m_varArg.has_value())
        {
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_VARARG };
        }

        if (m_genericParameters.has_value())
        {
            firstByte |= std::byte { IMAGE_CEE_CS_CALLCONV_GENERIC };
        }

        target.push_back(firstByte);

        if (m_genericParameters.has_value())
        {
            const std::vector<std::byte> genericParametersCount {
                CompressSignatureInteger(static_cast<uint32_t>(*m_genericParameters)) };
            std::copy(
                genericParametersCount.cbegin(),
                genericParametersCount.cend(),
                std::back_inserter(target));
        }

        const std::vector<std::byte> parametersCount {
            CompressSignatureInteger(static_cast<uint32_t>(m_parameterTypes.size())) };
        std::copy(
            parametersCount.cbegin(),
            parametersCount.cend(),
            std::back_inserter(target));

        m_returnType.AppendToBytes(target);

        for (size_t i { 0 }; i != m_parameterTypes.size(); ++i)
        {
            if (m_varArg.has_value()
                && i == m_varArg->RequiredParametersCount
                && m_varArg->OptionalParametersCount == 0)
            {
                target.push_back(std::byte { CorElementType::ELEMENT_TYPE_SENTINEL });
            }

            m_parameterTypes[i].AppendToBytes(target);
        }
    }

    ParseResult<ArrayShape> ArrayShape::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        std::vector<std::byte>::const_iterator current { position };
        size_t bytesTaken { 0 };
        const auto rank { DecompressSignatureUnsignedInteger(current, end) };
        current += rank.BytesTaken;
        bytesTaken += rank.BytesTaken;

        ArrayShape result{};
        result.Rank = rank.ParsedValue;

        const auto numSizes { DecompressSignatureUnsignedInteger(current, end) };
        current += numSizes.BytesTaken;
        bytesTaken += numSizes.BytesTaken;

        for (uint32_t i { 0 }; i != numSizes.ParsedValue; ++i)
        {
            const auto size { DecompressSignatureUnsignedInteger(current, end) };
            current += size.BytesTaken;
            bytesTaken += size.BytesTaken;
            result.Sizes.push_back(size.ParsedValue);
        }

        const auto numLowerbounds { DecompressSignatureUnsignedInteger(current, end) };
        current += numLowerbounds.BytesTaken;
        bytesTaken += numLowerbounds.BytesTaken;

        for (uint32_t i { 0 }; i != numLowerbounds.ParsedValue; ++i)
        {
            const auto lowerbound { DecompressSignatureSignedInteger(current, end) };
            current += lowerbound.BytesTaken;
            bytesTaken += lowerbound.BytesTaken;
            result.LowerBounds.push_back(lowerbound.ParsedValue);
        }

        return { bytesTaken, result };
    }

    void ArrayShape::AppendToBytes(std::vector<std::byte>& target) const
    {
        const std::vector<std::byte> rankBytes { CompressSignatureInteger(Rank) };
        target.insert(
            target.cend(),
            rankBytes.cbegin(),
            rankBytes.cend());

        if (Sizes.size() > MaxCompressedSignatureUnsignedInteger)
        {
            throw std::runtime_error("The Sizes vector contains too much elements to store in an ArrayShape");
        }

        const std::vector<std::byte> numSizesBytes { CompressSignatureInteger(static_cast<uint32_t>(Sizes.size())) };
        target.insert(
            target.cend(),
            numSizesBytes.cbegin(),
            numSizesBytes.cend());

        for (const auto& size : Sizes)
        {
            const std::vector<std::byte> sizeBytes { CompressSignatureInteger(size) };
            target.insert(
                target.cend(),
                sizeBytes.cbegin(),
                sizeBytes.cend());
        }

        if (LowerBounds.size() > MaxCompressedSignatureUnsignedInteger)
        {
            throw std::runtime_error("The LowerBounds vector contains too much elements to store in an ArrayShape");
        }

        const std::vector<std::byte> lowerBoundsBytes { CompressSignatureInteger(static_cast<uint32_t>(LowerBounds.size())) };
        target.insert(
            target.cend(),
            lowerBoundsBytes.cbegin(),
            lowerBoundsBytes.cend());

        for (const auto& bound : LowerBounds)
        {
            const std::vector<std::byte> boundBytes { CompressSignatureInteger(bound) };
            target.insert(
                target.cend(),
                boundBytes.cbegin(),
                boundBytes.cend());
        }
    }

    ParseResult<mdToken> DecompressTypeDefOrRefOrSpecEncoded(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        const auto intermediate { DecompressSignatureUnsignedInteger(position, end) };
        mdToken kind;
        switch (intermediate.ParsedValue & 0b11)
        {
        case 0b00:
            kind = mdtTypeDef;
            break;
        case 0b01:
            kind = mdtTypeRef;
            break;
        case 0b10:
            kind = mdtTypeSpec;
            break;
        default:
            throw std::runtime_error("The TypeDefOrRefOrSpecEncoded has invalid token type flag");
        }

        return { intermediate.BytesTaken, kind | (intermediate.ParsedValue >> 2) };
    }

    void CompressTypeDefOrRefOrSpecEncoded(
        const mdToken typeToken,
        std::vector<std::byte>& target)
    {
        uint32_t intermediate;
        switch (typeToken & 0xFF00'0000)
        {
        case mdtTypeDef:
            intermediate = 0;
            break;
        case mdtTypeRef:
            intermediate = 1;
            break;
        case mdtTypeSpec:
            intermediate = 1;
            break;
        default:
            throw std::logic_error("This type of token is not expected here");
        }

        intermediate |= ((typeToken & 0x00FF'FFFF) << 2);
        const std::vector<std::byte> tokenBytes { CompressSignatureInteger(intermediate) };
        target.insert(
            target.cend(),
            tokenBytes.cbegin(),
            tokenBytes.cend());
    }

    std::optional<ParseResult<CustomMod>> CustomMod::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        if (position == end)
        {
            return std::nullopt;
        }

        const std::byte requiredByte { *position };
        bool required;
        switch (requiredByte)
        {
        case std::byte{ ELEMENT_TYPE_CMOD_OPT }:
            required = false;
            break;
        case std::byte{ ELEMENT_TYPE_CMOD_REQD }:
            required = true;
            break;
        default:
            return std::nullopt;
        };

        const auto typeToken = DecompressTypeDefOrRefOrSpecEncoded(position + 1, end);
        ParseResult<CustomMod> result;
        result.BytesTaken = typeToken.BytesTaken + 1;
        result.ParsedValue = CustomMod { required, typeToken.ParsedValue };
        return result;
    }

    void CustomMod::AppendToBytes(std::vector<std::byte>& target) const
    {
        target.push_back(Required
            ? std::byte { ELEMENT_TYPE_CMOD_REQD }
            : std::byte { ELEMENT_TYPE_CMOD_OPT });

        CompressTypeDefOrRefOrSpecEncoded(TypeToken, target);
    }

    ParseResult<std::vector<CustomMod>> CustomMod::ParseSeveral(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        size_t bytesTaken { 0 };
        std::vector<CustomMod> result{};
        while (true)
        {
            auto customMod { CustomMod::Parse(position + bytesTaken, end) };
            if (!customMod.has_value())
            {
                return { bytesTaken, std::move(result) };
            }

            bytesTaken += customMod->BytesTaken;
            result.push_back(std::move(customMod->ParsedValue));
        }
    }

    void CustomMod::AppendSeveral(
        const std::vector<CustomMod> customMods,
        std::vector<std::byte>& target)
    {
        for (const auto& customMod : customMods)
        {
            customMod.AppendToBytes(target);
        }
    }

    ParseResult<ArrayType> ArrayType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        auto typeValue { ParseType(position, end) };
        auto arrayShape { ArrayShape::Parse(position + typeValue.BytesTaken, end) };
        return {
            typeValue.BytesTaken + arrayShape.BytesTaken,
            { std::move(typeValue.ParsedValue), std::move(arrayShape.ParsedValue) } };
    }

    void ArrayType::AppendToBytes(std::vector<std::byte>& target) const
    {
        AppendTypeToBytes(ElementType(), target);
        Shape.AppendToBytes(target);
    }

    ParseResult<ClassType> ClassType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        const auto classToken { DecompressTypeDefOrRefOrSpecEncoded(position, end) };
        return { classToken.BytesTaken, { classToken.ParsedValue } };
    }

    void ClassType::AppendToBytes(std::vector<std::byte>& target) const
    {
        CompressTypeDefOrRefOrSpecEncoded(Class, target);
    }

    ParseResult<StructType> StructType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        const auto structToken { DecompressTypeDefOrRefOrSpecEncoded(position, end) };
        return { structToken.BytesTaken, { structToken.ParsedValue } };
    }

    void StructType::AppendToBytes(std::vector<std::byte>& target) const
    {
        CompressTypeDefOrRefOrSpecEncoded(Struct, target);
    }

    ParseResult<FunctionPointerType> FunctionPointerType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        auto signatureValue { MethodSignature::Parse(position, end) };
        return { signatureValue.BytesTaken, { std::move(signatureValue.ParsedValue) } };
    }

    void FunctionPointerType::AppendToBytes(std::vector<std::byte>& target) const
    {
        // Check that only MethodRefSig or MethodDefSig is used
        const MethodSignatureKind kind { m_targetSignature->Kind() };
        if (kind != MethodSignatureKind::MethodInSameAssembly
            && kind != MethodSignatureKind::MethodInDifferentAssembly)
        {
            throw std::logic_error("Free standing signatures are not allowed in Type definitions.");
        }

        m_targetSignature->AppendToBytes(target);
    }

    ParseResult<GenericInstanceType> GenericInstanceType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        if (position == end)
        {
            throw std::runtime_error("Generic type instance definition is expected");
        }

        const bool isValueType { static_cast<CorElementType>(*position)
            == CorElementType::ELEMENT_TYPE_VALUETYPE };
        size_t bytesTaken { 1 };

        const auto genericType { DecompressTypeDefOrRefOrSpecEncoded(position + bytesTaken, end) };
        bytesTaken += genericType.BytesTaken;
        const auto genericParametersCount { DecompressSignatureUnsignedInteger(position + bytesTaken, end) };
        bytesTaken += genericParametersCount.BytesTaken;

        std::vector<Type> genericParameters{};
        genericParameters.reserve(genericParametersCount.ParsedValue);
        for (uint32_t i { 0 }; i != genericParametersCount.ParsedValue; ++i)
        {
            auto genericParameterValue { ParseType(position + bytesTaken, end) };
            bytesTaken += genericParameterValue.BytesTaken;
            genericParameters.push_back(std::move(genericParameterValue.ParsedValue));
        }

        return { bytesTaken, {
            isValueType,
            genericType.ParsedValue,
            std::move(genericParameters) } };
    }

    void GenericInstanceType::AppendToBytes(std::vector<std::byte>& target) const
    {
        target.push_back(IsValueType
            ? std::byte { CorElementType::ELEMENT_TYPE_VALUETYPE }
            : std::byte { CorElementType::ELEMENT_TYPE_CLASS });

        CompressTypeDefOrRefOrSpecEncoded(GenericType, target);
        if (GenericParameters.size() > MaxCompressedSignatureUnsignedInteger)
        {
            throw std::runtime_error("The signature contains too many generic parameters to be stored");
        }

        const std::vector<std::byte> parametersCount { CompressSignatureInteger(static_cast<uint32_t>(GenericParameters.size())) };
        std::copy(
            parametersCount.cbegin(),
            parametersCount.cend(),
            std::back_inserter(target));

        for (const auto& genericParameter : GenericParameters)
        {
            AppendTypeToBytes(genericParameter, target);
        }
    }

    ParseResult<MethodGenericArgument> MethodGenericArgument::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        const auto result { DecompressSignatureUnsignedInteger(position, end) };
        return { result.BytesTaken, { result.ParsedValue } };
    }

    void MethodGenericArgument::AppendToBytes(std::vector<std::byte>& target) const
    {
        const std::vector<std::byte> compressed { CompressSignatureInteger(Index) };
        std::copy(
            compressed.cbegin(),
            compressed.cend(),
            std::back_inserter(target));
    }

    ParseResult<TypeGenericArgument> TypeGenericArgument::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        const auto result { DecompressSignatureUnsignedInteger(position, end) };
        return { result.BytesTaken, { result.ParsedValue } };
    }

    void TypeGenericArgument::AppendToBytes(std::vector<std::byte>& target) const
    {
        const std::vector<std::byte> compressed { CompressSignatureInteger(Index) };
        std::copy(
            compressed.cbegin(),
            compressed.cend(),
            std::back_inserter(target));
    }

    ParseResult<PointerType> PointerType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        auto customMods { CustomMod::ParseSeveral(position, end) };
        const auto current = position + customMods.BytesTaken;
        if (current == end)
        {
            throw std::runtime_error("Underlying type for pointer is expected");
        }

        if (*current == std::byte { CorElementType::ELEMENT_TYPE_VOID })
        {
            return {
                customMods.BytesTaken + 1,
                {
                    std::unique_ptr<Type>{},
                    std::move(customMods.ParsedValue) } };
        }

        auto valueType { ParseType(current, end) };
        return {
            customMods.BytesTaken + valueType.BytesTaken,
            {
                std::make_unique<Type>(std::move(valueType.ParsedValue)),
                std::move(customMods.ParsedValue) } };
    }

    void PointerType::AppendToBytes(std::vector<std::byte>& target) const
    {
        CustomMod::AppendSeveral(CustomMods, target);
        if (ValueType == nullptr)
        {
            target.push_back(std::byte { CorElementType::ELEMENT_TYPE_VOID });
        }
        else
        {
            AppendTypeToBytes(*ValueType, target);
        }
    }

    ParseResult<ZeroBasedArrayType> ZeroBasedArrayType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        auto customMods { CustomMod::ParseSeveral(position, end) };
        const auto current = position + customMods.BytesTaken;
        auto elementType { ParseType(current, end) };
        return {
            customMods.BytesTaken + elementType.BytesTaken,
            {
                std::move(elementType.ParsedValue),
                std::move(customMods.ParsedValue) } };

    }

    void ZeroBasedArrayType::AppendToBytes(std::vector<std::byte>& target) const
    {
        CustomMod::AppendSeveral(CustomMods, target);
        AppendTypeToBytes(*m_elementType, target);
    }

    ParseResult<Type> ParseType(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        if (position == end)
        {
            throw std::runtime_error("Type is expected here");
        }

        const CorElementType type { static_cast<CorElementType>(*position) };
        const auto next { position + 1 };
        Type result { PrimitiveType { CorElementType::ELEMENT_TYPE_OBJECT } };
        size_t bytesTaken { 1 };
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
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_STRING:
        {
            result = PrimitiveType { type };
        }
        break;

        case ELEMENT_TYPE_PTR:
        {
            auto pointer { PointerType::Parse(next, end) };
            bytesTaken += pointer.BytesTaken;

            result = std::move(pointer.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_VALUETYPE:
        {
            auto structType { StructType::Parse(next, end) };
            bytesTaken += structType.BytesTaken;

            result = std::move(structType.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_CLASS:
        {
            auto classType { ClassType::Parse(next, end) };
            bytesTaken += classType.BytesTaken;

            result = std::move(classType.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_VAR:
        {
            auto var { TypeGenericArgument::Parse(next, end) };
            bytesTaken += var.BytesTaken;

            result = std::move(var.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_ARRAY:
        {
            auto arrayType { ArrayType::Parse(next, end) };
            bytesTaken += arrayType.BytesTaken;

            result = std::move(arrayType.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_GENERICINST:
        {
            auto genericInstance { GenericInstanceType::Parse(next, end) };
            bytesTaken += genericInstance.BytesTaken;

            result = std::move(genericInstance.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_FNPTR:
        {
            auto functionPointer { FunctionPointerType::Parse(next, end) };
            bytesTaken += functionPointer.BytesTaken;

            result = std::move(functionPointer.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_SZARRAY:
        {
            auto arrayType { ZeroBasedArrayType::Parse(next, end) };
            bytesTaken += arrayType.BytesTaken;

            result = std::move(arrayType.ParsedValue);
        }
        break;

        case ELEMENT_TYPE_MVAR:
        {
            auto var { MethodGenericArgument::Parse(next, end) };
            bytesTaken += var.BytesTaken;

            result = std::move(var.ParsedValue);
        }
        break;

        default:
            throw std::runtime_error("Unexpected discriminator for Type");
        }

        return { bytesTaken, std::move(result) };
    }

    void AppendTypeToBytes(const Type& type, std::vector<std::byte>& target)
    {
        std::visit(
            [&target](const auto& option)
            {
                using T = std::decay_t<decltype(option)>;
                if constexpr (std::is_same_v<T, PrimitiveType>)
                {
                    target.push_back(std::byte { option.Type() });
                    return;
                }
                else if constexpr (std::is_same_v<T, ArrayType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_ARRAY });
                }
                else if constexpr (std::is_same_v<T, ClassType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_CLASS });
                }
                else if constexpr (std::is_same_v<T, StructType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_VALUETYPE });
                }
                else if constexpr (std::is_same_v<T, FunctionPointerType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_FNPTR });
                }
                else if constexpr (std::is_same_v<T, GenericInstanceType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_GENERICINST });
                }
                else if constexpr (std::is_same_v<T, MethodGenericArgument>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_MVAR });
                }
                else if constexpr (std::is_same_v<T, TypeGenericArgument>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_VAR });
                }
                else if constexpr (std::is_same_v<T, PointerType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_PTR });
                }
                else if constexpr (std::is_same_v<T, ZeroBasedArrayType>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_SZARRAY });
                }
                else
                {
                    throw std::logic_error("Not ready to serialize this kind of Type");
                }

                if constexpr (!std::is_same_v<T, PrimitiveType>)
                {
                    option.AppendToBytes(target);
                }
            },
            type);
    }

    ParseResult<ParameterType> ParameterType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        auto returnType { ReturnType::Parse(position, end) };
        if (!returnType.ParsedValue.PassDescription.has_value())
        {
            throw std::runtime_error("Void is not allowed in parameter types");
        }

        return { returnType.BytesTaken,
            {
                std::move(returnType.ParsedValue.CustomMods),
                std::move(*returnType.ParsedValue.PassDescription)
            } };
    }

    void AppendPassDescriptionToBytes(
        const std::variant<ModernParameterPassed, ObsoleteParameterPassed>& passDescription,
        std::vector<std::byte>& target)
    {
        std::visit(
            [&target](const auto& parameter)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(parameter)>, ObsoleteParameterPassed>)
                {
                    target.push_back(std::byte { CorElementType::ELEMENT_TYPE_TYPEDBYREF });
                }
                else
                {
                    if (parameter.IsPassedByReference)
                    {
                        target.push_back(std::byte { CorElementType::ELEMENT_TYPE_BYREF });
                    }

                    AppendTypeToBytes(parameter.ParameterType, target);
                }
            },
            passDescription);
    }

    void ParameterType::AppendToBytes(std::vector<std::byte>& target) const
    {
        CustomMod::AppendSeveral(CustomMods, target);
        AppendPassDescriptionToBytes(PassDescription, target);
    }

    ParseResult<ReturnType> ReturnType::Parse(
        const std::vector<std::byte>::const_iterator position,
        const std::vector<std::byte>::const_iterator end)
    {
        auto customMods = CustomMod::ParseSeveral(position, end);
        size_t bytesTaken { customMods.BytesTaken };
        auto current = position + bytesTaken;
        if (current == end)
        {
            throw std::runtime_error("Parameter type was expected");
        }

        const std::byte firstByte { *(position + bytesTaken) };
        if (firstByte == std::byte { CorElementType::ELEMENT_TYPE_VOID })
        {
            return {
                bytesTaken + 1,
                {
                    std::move(customMods.ParsedValue),
                    std::nullopt } };
        }

        if (firstByte == std::byte { CorElementType::ELEMENT_TYPE_TYPEDBYREF })
        {
            return {
                bytesTaken + 1,
                {
                    std::move(customMods.ParsedValue),
                    { { ObsoleteParameterPassed {} } } } };
        }

        const bool isPassedByReference { firstByte == std::byte { CorElementType::ELEMENT_TYPE_BYREF } };
        if (isPassedByReference)
        {
            ++bytesTaken;
        }

        auto type { ParseType(position + bytesTaken, end) };
        return {
            bytesTaken + type.BytesTaken,
            {
                std::move(customMods.ParsedValue),
                { {
                    ModernParameterPassed {
                        std::move(type.ParsedValue),
                        isPassedByReference
                    }
                } }
            } };
    }

    void ReturnType::AppendToBytes(std::vector<std::byte>& target) const
    {
        CustomMod::AppendSeveral(CustomMods, target);
        if (!PassDescription.has_value())
        {
            target.push_back(std::byte { CorElementType::ELEMENT_TYPE_VOID });
        }
        else
        {
            AppendPassDescriptionToBytes(*PassDescription, target);
        }
    }

    void ArrayType::Assign(const ArrayType& other)
    {
        m_elementType = std::make_unique<Type>(*other.m_elementType);
        Shape = other.Shape;
    }

    void FunctionPointerType::Assign(const FunctionPointerType& other)
    {
        m_targetSignature = std::make_unique<MethodSignature>(*other.m_targetSignature);
    }

    void GenericInstanceType::Assign(const GenericInstanceType& other)
    {
        IsValueType = other.IsValueType;
        GenericType = other.GenericType;
        GenericParameters.clear();
        GenericParameters.reserve(other.GenericParameters.size());
        std::copy(
            other.GenericParameters.cbegin(),
            other.GenericParameters.cend(),
            std::back_inserter(GenericParameters));
    }

    void PointerType::Assign(const PointerType& other)
    {
        CustomMods = other.CustomMods;
        if (other.ValueType == nullptr)
        {
            ValueType = nullptr;
        }
        else
        {
            ValueType = std::make_unique<Type>(*other.ValueType);
        }
    }

    void ZeroBasedArrayType::Assign(const ZeroBasedArrayType& other)
    {
        CustomMods = other.CustomMods;
        m_elementType = std::make_unique<Type>(*other.m_elementType);
    }

    ArrayType::ArrayType(
        const Type& type,
        const ArrayShape& shape)
        : m_elementType { std::make_unique<Type>(type) },
        Shape{ shape }
    {
    }

    ArrayType::ArrayType(
        Type&& type,
        ArrayShape&& shape)
        : m_elementType { std::make_unique<Type>(type) },
        Shape { std::move(shape) }
    {
    }

    const Type& ArrayType::ElementType() const
    {
        return *m_elementType;
    }

    Type& ArrayType::ElementType()
    {
        return *m_elementType;
    }

    FunctionPointerType::FunctionPointerType(const MethodSignature& signature)
        : m_targetSignature { std::make_unique<MethodSignature>(signature) }
    {
    }

    FunctionPointerType::FunctionPointerType(MethodSignature&& signature)
        : m_targetSignature { std::make_unique<MethodSignature>(signature) }
    {
    }

    const MethodSignature& FunctionPointerType::TargetSignature() const
    {
        return *m_targetSignature;
    }

    MethodSignature& FunctionPointerType::TargetSignature()
    {
        return *m_targetSignature;
    }

    GenericInstanceType::GenericInstanceType(
        const bool isValueType,
        const mdToken genericType,
        const std::vector<Type>& genericParameters)
        : IsValueType { isValueType },
        GenericType { GenericType },
        GenericParameters { genericParameters }
    {
    }

    GenericInstanceType::GenericInstanceType(
        const bool isValueType,
        const mdToken genericType,
        std::vector<Type>&& genericParameters)
        : IsValueType { isValueType },
        GenericType { GenericType },
        GenericParameters { genericParameters }
    {
    }

    PointerType::PointerType(
        const std::unique_ptr<Type>& valueType,
        const std::vector<CustomMod>& customMods)
        : ValueType {},
        CustomMods { customMods }
    {
        if (valueType != nullptr)
        {
            ValueType = std::make_unique<Type>(*valueType);
        }
    }

    PointerType::PointerType(
        std::unique_ptr<Type>&& valueType,
        std::vector<CustomMod>&& customMods)
        : ValueType { std::move(valueType) },
        CustomMods { customMods }
    {
    }

    ZeroBasedArrayType::ZeroBasedArrayType(
        const Type& elementType,
        const std::vector<CustomMod>& customMods)
        : m_elementType { std::make_unique<Type>(elementType) },
        CustomMods { customMods }
    {
    }

    ZeroBasedArrayType::ZeroBasedArrayType(
        Type&& elementType,
        std::vector<CustomMod>&& customMods)
        : m_elementType { std::make_unique<Type>(std::move(elementType)) },
        CustomMods { customMods }
    {
    }

    const Type& ZeroBasedArrayType::ElementType() const
    {
        return *m_elementType;
    }


    Type& ZeroBasedArrayType::ElementType()
    {
        return *m_elementType;
    }
}
