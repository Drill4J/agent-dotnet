#include "pch.h"

#include "InstructionStream.h"

using namespace Drill4dotNet;

TEST(InstructionStreamTests, FindInstruction)
{
    // Arrange
    const InstructionStream stream { OpCode_CEE_BREAK{}, OpCode_CEE_ADD{} };

    // Act
    const ConstStreamPosition first { FindInstruction<OpCode_CEE_BREAK>(stream.cbegin(), stream.cend()) };
    const ConstStreamPosition second { FindInstruction<OpCode_CEE_ADD>(stream.cbegin(), stream.cend()) };
    const ConstStreamPosition notFound { FindInstruction<OpCode_CEE_NOP>(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin(), first);
    EXPECT_EQ(stream.cbegin() + 1, second);
    EXPECT_EQ(stream.cend(), notFound);
}

TEST(InstructionStreamTests, ResolveJumpOffsetZero)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP { });
    const ConstStreamPosition firstPosition { stream.cbegin() };
    const ConstStreamPosition secondPosition { stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition { stream.cend() - 1 };
    const LongJump::Offset offset { 0 };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, firstPosition, offset) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, secondPosition, offset) };
    const ConstStreamPosition third { ResolveJumpOffset(stream, thirdPosition, offset) };

    // Assert
    EXPECT_EQ(firstPosition + 1, first);
    EXPECT_EQ(secondPosition + 1, second);
    EXPECT_EQ(thirdPosition + 1, third);
}

TEST(InstructionStreamTests, ResolveJumpOffsetPositive)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP { });
    const ConstStreamPosition firstPosition { stream.cbegin() };
    const ConstStreamPosition secondPosition { stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition { stream.cend() - 1 };
    const LongJump::Offset offset { +5 };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, firstPosition, offset) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, secondPosition, offset) };
    const ConstStreamPosition third { ResolveJumpOffset(stream, thirdPosition, offset) };

    // Assert
    EXPECT_EQ(firstPosition + 6, first);
    EXPECT_EQ(stream.cend(), second);
    EXPECT_EQ(stream.cend(), third);
}

TEST(InstructionStreamTests, ResolveJumpOffsetNegative)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP { });
    const ConstStreamPosition firstPosition { stream.cbegin() };
    const ConstStreamPosition secondPosition { stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition { stream.cend() - 1 };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, firstPosition, -1) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, secondPosition, -3) };
    const ConstStreamPosition third { ResolveJumpOffset(stream, thirdPosition, -10) };

    // Assert
    EXPECT_EQ(firstPosition, first);
    EXPECT_EQ(stream.cbegin() + 3, second);
    EXPECT_EQ(stream.cbegin(), third);
}

TEST(InstructionStreamTests, ResolveJumpOffsetOutOfStream)
{
    // Arrange
    const InstructionStream stream { OpCode_CEE_NOP { } };
    const ConstStreamPosition position { stream.cbegin() };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, position, -10) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, position, +10) };

    // Assert
    EXPECT_EQ(stream.cend(), first);
    EXPECT_EQ(stream.cend(), second);
}

TEST(InstructionStreamTests, ResolveJumpOffsetEmptyStream)
{
    // Arrange
    const InstructionStream stream {};
    const ConstStreamPosition position { stream.cend() };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, position, -10) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, position, 0) };
    const ConstStreamPosition third { ResolveJumpOffset(stream, position, +10) };

    // Assert
    EXPECT_EQ(position, first);
    EXPECT_EQ(position, second);
    EXPECT_EQ(position, third);
}

TEST(InstructionStreamTests, ResolveJumpOffsetSkipsLabels)
{
    // Arrange
    LabelCreator creator{};
    const InstructionStream stream {
        creator.CreateLabel(),
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{}
    };

    const ConstStreamPosition position { stream.cbegin() + 2 };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, position, -2) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, position, -1) };
    const ConstStreamPosition third { ResolveJumpOffset(stream, position, 0) };
    const ConstStreamPosition forth { ResolveJumpOffset(stream, stream.cbegin(), 1) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 1, first);
    EXPECT_EQ(stream.cbegin() + 3, second);
    EXPECT_EQ(stream.cbegin() + 5, third);
    EXPECT_EQ(stream.cbegin() + 5, forth);
}

