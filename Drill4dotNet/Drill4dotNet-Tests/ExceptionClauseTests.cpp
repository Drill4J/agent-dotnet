#include "pch.h"

#include "ExceptionClause.h"

using namespace Drill4dotNet;

// Data required to create an ExceptionClause.
template <typename TRawClause>
class RawExceptionClause
{
public:
    TRawClause RawClause;
    InstructionStream Stream;
    LabelCreator LabelCreator;
};

// Common part of TryCatch, TryCatchWhen, and TryFinally methods.
// Returns representation of method
// int GetSum()
// {
//     int x = 1;
//     int y = 2;
//     int z;
//     try
//     {
//         z = x + y;
//     }
//     // catch or finally handler
//     {
//         z = 0;
//     }
//
//     return z;
// }
template <typename TRawClause>
static RawExceptionClause<TRawClause> RawClause()
{
    LabelCreator creator{};
    const Label exitLabel { creator.CreateLabel() };

    // all instructions are 1 byte
    InstructionStream stream {
        // int x = 1;
        OpCode_CEE_LDC_I4_1{},
        OpCode_CEE_STLOC_0{},

        // int y = 2;
        OpCode_CEE_LDC_I4_2{},
        OpCode_CEE_STLOC_1{},

        // will have try { here

        // z = x + y;
        OpCode_CEE_LDLOC_0{},
        OpCode_CEE_LDLOC_1{},
        OpCode_CEE_ADD{},
        OpCode_CEE_STLOC_2{},

        OpCode_CEE_LEAVE{ exitLabel },
        // will have
        // } // end of try
        // /* begin of handler */
        // {

        // z = 0;
        OpCode_CEE_LDC_I4_0{},
        OpCode_CEE_STLOC_2{},
        OpCode_CEE_LEAVE_S{exitLabel},
        // here
        // Will have } here

        // return z;
        exitLabel,
        OpCode_CEE_RET{}
    };

    TRawClause rawClause;
    rawClause.TryOffset = 4;
    rawClause.TryLength = 9;
    rawClause.HandlerOffset = 13;
    rawClause.HandlerLength = 4;

    return { rawClause, stream, creator };
}

// Asserts the instructions stream has the given
// label at the given postion, and the label has the given id.
static void AssertLabelAt(
    const Label label,
    const ConstStreamPosition position,
    const Label::Id expectedId)
{
    const Label* streamLabel = std::get_if<Label>(&*position);

    EXPECT_NE(nullptr, streamLabel);
    if (streamLabel == nullptr)
    {
        return;
    }

    EXPECT_EQ(expectedId, streamLabel->GetId());
    EXPECT_EQ(streamLabel->GetId(), label.GetId());
}

// Asserts that the given exception clause and
// instructions stream represent the exception handling
// structures returned by RawClause().
static void AssertRightLabelsAdded(const ExceptionClause& clause, const InstructionStream& stream)
{
    ASSERT_TRUE(stream.size() > 17);

    AssertLabelAt(
        clause.TryOffset(),
        stream.cbegin() + 4,
        1);

    AssertLabelAt(
        clause.TryEndOffset(),
        stream.cbegin() + 10,
        2);

    AssertLabelAt(
        clause.HandlerOffset(),
        stream.cend() - 7,
        3);

    AssertLabelAt(
        clause.HandlerEndOffset(),
        stream.cend() - 2,
        4);
}

// Checks that two given exception clauses have all same values.
template <typename TRawClause>
static void AssertRawClausesEqual(const TRawClause& expected, const TRawClause& actual)
{
    EXPECT_EQ(expected.Flags, actual.Flags);
    EXPECT_EQ(expected.TryOffset, actual.TryOffset);
    EXPECT_EQ(expected.TryLength, actual.TryLength);
    EXPECT_EQ(expected.HandlerOffset, actual.HandlerOffset);
    EXPECT_EQ(expected.HandlerLength, actual.HandlerLength);
}

