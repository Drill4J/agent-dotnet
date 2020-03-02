#include "pch.h"

#include "ByteUtils.h"

using namespace Drill4dotNet;

TEST(ByteUtilsTests, AdvanceToBoundary)
{
    // Arrange
    constexpr size_t boundary{ 4 };
    const std::vector<std::byte> bytes(boundary * 2 + 3, std::byte{ 0 });
    const auto firstPosition{ bytes.cbegin() };
    const auto secondPosition{ bytes.cbegin() + boundary };
    const auto thirdPosition{ bytes.cbegin() + boundary + 1 };
    const auto forthPosition{ bytes.cend() - 1 };
    const auto fifthPosition{ bytes.cend() };

    // Act
    const ptrdiff_t first = AdvanceToBoundary<boundary>(firstPosition, bytes);
    const ptrdiff_t second = AdvanceToBoundary<boundary>(secondPosition, bytes);
    const ptrdiff_t third = AdvanceToBoundary<boundary>(thirdPosition, bytes);
    const ptrdiff_t forth = AdvanceToBoundary<boundary>(forthPosition, bytes);
    const ptrdiff_t fifth = AdvanceToBoundary<boundary>(fifthPosition, bytes);

    // Assert
    EXPECT_EQ(0, first);
    EXPECT_EQ(0, second);
    EXPECT_EQ(3, third);
    EXPECT_EQ(2, forth);
    EXPECT_EQ(1, fifth);
}

TEST(ByteUtilsTests, AdvanceToBoundaryEmptyVector)
{
    // Arrange
    constexpr size_t boundary{ 4 };
    const std::vector<std::byte> bytes{};
    const auto position{ bytes.cend() };

    // Act
    const ptrdiff_t mustAdvanceBy = AdvanceToBoundary<boundary>(position, bytes);

    // Assert
    EXPECT_EQ(0, mustAdvanceBy);
}

TEST(ByteUtilsTests, AppendAsBytes)
{
    // Arrange
    const int value = 0x0734BDB5;
    const std::vector<std::byte> expectedBytes
    {
        std::byte{ 0xB5 },
        std::byte{ 0xBD },
        std::byte{ 0x34 },
        std::byte{ 0x07 }
    };

    // Act
    std::vector<std::byte> bytes{};
    AppendAsBytes(bytes, value);

    // Assert
    EXPECT_EQ(expectedBytes, bytes);
}

TEST(ByteUtilsTests, Overflows)
{
    // Arrange
    const uint32_t tooLarge = 0xFD73EEF2;

    // Act
    const bool overflows = Overflows<int32_t>(tooLarge);

    // Assert
    EXPECT_TRUE(overflows);
}

static_assert(!Overflows<int8_t>(int8_t{ 0 }));
static_assert(!Overflows<int>(int{ 0 }));
static_assert(!Overflows<uint8_t>(uint8_t{ 0 }));
static_assert(!Overflows<uint32_t>(uint32_t{ 0 }));
static_assert(!Overflows<uint8_t>(int8_t{ 0 }));
static_assert(!Overflows<uint32_t>(int{ 0 }));
static_assert(!Overflows<int8_t>(uint8_t{ 0 }));
static_assert(!Overflows<int>(uint32_t{ 0 }));
static_assert(!Overflows<uint32_t>(uint8_t{ 0xC0 }));
static_assert(!Overflows<uint8_t>(uint32_t{ 0xC0 }));
static_assert(Overflows<uint8_t>(int8_t{ -1 }));
static_assert(Overflows<uint8_t>(int{ -1 }));
static_assert(Overflows<uint8_t>(uint32_t{ 0x0100 }));
static_assert(Overflows<int8_t>(int{ 0x0100 }));
static_assert(Overflows<int8_t>(int{ -0x0100 }));