// In .net, a relative jump offset should point exactly to the
// position where target instruction begins. If ResolveJumpOffset
// was given an offset pointing to the middle of some instruction,
// it must return stream.cend() to indicate no target found.
// The test checks this.
TEST(InstructionStreamTests, ResolveJumpOffsetIgnoresMiddleOfInstruction)
{
    // Arrange
    const InstructionStream stream{
        OpCode_CEE_LDARG{0},
        OpCode_CEE_CEQ{},
        OpCode_CEE_LDC_I4{42}, 
        OpCode_CEE_BREAK{}
    };
    const ConstStreamPosition position { stream.cbegin() + 1 };

    // Act
    const ConstStreamPosition first { ResolveJumpOffset(stream, position, -3) };
    const ConstStreamPosition second { ResolveJumpOffset(stream, position, -1) };
    const ConstStreamPosition third { ResolveJumpOffset(stream, position, 1) };
    const ConstStreamPosition forth { ResolveJumpOffset(stream, position, 2) };

    // Assert
    EXPECT_EQ(stream.cend(), first);
    EXPECT_EQ(stream.cend(), second);
    EXPECT_EQ(stream.cend(), third);
    EXPECT_EQ(stream.cend(), forth);
}

TEST(InstructionStreamTests, ResolveAbsoluteOffsetEmptyStream)
{
    // Arrange
    const InstructionStream stream {};

    // Act
    const ConstStreamPosition first { ResolveAbsoluteOffset(stream, 0) };
    const ConstStreamPosition second { ResolveAbsoluteOffset(stream, 10) };

    // Assert
    EXPECT_EQ(stream.cend(), first);
    EXPECT_EQ(stream.cend(), second);
}

TEST(InstructionStreamTests, ResolveAbsoluteOffsetOutOfStream)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP { });

    // Act
    const ConstStreamPosition position { ResolveAbsoluteOffset(stream, 100) };

    // Assert
    EXPECT_EQ(stream.cend(), position);
}

TEST(InstructionStreamTests, ResolveAbsoluteOffset)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP { });
    AbsoluteOffset firstPosition { 0 };
    AbsoluteOffset secondPosition { 4 };
    AbsoluteOffset thirdPosition { 9 };


    // Act
    const ConstStreamPosition first{ ResolveAbsoluteOffset(stream, firstPosition) };
    const ConstStreamPosition second{ ResolveAbsoluteOffset(stream, secondPosition) };
    const ConstStreamPosition third{ ResolveAbsoluteOffset(stream, thirdPosition) };

    // Assert
    EXPECT_EQ(stream.cbegin() + firstPosition, first);
    EXPECT_EQ(stream.cbegin() + secondPosition, second);
    EXPECT_EQ(stream.cbegin() + thirdPosition, third);
}

template <typename TOpCode>
static void AssertHolds(const ConstStreamPosition position, const InstructionStream& stream)
{
    ASSERT_NE(position, stream.cend());
    const StreamElement& element = *position;
    ASSERT_TRUE(std::holds_alternative<OpCodeVariant>(element));
    const OpCodeVariant& opCode { std::get<OpCodeVariant>(element) };
    ASSERT_TRUE(opCode.HoldsAlternative<TOpCode>());
}