// If TRawClause is IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT,
// asserts that the given clause cannot be put to a small header.
// Otherwise, asserts the clause can be put to a small header.
template <typename TRawClause>
static void AssertCanPutToSmallHeader(const ExceptionClause& clause)
{
    if constexpr (std::is_same_v<TRawClause, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>)
    {
        EXPECT_FALSE(clause.CanPutToSmallHeader());
    }
    else
    {
        EXPECT_TRUE(clause.CanPutToSmallHeader());
    }
}

// Returns representation of method
// int GetSum()
// {
//     int x = 1;
//     int y = 2;
//     int z;
//     try
//     {
//         z = x + y;
//     }
//     catch (Exception)
//     {
//         z = 0;
//     }
//
//     return z;
// }
template <typename TRawClause>
static RawExceptionClause<TRawClause> TryCatch()
{
    auto result = RawClause<TRawClause>();
    result.RawClause.Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    result.RawClause.ClassToken = 0x4589D7F3;
    return result;
}

// Returns representation of method
// int GetSum()
// {
//     int x = 1;
//     int y = 2;
//     int z;
//     try
//     {
//         z = x + y;
//     }
//     // Not valid C# or MSIL code, but does not matter
//     // for the exception clause test.
//     catch (Exception e) when e == 3
//     {
//         z = 0;
//     }
//
//     return z;
// }
template <typename TRawClause>
static RawExceptionClause<TRawClause> TryCatchWhen()
{
    auto result = RawClause<TRawClause>();
    result.Stream.insert(result.Stream.cbegin() + 9,
        {
            OpCode_CEE_LDLOC_S{4},
            OpCode_CEE_LDC_I4_3{},
            OpCode_CEE_CEQ{}
        });
    result.RawClause.Flags = COR_ILEXCEPTION_CLAUSE_FILTER;
    result.RawClause.FilterOffset = result.RawClause.HandlerOffset;
    result.RawClause.HandlerOffset += 5;
    return result;
}

// Returns representation of method
// int GetSum()
// {
//     int x = 1;
//     int y = 2;
//     int z;
//     try
//     {
//         z = x + y;
//     }
//     finally
//     {
//         z = 0;
//     }
//
//     return z;
// }
template <typename TRawClause>
static RawExceptionClause<TRawClause> TryFinally()
{
    auto result = RawClause<TRawClause>();
    result.RawClause.Flags = COR_ILEXCEPTION_CLAUSE_FINALLY;
    result.RawClause.ClassToken = 0;
    return result;
}

// Checks that ExceptionClause with try..catch is
// created from a small or fat header.
template <typename TRawClause>
static void CreateTryCatch()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryCatch<TRawClause>();

    // Act
    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Assert
    EXPECT_EQ(18, stream.size());
    AssertRightLabelsAdded(clause, stream);
    AssertCanPutToSmallHeader<TRawClause>(clause);
    EXPECT_FALSE(clause.IsFinally());
    EXPECT_FALSE(std::holds_alternative<Label>(clause.Handler()));
    ASSERT_TRUE(std::holds_alternative<mdTypeDef>(clause.Handler()));
    EXPECT_EQ(rawClause.ClassToken, std::get<mdTypeDef>(clause.Handler()));
}

// Checks that ExceptionClause with try..catch with when
// is created from a small or fat header.
template <typename TRawClause>
static void CreateTryCatchWhen()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryCatchWhen<TRawClause>();

    // Act
    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Assert
    EXPECT_EQ(22, stream.size());
    AssertRightLabelsAdded(clause, stream);
    AssertCanPutToSmallHeader<TRawClause>(clause);
    EXPECT_FALSE(clause.IsFinally());
    EXPECT_FALSE(std::holds_alternative<mdTypeDef>(clause.Handler()));
    ASSERT_TRUE(std::holds_alternative<Label>(clause.Handler()));
    AssertLabelAt(
        std::get<Label>(clause.Handler()),
        stream.cbegin() + 11,
        5);
}

