#include "pch.h"

#include "ExceptionsSection.h"

using namespace Drill4dotNet;

// Creates instructions stream containing of
// four No Operation instructions.
InstructionStream CreateStream()
{
    return InstructionStream(4, OpCode_CEE_NOP{});
}

// Creates a small exception clause header,
// compatible with instructions stream
// from CreateStream().
constexpr static auto CreateSmallClause()
{
    return std::array
    {
        // Flags
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryOffset
        std::byte { 0x01 },
        std::byte { 0x00 },

        // TryLength
        std::byte { 0x01 },

        // HandlerOffset
        std::byte { 0x02 },
        std::byte { 0x00 },

        // HandlerLength
        std::byte { 0x01 },

        // ClassToken
        std::byte { 0x10 },
        std::byte { 0x20 },
        std::byte { 0x40 },
        std::byte { 0x80 }
    };
}

// Creates a fat exception clause header,
// compatible with instructions stream
// from CreateStream().
constexpr static auto CreateFatClause()
{
    return std::array
    {
        // Flags
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryOffset
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryLength
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // HandlerOffset
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // HandlerLength
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // ClassToken
        std::byte { 0x10 },
        std::byte { 0x20 },
        std::byte { 0x40 },
        std::byte { 0x80 }
    };
}

// Concatenates several arrays of bytes into one vector
template <size_t ... sizes>
static std::vector<std::byte> Concat(const std::array<std::byte, sizes>& ... arrays)
{
    std::vector<std::byte> result{};
    (
        result.insert(
            result.cend(),
            arrays.cbegin(),
            arrays.cend())
        ,
        ...
    );
    return result;
}

// Checks that an ExceptionsSection is created from
// a small bytes representation and converted back to
// the same bytes.
TEST(ExceptionsSectionTests, CreateFromSmallClauses)
{
    // Arrange
    constexpr auto sectionHeader = std::array {
        std::byte { 0x01 },
        std::byte { 0x10 },
        std::byte { 0x00 },
        std::byte { 0x00 }
    };

    const std::vector<std::byte> sectionData { Concat(
        sectionHeader,
        CreateSmallClause()
    ) };

    InstructionStream stream { CreateStream() };
    LabelCreator labelCreator{};

    // Act
    std::vector<std::byte>::const_iterator iterator { sectionData.cbegin() };
    ExceptionsSection section(
        iterator,
        sectionData.cend(),
        stream,
        labelCreator);
    std::vector<std::byte> serialized{};
    section.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(1, section.Clauses().size());
    EXPECT_EQ(sectionData, serialized);
    EXPECT_EQ(sectionData.cend(), iterator);
    EXPECT_FALSE(section.HasMoreSections());
}

// Checks that an ExceptionsSection with MoreSections flag
// is created from a small bytes representation and converted
// back to the same bytes.
TEST(ExceptionsSectionTests, CreateFromSmallClausesWithMoreSections)
{
    // Arrange
    constexpr auto firstSectionHeader = std::array {
        std::byte { 0x81 },
        std::byte { 0x10 },
        std::byte { 0x00 },
        std::byte { 0x00 },
    };

    constexpr auto secondSectionHeader = std::array {
        std::byte{ 0x01 },
        std::byte{ 0x10 },
        std::byte{ 0x00 },
        std::byte{ 0x00 }
    };

    const std::vector<std::byte> expectedSerialized { Concat(
        firstSectionHeader,
        CreateSmallClause()
    ) };

    const std::vector<std::byte> sectionData { Concat(
        firstSectionHeader,
        CreateSmallClause(),
        secondSectionHeader,
        CreateSmallClause()
    ) };

    InstructionStream stream { CreateStream() };
    LabelCreator labelCreator{};

    // Act
    std::vector<std::byte>::const_iterator iterator { sectionData.cbegin() };
    ExceptionsSection section(
        iterator,
        sectionData.cend(),
        stream,
        labelCreator);
    std::vector<std::byte> serialized{};
    section.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(1, section.Clauses().size());
    EXPECT_EQ(expectedSerialized, serialized);
    EXPECT_EQ(sectionData.cbegin() + 0x10, iterator);
    EXPECT_TRUE(section.HasMoreSections());
}

// Checks that an ExceptionsSection is created from
// a fat bytes representation and converted back to
// the same bytes.
TEST(ExceptionsSectionTests, CreateFromFatClauses)
{
    // Arrange
    constexpr auto sectionHeader = std::array {
        std::byte { 0x41 },
        std::byte { 0x1C },
        std::byte { 0x00 },
        std::byte { 0x00 }
    };

    const std::vector<std::byte> sectionData { Concat(
        sectionHeader,
        CreateFatClause()
    ) };

    InstructionStream stream { CreateStream() };
    LabelCreator labelCreator{};

    // Act
    std::vector<std::byte>::const_iterator iterator { sectionData.cbegin() };
    ExceptionsSection section(
        iterator,
        sectionData.cend(),
        stream,
        labelCreator);
    std::vector<std::byte> serialized{};
    section.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(1, section.Clauses().size());
    EXPECT_EQ(sectionData, serialized);
    EXPECT_EQ(sectionData.cend(), iterator);
    EXPECT_FALSE(section.HasMoreSections());
}