TEST(InstructionStreamTests, ResolveAbsoluteOffsetVariousInstructions)
{
    // Arrange
    const InstructionStream stream{
        OpCode_CEE_CEQ{},
        OpCode_CEE_LDC_I4{ 42 },
        OpCode_CEE_ADD{}
    };
    AbsoluteOffset firstPosition { 0 };
    AbsoluteOffset secondPosition { 2 };
    AbsoluteOffset thirdPosition { 7 };

    // Act
    const ConstStreamPosition first{ ResolveAbsoluteOffset(stream, firstPosition) };
    const ConstStreamPosition second{ ResolveAbsoluteOffset(stream, secondPosition) };
    const ConstStreamPosition third{ ResolveAbsoluteOffset(stream, thirdPosition) };

    // Assert
    AssertHolds<OpCode_CEE_CEQ>(first, stream);
    AssertHolds<OpCode_CEE_LDC_I4>(second, stream);
    AssertHolds<OpCode_CEE_ADD>(third, stream);
}

// In .net, an offset from the method beginning should point exactly to
// the position where an instruction begins. If ResolveAbsoluteOffset
// was given an offset pointing to the middle of some instruction,
// it must return stream.cend() to indicate no target found.
// The test checks this.
TEST(InstructionStreamTests, ResolveAbsoluteOffsetIgnoresMiddleOfInstruction)
{
    // Arrange
    const InstructionStream stream{
        OpCode_CEE_LDARG{0},
        OpCode_CEE_CEQ{},
        OpCode_CEE_LDC_I4{42},
        OpCode_CEE_BREAK{}
    };

    // Act
    const ConstStreamPosition first{ ResolveAbsoluteOffset(stream, 2) };
    const ConstStreamPosition second{ ResolveAbsoluteOffset(stream, 5) };
    const ConstStreamPosition third{ ResolveAbsoluteOffset(stream, 8) };

    // Assert
    EXPECT_EQ(stream.cend(), first);
    EXPECT_EQ(stream.cend(), second);
    EXPECT_EQ(stream.cend(), third);
}

TEST(InstructionStreamTests, GetNthInstruction)
{
    // Arrange
    LabelCreator creator{};
    const InstructionStream stream {
        creator.CreateLabel(),
        OpCode_CEE_LDARG{0},
        creator.CreateLabel(),
        creator.CreateLabel(),
        OpCode_CEE_CEQ{},
        creator.CreateLabel(),
        OpCode_CEE_LDC_I4{42},
        creator.CreateLabel(),
        OpCode_CEE_BREAK{} };

    // Act
    const ConstStreamPosition first{ GetNthInstruction(stream, 0) };
    const ConstStreamPosition second{ GetNthInstruction(stream, 1) };
    const ConstStreamPosition third{ GetNthInstruction(stream, 2) };
    const ConstStreamPosition forth{ GetNthInstruction(stream, 3) };

    // Assert
    AssertHolds<OpCode_CEE_LDARG>(first, stream);
    AssertHolds<OpCode_CEE_CEQ>(second, stream);
    AssertHolds<OpCode_CEE_LDC_I4>(third, stream);
    AssertHolds<OpCode_CEE_BREAK>(forth, stream);
}

TEST(InstructionStreamTests, CalculateJumpOffsetPositive)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP{ });
    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition{ stream.cend() - 1 };

    // Act
    const LongJump::Offset firstToSecond { CalculateJumpOffset(stream, firstPosition, secondPosition) };
    const LongJump::Offset firstToThird{ CalculateJumpOffset(stream, firstPosition, thirdPosition) };
    const LongJump::Offset secondToThird{ CalculateJumpOffset(stream, secondPosition, thirdPosition) };

    // Assert
    EXPECT_EQ(4, firstToSecond);
    EXPECT_EQ(8, firstToThird);
    EXPECT_EQ(3, secondToThird);
}

TEST(InstructionStreamTests, CalculateJumpOffsetAtSameInstruction)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP{ });
    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition{ stream.cend() - 1 };

    // Act
    const LongJump::Offset first { CalculateJumpOffset(stream, firstPosition, firstPosition) };
    const LongJump::Offset second { CalculateJumpOffset(stream, secondPosition, secondPosition) };
    const LongJump::Offset third { CalculateJumpOffset(stream, thirdPosition, thirdPosition) };

    // Assert
    EXPECT_EQ(-1, first);
    EXPECT_EQ(-1, second);
    EXPECT_EQ(-1, third);
}