// Checks that ExceptionClause with try..finally
// is created from a small or fat header.
template <typename TRawClause>
static void CreateTryFinally()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryFinally<TRawClause>();

    // Act
    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Assert
    EXPECT_EQ(18, stream.size());
    AssertRightLabelsAdded(clause, stream);
    AssertCanPutToSmallHeader<TRawClause>(clause);
    EXPECT_TRUE(clause.IsFinally());
    EXPECT_FALSE(std::holds_alternative<Label>(clause.Handler()));
    ASSERT_TRUE(std::holds_alternative<mdTypeDef>(clause.Handler()));
    EXPECT_EQ(rawClause.ClassToken, std::get<mdTypeDef>(clause.Handler()));
}

// Calls FillSmallHeader() or FillFatHeader() for the given
// clause, depending on the argument header type.
template <typename TRawClause>
static TRawClause FillRawClause(const ExceptionClause& clause)
{
    if constexpr (std::is_same_v<TRawClause, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>)
    {
        return clause.FillSmallHeader();
    }
    else
    {
        return clause.FillFatHeader();
    }
}

// Checks that ExceptionClause representing try..catch is
// converted back to fat or small header.
template <typename TRawClause>
static void FillRawClauseTryCatch()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryCatch<TRawClause>();
    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Act
    TRawClause serialized{ FillRawClause<TRawClause>(clause) };

    // Assert
    AssertRawClausesEqual(rawClause, serialized);
    EXPECT_EQ(rawClause.ClassToken, serialized.ClassToken);
}

// Checks that ExceptionClause representing try..catch with when is
// converted back to fat or small header.
template <typename TRawClause>
static void FillRawClauseTryCatchWhen()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryCatchWhen<TRawClause>();
    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Act
    TRawClause serialized{ FillRawClause<TRawClause>(clause) };

    // Assert
    AssertRawClausesEqual(rawClause, serialized);
    EXPECT_EQ(rawClause.FilterOffset, serialized.FilterOffset);
}

// Checks that ExceptionClause representing try..finally is
// converted back to fat or small header.
template <typename TRawClause>
static void FillRawClauseTryFinally()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryFinally<TRawClause>();
    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Act
    TRawClause serialized{ FillRawClause<TRawClause>(clause) };

    // Assert
    AssertRawClausesEqual(rawClause, serialized);
    EXPECT_EQ(rawClause.ClassToken, serialized.ClassToken);
}

// Checks that ExceptionClause with try..catch is
// created from a small header.
TEST(ExceptionClauseTests, CreateTryCatchFromSmallClause)
{
    CreateTryCatch<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
}

// Checks that ExceptionClause with try..catch with when
// is created from a small header.
TEST(ExceptionClauseTests, CreateTryCatchWhenFromSmallClause)
{
    CreateTryCatchWhen<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
}

// Checks that ExceptionClause with try..finally
// is created from a small header.
TEST(ExceptionClauseTests, CreateTryFinallyFromSmallClause)
{
    CreateTryFinally<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
}

// Checks that ExceptionClause with try..catch is
// created from a fat header.
TEST(ExceptionClauseTests, CreateTryCatchFromFatClause)
{
    CreateTryCatch<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
}

// Checks that ExceptionClause with try..catch with when
// is created from a fat header.
TEST(ExceptionClauseTests, CreateTryCatchWhenFromFatClause)
{
    CreateTryCatchWhen<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
}

// Checks that ExceptionClause with try..finally
// is created from a fat header.
TEST(ExceptionClauseTests, CreateTryFinallyFromFatClause)
{
    CreateTryFinally<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
}

// Checks that ExceptionClause representing try..catch is
// converted back to small header.
TEST(ExceptionClauseTests, FillSmallHeaderTryCatch)
{
    FillRawClauseTryCatch<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
}

// Checks that ExceptionClause representing try..catch with when is
// converted back to small header.
TEST(ExceptionClauseTests, FillSmallHeaderTryCatchWhen)
{
    FillRawClauseTryCatchWhen<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
}