// Checks that an ExceptionsSection with MoreSections flag
// is created from a fat bytes representation and converted
// back to the same bytes.
TEST(ExceptionsSectionTests, CreateFromFatClausesWithMoreSections)
{
    // Arrange
    constexpr auto firstSectionHeader = std::array {
        std::byte { 0xC1 },
        std::byte { 0x1C },
        std::byte { 0x00 },
        std::byte { 0x00 },
    };

    constexpr auto secondSectionHeader = std::array {
        std::byte{ 0x01 },
        std::byte{ 0x10 },
        std::byte{ 0x00 },
        std::byte{ 0x00 }
    };

    const std::vector<std::byte> expectedSerialized { Concat(
        firstSectionHeader,
        CreateFatClause()
    ) };

    const std::vector<std::byte> sectionData { Concat(
        firstSectionHeader,
        CreateFatClause(),
        secondSectionHeader,
        CreateSmallClause()
    ) };

    InstructionStream stream { CreateStream() };
    LabelCreator labelCreator{};

    // Act
    std::vector<std::byte>::const_iterator iterator { sectionData.cbegin() };
    ExceptionsSection section(
        iterator,
        sectionData.cend(),
        stream,
        labelCreator);
    std::vector<std::byte> serialized{};
    section.AppendToBytes(serialized);

    // Assert
    EXPECT_EQ(1, section.Clauses().size());
    EXPECT_EQ(expectedSerialized, serialized);
    EXPECT_EQ(sectionData.cbegin() + 0x1C, iterator);
    EXPECT_TRUE(section.HasMoreSections());
}

// Checks that exceptions section created from
// small bytes representation will convert to
// a fat bytes representation if one of the
// exception clauses cannot be stored into small
// form after injection.
TEST(ExceptionsSectionTests, ExtendSectionOfSmallClauses)
{
    // Arrange
    constexpr auto sectionHeader = std::array{
        std::byte { 0x01 },
        std::byte { 0x1C },
        std::byte { 0x00 },
        std::byte { 0x00 }
    };

    constexpr auto secondClause = std::array
    {
        // Flags
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryOffset
        std::byte { 0x05 },
        std::byte { 0x00 },

        // TryLength
        std::byte { 0x01 },

        // HandlerOffset
        std::byte { 0x06 },
        std::byte { 0x00 },

        // HandlerLength
        std::byte { 0x01 },

        // ClassToken
        std::byte { 0x10 },
        std::byte { 0x20 },
        std::byte { 0x40 },
        std::byte { 0x80 }
    };

    const std::vector<std::byte> sectionData { Concat(
        sectionHeader,
        CreateSmallClause(),
        secondClause
    ) };

    InstructionStream stream(8, OpCode_CEE_NOP{});
    LabelCreator labelCreator{};
    std::vector<std::byte>::const_iterator iterator{ sectionData.cbegin() };
    ExceptionsSection section(
        iterator,
        sectionData.cend(),
        stream,
        labelCreator);

    const std::vector<std::byte> expectedBytes
    {
        // Section header
        std::byte { 0x41 },
        std::byte { 0x34 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // First clause, fat
        // Flags
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryOffset
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryLength
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // HandlerOffset
        std::byte { 0x02 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // HandlerLength
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // ClassToken
        std::byte { 0x10 },
        std::byte { 0x20 },
        std::byte { 0x40 },
        std::byte { 0x80 },

        // Second clause, fat
        // Flags
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // TryOffset
        std::byte { 0x05 },
        std::byte { 0x00 },
        std::byte { 0x01 },
        std::byte { 0x00 },

        // TryLength
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // HandlerOffset
        std::byte { 0x06 },
        std::byte { 0x00 },
        std::byte { 0x01 },
        std::byte { 0x00 },

        // HandlerLength
        std::byte { 0x01 },
        std::byte { 0x00 },
        std::byte { 0x00 },
        std::byte { 0x00 },

        // ClassToken
        std::byte { 0x10 },
        std::byte { 0x20 },
        std::byte { 0x40 },
        std::byte { 0x80 }
    };

    // Act
    stream.insert(
        stream.cbegin() + 8,
        0x00010000,
        OpCode_CEE_NOP{});

    std::vector<std::byte> serialized{};
    section.AppendToBytes(serialized);

    // Assert
    ASSERT_EQ(2, section.Clauses().size());
    EXPECT_TRUE(section.Clauses()[0].CanPutToSmallHeader());
    EXPECT_FALSE(section.Clauses()[1].CanPutToSmallHeader());
    EXPECT_EQ(expectedBytes, serialized);
}