TEST(InstructionStreamTests, CalculateJumpOffsetAtNextInstruction)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP{ });
    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition{ stream.cend() - 1 };

    // Act
    const LongJump::Offset first { CalculateJumpOffset(stream, firstPosition, firstPosition + 1) };
    const LongJump::Offset second { CalculateJumpOffset(stream, secondPosition, secondPosition + 1) };
    const LongJump::Offset third { CalculateJumpOffset(stream, thirdPosition, thirdPosition + 1) };

    // Assert
    EXPECT_EQ(0, first);
    EXPECT_EQ(0, second);
    EXPECT_EQ(0, third);
}

TEST(InstructionStreamTests, CalculateJumpOffsetNegative)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP{ });
    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition{ stream.cend() - 1 };

    // Act
    const LongJump::Offset secondToFirst { CalculateJumpOffset(stream, secondPosition, firstPosition) };
    const LongJump::Offset thirdToSecond { CalculateJumpOffset(stream, thirdPosition, secondPosition) };
    const LongJump::Offset thirdToFirst { CalculateJumpOffset(stream, thirdPosition, firstPosition) };

    // Assert
    EXPECT_EQ(-6, secondToFirst);
    EXPECT_EQ(-5, thirdToSecond);
    EXPECT_EQ(-10, thirdToFirst);
}

TEST(InstructionStreamTests, CalculateJumpOffsetSkipsLabels)
{
    // Arrange
    LabelCreator creator{};
    const InstructionStream stream {
        creator.CreateLabel(),
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{}
    };

    const ConstStreamPosition first { stream.cbegin() };
    const ConstStreamPosition second { stream.cbegin() + 2 };
    const ConstStreamPosition third { stream.cbegin() + 4 };

    // Act
    const LongJump::Offset firstToSecond { CalculateJumpOffset(stream, first, second) };
    const LongJump::Offset firstToThird { CalculateJumpOffset(stream, first, third) };
    const LongJump::Offset thirdToFirst { CalculateJumpOffset(stream, third, first) };

    // Assert
    EXPECT_EQ(0, firstToSecond);
    EXPECT_EQ(1, firstToThird);
    EXPECT_EQ(-3, thirdToFirst);
}

TEST(InstructionStreamTests, CalculateJumpOffsetCalculatesSizesProperly)
{
    // Arrange
    const InstructionStream stream{
        OpCode_CEE_LDARG{0},
        OpCode_CEE_CEQ{},
        OpCode_CEE_LDC_I4{42},
        OpCode_CEE_BREAK{} };

    const ConstStreamPosition first { stream.cbegin() };
    const ConstStreamPosition second { stream.cbegin() + 1 };
    const ConstStreamPosition third { stream.cbegin() + 2 };
    const ConstStreamPosition forth { stream.cbegin() + 3 };

    // Act
    const LongJump::Offset firstToSecond { CalculateJumpOffset(stream, first, second) };
    const LongJump::Offset firstToThird { CalculateJumpOffset(stream, first, third) };
    const LongJump::Offset firstToForth { CalculateJumpOffset(stream, first, forth) };

    // Assert
    EXPECT_EQ(0, firstToSecond);
    EXPECT_EQ(2, firstToThird);
    EXPECT_EQ(7, firstToForth);
}

TEST(InstructionStreamTests, CalculateAbsoluteOffset)
{
    // Arrange
    const InstructionStream stream(10, OpCode_CEE_NOP{ });
    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 5 };
    const ConstStreamPosition thirdPosition{ stream.cend() - 1 };

    // Act
    const AbsoluteOffset first { CalculateAbsoluteOffset(stream, firstPosition) };
    const AbsoluteOffset second { CalculateAbsoluteOffset(stream, secondPosition) };
    const AbsoluteOffset third { CalculateAbsoluteOffset(stream, thirdPosition) };

    // Assert
    EXPECT_EQ(0, first);
    EXPECT_EQ(5, second);
    EXPECT_EQ(9, third);
}

