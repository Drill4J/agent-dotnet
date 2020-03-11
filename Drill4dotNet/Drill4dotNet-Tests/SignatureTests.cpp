#include "pch.h"

#include "Signature.h"
#include <algorithm>

using namespace Drill4dotNet;

static const std::vector<int32_t> s_SignedValues {
    3,
    -3,
    64,
    -64,
    8'192,
    -8'192,
    268'435'455,
    -268'435'456
};

static const std::vector<std::vector<std::byte>> s_SignedBytes {
    {
        std::byte { 0x06 }
    },
    {
        std::byte { 0x7B }
    },
    {
        std::byte { 0x80 },
        std::byte { 0x80 }
    },
    {
        std::byte { 0x01 }
    },
    {
        std::byte { 0xC0 },
        std::byte { 0x00 },
        std::byte { 0x40 },
        std::byte { 0x00 }
    },
    {
        std::byte { 0x80 },
        std::byte { 0x01 }
    },
    {
        std::byte { 0xDF },
        std::byte { 0xFF },
        std::byte { 0xFF },
        std::byte { 0xFE }
    },
    {
        std::byte { 0xC0 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x01 }
    }
};

static const std::vector<uint32_t> s_UnsignedValues {
    0x03,
    0x7F,
    0x80,
    0x2E57,
    0x3FFF,
    0x4000,
    0x1FFF'FFFF
};

static const std::vector<std::vector<std::byte>> s_UnsignedBytes {
    {
        std::byte { 0x03 }
    },
    {
        std::byte { 0x7F }
    },
    {
        std::byte { 0x80 },
        std::byte { 0x80 }
    },
    {
        std::byte { 0xAE },
        std::byte { 0x57 }
    },
    {
        std::byte { 0xBF },
        std::byte { 0xFF }
    },
    {
        std::byte { 0xC0 },
        std::byte { 0x00 },
        std::byte { 0x40 },
        std::byte { 0x00 }
    },
    {
        std::byte { 0xDF },
        std::byte { 0xFF },
        std::byte { 0xFF },
        std::byte { 0xFF }
    }
};

TEST(SignatureTests, CompressSignatureIntegerSigned)
{
    // Arrange
    const std::vector<int32_t>& inputs { s_SignedValues };
    const std::vector<std::vector<std::byte>>& expectedResults { s_SignedBytes };

    // Act
    std::vector results(inputs.size(), std::vector<std::byte>{});
    std::transform(
        inputs.cbegin(),
        inputs.cend(),
        results.begin(),
        [](const int32_t input)
        {
            return CompressSignatureInteger(input);
        });

    // Assert
    EXPECT_EQ(expectedResults, results);
}

TEST(SignatureTests, CompressSignatureIntegerUnsigned)
{
    // Arrange
    const std::vector<uint32_t>& inputs { s_UnsignedValues };
    const std::vector<std::vector<std::byte>>& expectedResults { s_UnsignedBytes };

    // Act
    std::vector results(inputs.size(), std::vector<std::byte>{});
    std::transform(
        inputs.cbegin(),
        inputs.cend(),
        results.begin(),
        [](const uint32_t input)
        {
            return CompressSignatureInteger(input);
        });

    // Assert
    EXPECT_EQ(expectedResults, results);
}

TEST(SignatureTests, DecompressSignatureSignedInteger)
{
    // Arrange
    const std::vector<std::vector<std::byte>>& inputs { s_SignedBytes };

    const std::vector<ParseResult<int32_t>> expectedResults {
        { inputs[0].size(), s_SignedValues[0] },
        { inputs[1].size(), s_SignedValues[1] },
        { inputs[2].size(), s_SignedValues[2] },
        { inputs[3].size(), s_SignedValues[3] },
        { inputs[4].size(), s_SignedValues[4] },
        { inputs[5].size(), s_SignedValues[5] },
        { inputs[6].size(), s_SignedValues[6] },
        { inputs[7].size(), s_SignedValues[7] }
    };

    // Act
    std::vector<ParseResult<int32_t>> results{};
    std::transform(
        inputs.cbegin(),
        inputs.cend(),
        std::back_inserter(results),
        [](const std::vector<std::byte>& input)
        {
            return DecompressSignatureSignedInteger(input.cbegin(), input.cend());
        });

    // Assert
    EXPECT_EQ(expectedResults, results);
}

TEST(SignatureTests, DecompressSignatureUnsignedInteger)
{
    // Arrange
    const std::vector<std::vector<std::byte>>& inputs { s_UnsignedBytes };

    const std::vector<ParseResult<uint32_t>> expectedResults{
        { inputs[0].size(), s_UnsignedValues[0] },
        { inputs[1].size(), s_UnsignedValues[1] },
        { inputs[2].size(), s_UnsignedValues[2] },
        { inputs[3].size(), s_UnsignedValues[3] },
        { inputs[4].size(), s_UnsignedValues[4] },
        { inputs[5].size(), s_UnsignedValues[5] },
        { inputs[6].size(), s_UnsignedValues[6] }
    };

    // Act
    std::vector<ParseResult<uint32_t>> results{};
    std::transform(
        inputs.cbegin(),
        inputs.cend(),
        std::back_inserter(results),
        [](const std::vector<std::byte>& input)
    {
        return DecompressSignatureUnsignedInteger(input.cbegin(), input.cend());
    });

    // Assert
    EXPECT_EQ(expectedResults, results);
}
