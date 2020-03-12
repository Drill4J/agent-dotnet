#include "pch.h"

#include "MethodHeader.h"

using namespace Drill4dotNet;

// Checks MethodHeader is created from a
// tiny method header bytes.
TEST(MethodHeaderTests, CreateFromSmallHeader)
{
    // Arrange
    const int codeSize{ 0x30 };
    // Header is 1 byte combining size and tiny header flag.
    const std::vector<std::byte> headerBytes { std::byte { 0xC2 } };

    // Act
    const MethodHeader header(headerBytes);
    std::vector<std::byte> serialized{};
    header.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(1, header.Size());
    EXPECT_EQ(codeSize, header.CodeSize());
    EXPECT_EQ(headerBytes, serialized);
    EXPECT_FALSE(header.HasExceptionsSections());
}

// Checks MethodHeader's constructor throws an
// std::runtime_error if the input sequence of
// bytes is empty.
TEST(MethodHeaderTests, CreateThrowsEmptyInput)
{
    // Arrange
    const std::vector<std::byte> headerBytes {};

    // Assert
    EXPECT_THROW(MethodHeader { headerBytes }, std::runtime_error);
}

// Checks that the SetCodeSize() updates the
// CodeSize() and this change is properly reflected
// in the tiny binary representation.
TEST(MethodHeaderTests, SetCodeSizeAffectsSmallHeader)
{
    // Arrange
    const int codeSize{ 0x30 };
    const int newCodeSize { 0x39 };
    // Header is 1 byte combining size and tiny header flag.
    const std::vector<std::byte> headerBytes { std::byte { 0xC2 } };
    const std::vector<std::byte> expectedSerialized { std::byte { 0xE6 } };

    // Act
    MethodHeader header(headerBytes);
    header.SetCodeSize(newCodeSize);
    std::vector<std::byte> serialized{};
    header.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(1, header.Size());
    EXPECT_EQ(newCodeSize, header.CodeSize());
    EXPECT_EQ(expectedSerialized, serialized);
    EXPECT_FALSE(header.HasExceptionsSections());
}

// Checks MethodHeader is created from a
// fat method header bytes.
TEST(MethodHeaderTests, CreateFromFatHeader)
{
    // Arrange
    const int codeSize { 0x5D18D4D7 };
    const std::vector<std::byte> headerBytes
    {
        std::byte { 0x1B },
        std::byte { 0x30 },
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0xD7 },
        std::byte { 0xD4 },
        std::byte { 0x18 },
        std::byte { 0x5D },
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x11 }
    };

    // Act
    const MethodHeader header(headerBytes);
    std::vector<std::byte> serialized{};
    header.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(12, header.Size());
    EXPECT_EQ(codeSize, header.CodeSize());
    EXPECT_EQ(headerBytes, serialized);
    EXPECT_TRUE(header.HasExceptionsSections());
}

// Checks MethodHeader's constructor throws an
// std::runtime_error if the input sequence of
// bytes ends unexpectedly.
TEST(MethodHeaderTests, CreateThrowsInputUnexpectedEnd)
{
    // Arrange
    // This header meant to be 12 bytes long, but
    // has 5 bytes only
    const std::vector<std::byte> headerBytes
    {
        std::byte { 0x1B },
        std::byte { 0x30 },
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0xD7 }
    };

    // Assert
    EXPECT_THROW(MethodHeader{ headerBytes }, std::runtime_error);
}

// Checks that if we got method header with
// some extra data, this data is saved and then
// appended to the fat binary representation.
TEST(MethodHeaderTests, CreateFromFatHeaderBiggerThanUsual)
{
    // Arrange
    const std::vector<std::byte> headerBytes
    {
        std::byte { 0x1B },
        std::byte { 0x40 },
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0xD7 },
        std::byte { 0xD4 },
        std::byte { 0x18 },
        std::byte { 0x5D },
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x11 },
        std::byte { 0xFF },
        std::byte { 0xFF },
        std::byte { 0xFF },
        std::byte { 0xFF }
    };

    // Act
    const MethodHeader header(headerBytes);
    std::vector<std::byte> serialized{};
    header.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(16, header.Size());
    EXPECT_EQ(headerBytes, serialized);
}

// Checks that the SetCodeSize() updates the
// CodeSize() and this change is properly reflected
// in the fat binary representation.
TEST(MethodHeaderTests, SetCodeSizeAffectsFatHeader)
{
    // Arrange
    const int codeSize { 0x5D18D4D7 };
    const int newCodeSize { 0x5D19A959 };
    const std::vector<std::byte> headerBytes
    {
        std::byte { 0x1B },
        std::byte { 0x30 },
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0xD7 },
        std::byte { 0xD4 },
        std::byte { 0x18 },
        std::byte { 0x5D },
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x11 }
    };

    const std::vector<std::byte> expectedSerialized
    {
        std::byte { 0x1B },
        std::byte { 0x30 },
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0x59 },
        std::byte { 0xA9 },
        std::byte { 0x19 },
        std::byte { 0x5D },
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x11 }
    };

    // Act
    MethodHeader header(headerBytes);
    header.SetCodeSize(newCodeSize);
    std::vector<std::byte> serialized{};
    header.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(newCodeSize, header.CodeSize());
    EXPECT_TRUE(header.HasExceptionsSections());
    EXPECT_EQ(expectedSerialized, serialized);
}

static_assert(MethodHeader::IsValidCodeSize(0x38));
static_assert(MethodHeader::IsValidCodeSize(0x8D1C9D5E));
static_assert(!MethodHeader::IsValidCodeSize(0x000000018D1C9D5E));
static_assert(!MethodHeader::IsValidCodeSize(-1));