TEST(InstructionStreamTests, CalculateAbsoluteOffsetSkipsLabels)
{
    // Arrange
    LabelCreator creator{};
    const InstructionStream stream {
        creator.CreateLabel(),
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{}
    };

    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 2 };
    const ConstStreamPosition thirdPosition{ stream.cbegin() + 4 };

    // Act
    const AbsoluteOffset first{ CalculateAbsoluteOffset(stream, firstPosition) };
    const AbsoluteOffset second{ CalculateAbsoluteOffset(stream, secondPosition) };
    const AbsoluteOffset third{ CalculateAbsoluteOffset(stream, thirdPosition) };

    // Assert
    EXPECT_EQ(0, first);
    EXPECT_EQ(1, second);
    EXPECT_EQ(2, third);
}

TEST(InstructionStreamTests, CalculateAbsoluteOffsetCalculatesSizesProperly)
{
    // Arrange
    const InstructionStream stream{
        OpCode_CEE_LDARG{0},
        OpCode_CEE_CEQ{},
        OpCode_CEE_LDC_I4{42},
        OpCode_CEE_BREAK{} };

    const ConstStreamPosition firstPosition{ stream.cbegin() };
    const ConstStreamPosition secondPosition{ stream.cbegin() + 1 };
    const ConstStreamPosition thirdPosition{ stream.cbegin() + 2 };
    const ConstStreamPosition forthPosition{ stream.cbegin() + 3 };

    // Act
    const AbsoluteOffset first{ CalculateAbsoluteOffset(stream, firstPosition) };
    const AbsoluteOffset second{ CalculateAbsoluteOffset(stream, secondPosition) };
    const AbsoluteOffset third{ CalculateAbsoluteOffset(stream, thirdPosition) };
    const AbsoluteOffset forth{ CalculateAbsoluteOffset(stream, forthPosition) };

    // Assert
    EXPECT_EQ(0, first);
    EXPECT_EQ(4, second);
    EXPECT_EQ(6, third);
    EXPECT_EQ(11, forth);
}

TEST(InstructionStreamTests, FindLabelInEmptyStream)
{
    // Arrange
    const InstructionStream stream{};
    LabelCreator creator{};
    const Label label{ creator.CreateLabel() };

    // Act
    const ConstStreamPosition labelPosition { FindLabel(stream, label) };

    // Assert
    EXPECT_EQ(stream.cend(), labelPosition);
}

TEST(InstructionStreamTests, FindLabelStreamWithLabel)
{
    // Arrange
    LabelCreator creator{};
    const Label label{ creator.CreateLabel() };
    const InstructionStream stream{ label };

    // Act
    const ConstStreamPosition labelPosition { FindLabel(stream, label) };

    // Assert
    EXPECT_EQ(stream.cbegin(), labelPosition);
}

TEST(InstructionStreamTests, FindLabelStreamWithOtherLabelOnly)
{
    // Arrange
    LabelCreator creator{};
    const Label otherLabel{ creator.CreateLabel() };
    const InstructionStream stream{ otherLabel };
    const Label label{ creator.CreateLabel() };

    // Act
    const ConstStreamPosition labelPosition { FindLabel(stream, label) };

    // Assert
    EXPECT_EQ(stream.cend(), labelPosition);
}

TEST(InstructionStreamTests, FindLabelStreamWithOtherLabel)
{
    // Arrange
    LabelCreator creator{};
    const Label otherLabel{ creator.CreateLabel() };
    const Label label{ creator.CreateLabel() };
    const InstructionStream stream{ otherLabel, label };

    // Act
    const ConstStreamPosition labelPosition { FindLabel(stream, label) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 1, labelPosition);
}