// Checks that ExceptionClause representing try..finally is
// converted back to small header.
TEST(ExceptionClauseTests, FillSmallHeaderTryFinally)
{
    FillRawClauseTryFinally<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
}

// Checks that ExceptionClause representing try..catch is
// converted back to fat header.
TEST(ExceptionClauseTests, FillFatHeaderTryCatch)
{
    FillRawClauseTryCatch<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
}

// Checks that ExceptionClause representing try..catch with when is
// converted back to fat header.
TEST(ExceptionClauseTests, FillFatHeaderTryCatchWhen)
{
    FillRawClauseTryCatchWhen<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
}

// Checks that ExceptionClause representing try..finally is
// converted back to fat header.
TEST(ExceptionClauseTests, FillFatHeaderTryFinally)
{
    FillRawClauseTryFinally<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>();
}

// Checks that the ExceptionClause representing try..catch,
// which was created from a small header, cannot be stored
// back to a small header after inserting the given
// amount of No Operation instructions at the given position.
template<size_t paddingSize, ptrdiff_t paddingPosition>
static void CanPutToSmallHeaderFalse()
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryCatch<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();

    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Act
    stream.insert(
        stream.cbegin() + paddingPosition,
        paddingSize,
        OpCode_CEE_NOP{});

    // Assert
    EXPECT_FALSE(clause.CanPutToSmallHeader());
}

// Checks that the ExceptionClause representing try..catch,
// which was created from a small header, cannot be stored
// back to a small header after inserting a lot of No Operation
// instructions before the beginning of try.
TEST(ExceptionClauseTests, CanPutToSmallHeaderFarTry)
{
    CanPutToSmallHeaderFalse<0x00010000, 4>();
}

// Checks that the ExceptionClause representing try..catch with when,
// which was created from a small header, cannot be stored
// back to a small header after inserting a lot of No Operation
// instructions in the middle of the when block, in case of when
// block is between try and catch blocks.
TEST(ExceptionClauseTests, CanPutToSmallHeaderFarCatch)
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = TryCatchWhen<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();

    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Act
    size_t padding { 0x00010000 };
    stream.insert(
        stream.cbegin() + 13,
        padding,
        OpCode_CEE_NOP{});

    // Assert
    EXPECT_FALSE(clause.CanPutToSmallHeader());
}

// Checks that the ExceptionClause representing try..catch,
// which was created from a small header, cannot be stored
// back to a small header after inserting a lot of No Operation
// instructions in the middle of the try block.
TEST(ExceptionClauseTests, CanPutToSmallHeaderBigTry)
{
    CanPutToSmallHeaderFalse<0x0100, 9>();
}

// Checks that the ExceptionClause representing try..catch,
// which was created from a small header, cannot be stored
// back to a small header after inserting a lot of No Operation
// instructions in the middle of the catch block.
TEST(ExceptionClauseTests, CanPutToSmallHeaderBigCatch)
{
    CanPutToSmallHeaderFalse<0x0100, 13>();
}

// Checks that the ExceptionClause representing try..catch with when,
// which was created from a small header, can be stored
// back to a small header after inserting a lot of No Operation
// instructions in the middle of the when block, in case of when
// block is after try and catch blocks.
TEST(ExceptionClauseTests, CanPutToSmallHeaderFarWhen)
{
    // Arrange
    auto [rawClause, stream, labelCreator]
        = RawClause<IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>();
    stream.insert(stream.cend(),
        {
            OpCode_CEE_LDLOC_S{4},
            OpCode_CEE_LDC_I4_3{},
            OpCode_CEE_CEQ{}
        });
    rawClause.Flags = COR_ILEXCEPTION_CLAUSE_FILTER;
    rawClause.FilterOffset = 18;

    ExceptionClause clause(
        rawClause,
        stream,
        labelCreator);

    // Act
    const size_t paddingSize { 0x00010000 };
    stream.insert(
        stream.cbegin() + 18,
        paddingSize,
        OpCode_CEE_NOP{});

    // Assert
    EXPECT_TRUE(clause.CanPutToSmallHeader());
}