TEST(InstructionStreamTests, SkipLabelsInEmptyStream)
{
    // Arrange
    const InstructionStream stream{};

    // Act
    const ConstStreamPosition nearestInstruction { SkipLabels(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cend(), nearestInstruction);
}

TEST(InstructionStreamTests, SkipLabelsInStreamWithOneLabel)
{
    // Arrange
    LabelCreator creator;
    const InstructionStream stream{ creator.CreateLabel() };

    // Act
    const ConstStreamPosition nearestInstruction { SkipLabels(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cend(), nearestInstruction);
}

TEST(InstructionStreamTests, SkipLabelsInStreamWithOneInstruction)
{
    // Arrange
    const InstructionStream stream{ OpCode_CEE_NOP{} };

    // Act
    const ConstStreamPosition nearestInstruction { SkipLabels(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin(), nearestInstruction);
}

TEST(InstructionStreamTests, SkipLabelsInStreamWithLabelAndInstruction)
{
    // Arrange
    LabelCreator creator;
    const InstructionStream stream{ creator.CreateLabel(), OpCode_CEE_NOP{} };

    // Act
    const ConstStreamPosition nearestInstruction { SkipLabels(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 1, nearestInstruction);
}

TEST(InstructionStreamTests, SkipLabelsInStreamWithTwoLabelsAndInstruction)
{
    // Arrange
    LabelCreator creator;
    const InstructionStream stream{ creator.CreateLabel(), creator.CreateLabel(), OpCode_CEE_NOP{} };

    // Act
    const ConstStreamPosition nearestInstruction { SkipLabels(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 2, nearestInstruction);
}

TEST(InstructionStreamTests, FindNextInstructionInEmptyStream)
{
    // Arrange
    const InstructionStream stream{};

    // Act
    const ConstStreamPosition nextInstruction { FindNextInstruction(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cend(), nextInstruction);
}

TEST(InstructionStreamTests, FindNextInstructionInStreamWithOneInstruction)
{
    // Arrange
    const InstructionStream stream{ OpCode_CEE_NOP {} };

    // Act
    const ConstStreamPosition nextInstruction { FindNextInstruction(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cend(), nextInstruction);
}

TEST(InstructionStreamTests, FindNextInstructionInStreamWithOneInstructionAndOneLabel)
{
    // Arrange
    LabelCreator creator;
    const InstructionStream stream{ OpCode_CEE_NOP{}, creator.CreateLabel() };

    // Act
    const ConstStreamPosition nextInstruction{ FindNextInstruction(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cend(), nextInstruction);
}

TEST(InstructionStreamTests, FindNextInstructionInStreamWithTwoInstructionsAndOneLabel)
{
    // Arrange
    LabelCreator creator;
    const InstructionStream stream{
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        OpCode_CEE_NOP{}
    };

    // Act
    const ConstStreamPosition nextInstruction{ FindNextInstruction(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 2, nextInstruction);
}

TEST(InstructionStreamTests, FindNextInstructionInStreamWithTwoInstructions)
{
    // Arrange
    const InstructionStream stream{ OpCode_CEE_NOP{}, OpCode_CEE_NOP{} };

    // Act
    const ConstStreamPosition nextInstruction{ FindNextInstruction(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 1, nextInstruction);
}

TEST(InstructionStreamTests, FindNextInstructionInStreamWithTwoLabelsAndTwoInstructions)
{
    // Arrange
    LabelCreator creator;
    const InstructionStream stream{
        OpCode_CEE_NOP{},
        creator.CreateLabel(),
        creator.CreateLabel(),
        OpCode_CEE_NOP{} };

    // Act
    const ConstStreamPosition nextInstruction{ FindNextInstruction(stream.cbegin(), stream.cend()) };

    // Assert
    EXPECT_EQ(stream.cbegin() + 3, nextInstruction);
}

// Checks that subsequent calls of
// LabelCreator::CreateLabel() return
// different labels.
TEST(InstructionStreamTests, CreateLabelUnique)
{
    // Arrange
    LabelCreator creator{};

    // Act
    const Label first { creator.CreateLabel() };
    const Label second { creator.CreateLabel() };

    // Assert
    EXPECT_FALSE(first == second);
}
