#include "pch.h"

#include "MethodBody.h"

using namespace Drill4dotNet;

// Checks that an std::runtime_error is thrown if
// the method bytes end unexpectedly in the middle of
// the instructions stream.
TEST(MethodBodyTests, CreateThrowsOnUnexpectedEnd)
{
    // Arrange
    const std::vector methodBytes {
        std::byte { 0x12 },
        std::byte { 0x02 },
        std::byte { 0x03 }
        // In a valid method body, there should be 2 more bytes
    };

    // Assert
    EXPECT_THROW(
        MethodBody { methodBytes },
        std::runtime_error);
}

// Checks that an std::runtime_error is thrown if the
// method body bytes contain an unknown instruction code.
TEST(MethodBodyTests, CreateThrowsOnInvalidOpCode)
{
    // Arrange
    const std::vector methodBytes {
        std::byte { 0x12 },
        std::byte { 0x02 },
        std::byte { 0x03 },
        std::byte { 0xFE }, // There is no OpCode these
        std::byte { 0xFD }  // two bytes represent
    };

    // Assert
    EXPECT_THROW(
        MethodBody { methodBytes },
        std::runtime_error);
}

// Creates method body representing
// public static int Sum(int x, int y)
// {
//     return x + y;
// }
static std::vector<std::byte> CreateSimpleFunction()
{
    // Captured from compiled assembly
    return {
        std::byte { 0x12 },
        std::byte { 0x02 },
        std::byte { 0x03 },
        std::byte { 0x58 },
        std::byte { 0x2A }
    };
}

// Checks the following injection
// public static int Sum(int x, int y)
// {
//     return x + y;
// }
// Will replace
//     return x + y;
// with
//     return (x + y) * 2;
TEST(MethodBodyTests, InsertSimpleFunction)
{
    // Arrange
    const std::vector<std::byte> sourceBytes(CreateSimpleFunction());

    // if we do not inject anything, Compile() should return exactly
    // the same bytes from which MethodBody was created.
    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // based on source bytes
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x1A }, // size updated
        std::byte { 0x02 },
        std::byte { 0x03 },
        std::byte { 0x58 },
        std::byte { 0x18 }, // inject: ldc.i4.2
        std::byte { 0x5A }, // inject: mul
        std::byte { 0x2A }
    };

    // MethodBody should turn sourceBytes to this
    const InstructionStream expectedSourceStream{
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_RET{}
    };

    // MethodBody should have this instructions
    // stream after injection.
    const InstructionStream expectedInjectionStream{
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_LDC_I4_2{}, // injection
        OpCode::CEE_MUL{}, // injection
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 3, OpCode::CEE_LDC_I4_2{});
    method.Insert(method.begin() + 4, OpCode::CEE_MUL{});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Tests do similar actions as InsertSimpleFunction, but
// this use different injection targets and different injections.

// Checks injection to.
// private static int MyInjectionTarget(bool a, int x, int y)
// {
//     if (a)
//     {
//         return x * y;
//     }
//     else
//     {
//         return 0;
//     }
// }
// Will replace line
// return 0;
// with
// return 0 + x + y;
TEST(MethodBodyTests, InsertFunctionWithIf)
{
    // Arrange
    // Got these values from MS IL decompiler.
    const std::vector<std::byte> sourceBytes {
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x14 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0A }, std::byte { 0x06 },
        std::byte { 0x2C }, std::byte { 0x07 }, std::byte { 0x00 }, std::byte { 0x03 },
        std::byte { 0x04 }, std::byte { 0x5A }, std::byte { 0x0B }, std::byte { 0x2B },
        std::byte { 0x05 }, std::byte { 0x00 }, std::byte { 0x16 }, std::byte { 0x0B },
        std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x07 }, std::byte { 0x2A }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // Recalculated sourceBytes manually to get this.
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x18 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0A }, std::byte { 0x06 },
        std::byte { 0x2C }, std::byte { 0x07 }, std::byte { 0x00 }, std::byte { 0x03 },
        std::byte { 0x04 }, std::byte { 0x5A }, std::byte { 0x0B }, std::byte { 0x2B },
        std::byte { 0x09 }, std::byte { 0x00 }, std::byte { 0x16 }, std::byte { 0x03 },
        std::byte { 0x58 }, std::byte { 0x04 }, std::byte { 0x58 }, std::byte { 0x0B },
        std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x07 }, std::byte { 0x2A }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label elseLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel1{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel2{ sourceStreamLabelCreator.CreateLabel() };
    // Got these values from MS IL decompiler.
    const InstructionStream expectedSourceStream{
        // if (a)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_BRFALSE_S{elseLabel},

        // return x * y; // <- x * y
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_LDARG_2{},
        OpCode::CEE_MUL{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{endLabel1},

        // return 0; // <- 0;
        elseLabel,
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{endLabel2},

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_RET{}
    };

    const InstructionStream expectedInjectionStream{
        // if (a)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_BRFALSE_S{elseLabel},

        // return x * y; // <- x * y
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_LDARG_2{},
        OpCode::CEE_MUL{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{endLabel1},

        // return 0 + x + y; // <- 0 + x + y;
        elseLabel,
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_LDARG_2{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{endLabel2},

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 14, OpCode::CEE_LDARG_1{});
    method.Insert(method.begin() + 15, OpCode::CEE_ADD{});
    method.Insert(method.begin() + 16, OpCode::CEE_LDARG_2{});
    method.Insert(method.begin() + 17, OpCode::CEE_ADD{});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

//Checks this injection:
// private static int MyInjectionTarget(int x)
// {
//     if (x < 2)
//     {
//         return x;
//     }
// 
//     int previous = 0;
//     int current = 1;
// 
//     for (int i = 2; i <= x; i++)
//     {
//         int newCurrent = current + previous;
//         previous = current;
//         current = newCurrent;
//     }
// 
//     return current;
// }
// Will replace
//         current = newCurrent;
// with
//         current = 2 * newCurrent;
TEST(MethodBodyTests, InsertFunctionWithLoop)
{
    // Arrange
    // Got these values from MS IL decompiler.
    const std::vector<std::byte> sourceBytes {
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x3D }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x18 }, std::byte { 0xFE },
        std::byte { 0x04 }, std::byte { 0x0C }, std::byte { 0x08 }, std::byte { 0x2C },
        std::byte { 0x05 }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0D },
        std::byte { 0x2B }, std::byte { 0x2D }, std::byte { 0x16 }, std::byte { 0x0A },
        std::byte { 0x17 }, std::byte { 0x0B }, std::byte { 0x18 }, std::byte { 0x13 },
        std::byte { 0x04 }, std::byte { 0x2B }, std::byte { 0x12 }, std::byte { 0x00 },
        std::byte { 0x07 }, std::byte { 0x06 }, std::byte { 0x58 }, std::byte { 0x13 },
        std::byte { 0x05 }, std::byte { 0x07 }, std::byte { 0x0A }, std::byte { 0x11 },
        std::byte { 0x05 }, std::byte { 0x0B }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x04 }, std::byte { 0x17 }, std::byte { 0x58 }, std::byte { 0x13 },
        std::byte { 0x04 }, std::byte { 0x11 }, std::byte { 0x04 }, std::byte { 0x02 },
        std::byte { 0xFE }, std::byte { 0x02 }, std::byte { 0x16 }, std::byte { 0xFE },
        std::byte { 0x01 }, std::byte { 0x13 }, std::byte { 0x06 }, std::byte { 0x11 },
        std::byte { 0x06 }, std::byte { 0x2D }, std::byte { 0xE0 }, std::byte { 0x07 },
        std::byte { 0x0D }, std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x09 },
        std::byte { 0x2A }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // Recalculated sourceBytes manually to get this.
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x3F }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x18 }, std::byte { 0xFE },
        std::byte { 0x04 }, std::byte { 0x0C }, std::byte { 0x08 }, std::byte { 0x2C },
        std::byte { 0x05 }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0D },
        std::byte { 0x2B }, std::byte { 0x2F }, std::byte { 0x16 }, std::byte { 0x0A },
        std::byte { 0x17 }, std::byte { 0x0B }, std::byte { 0x18 }, std::byte { 0x13 },
        std::byte { 0x04 }, std::byte { 0x2B }, std::byte { 0x14 }, std::byte { 0x00 },
        std::byte { 0x07 }, std::byte { 0x06 }, std::byte { 0x58 }, std::byte { 0x13 },
        std::byte { 0x05 }, std::byte { 0x07 }, std::byte { 0x0A }, std::byte { 0x18 },
        std::byte { 0x11 }, std::byte { 0x05 }, std::byte { 0x5A }, std::byte { 0x0B },
        std::byte { 0x00 }, std::byte { 0x11 }, std::byte { 0x04 }, std::byte { 0x17 },
        std::byte { 0x58 }, std::byte { 0x13 }, std::byte { 0x04 }, std::byte { 0x11 },
        std::byte { 0x04 }, std::byte { 0x02 }, std::byte { 0xFE }, std::byte { 0x02 },
        std::byte { 0x16 }, std::byte { 0xFE }, std::byte { 0x01 }, std::byte { 0x13 },
        std::byte { 0x06 }, std::byte { 0x11 }, std::byte { 0x06 }, std::byte { 0x2D },
        std::byte { 0xDE }, std::byte { 0x07 }, std::byte { 0x0D }, std::byte { 0x2B },
        std::byte { 0x00 }, std::byte { 0x09 }, std::byte { 0x2A }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label elseLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel1{ sourceStreamLabelCreator.CreateLabel() };
    const Label loopConditionLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label loopBodyLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel2{ sourceStreamLabelCreator.CreateLabel() };
    // Got these values from MS IL decompiler.
    const InstructionStream expectedSourceStream{
        // if (x < 2)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_CLT{},
        OpCode::CEE_STLOC_2{},
        OpCode::CEE_LDLOC_2{},
        OpCode::CEE_BRFALSE_S{elseLabel},

        // return x; // <- x
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_3{},
        OpCode::CEE_BR_S{endLabel1},

        // int previous = 0;
        elseLabel,
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_0{},

        // int current = 1;
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_STLOC_1{},

        // for (int i = 2; i <= x; i++) // <- int i = 2
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_STLOC_S{4},
        OpCode::CEE_BR_S{loopConditionLabel},

        // int newCurrent = current + previous;
        loopBodyLabel,
        OpCode::CEE_NOP{},
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_S{5},

        // previous = current;
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_STLOC_0{},

        // current = newCurrent;
        OpCode::CEE_LDLOC_S{5},
        OpCode::CEE_STLOC_1{},

        // for (int i = 2; i <= x; i++) // <- i++
        OpCode::CEE_NOP{},
        OpCode::CEE_LDLOC_S{4},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_S{4},

        // for (int i = 2; i <= x; i++) // <- i <= x
        loopConditionLabel,
        OpCode::CEE_LDLOC_S{4},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_CGT{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_S{6},
        OpCode::CEE_LDLOC_S{6},
        OpCode::CEE_BRTRUE_S{loopBodyLabel},

        // return current; // <- current
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_STLOC_3{},
        OpCode::CEE_BR_S{endLabel2},

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_3{},
        OpCode::CEE_RET{}
    };

    const InstructionStream expectedInjectionStream{
        // if (x < 2)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_CLT{},
        OpCode::CEE_STLOC_2{},
        OpCode::CEE_LDLOC_2{},
        OpCode::CEE_BRFALSE_S{elseLabel},

        // return x; // <- x
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_3{},
        OpCode::CEE_BR_S{endLabel1},

        // int previous = 0;
        elseLabel,
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_0{},

        // int current = 1;
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_STLOC_1{},

        // for (int i = 2; i <= x; i++) // <- int i = 2
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_STLOC_S{4},
        OpCode::CEE_BR_S{loopConditionLabel},

        // int newCurrent = current + previous;
        loopBodyLabel,
        OpCode::CEE_NOP{},
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_S{5},

        // previous = current;
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_STLOC_0{},

        // current = 2 * newCurrent;
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_LDLOC_S{5},
        OpCode::CEE_MUL{},
        OpCode::CEE_STLOC_1{},

        // for (int i = 2; i <= x; i++) // <- i++
        OpCode::CEE_NOP{},
        OpCode::CEE_LDLOC_S{4},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_S{4},

        // for (int i = 2; i <= x; i++) // <- i <= x
        loopConditionLabel,
        OpCode::CEE_LDLOC_S{4},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_CGT{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_S{6},
        OpCode::CEE_LDLOC_S{6},
        OpCode::CEE_BRTRUE_S{loopBodyLabel},

        // return current; // <- current
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_STLOC_3{},
        OpCode::CEE_BR_S{endLabel2},

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_3{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 27, OpCode::CEE_LDC_I4_2{});
    method.Insert(method.begin() + 29, OpCode::CEE_MUL{});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks this injection:
// private static int MyInjectionTarget(int x, int y)
// {
//     switch (x)
//     {
//     case 0:
//     {
//         return x + y; // Will change to return -(x + y);
//     }
//     break;
//     case 1:
//     case 2:
//     {
//         return x * y; // Will change to return x * y * 3;
//     }
//     break;
//     case 3:
//     {
//         return x - y; // Will change to return x - y + 42;
//     }
//     break;
//     case 4:
//     default:
//     {
//         return x / y; // Will change to return x / y / 5;
//     }
//     break;
//     }
// }
TEST(MethodBodyTests, InsertFunctionWithSwitch)
{
    // Arrange
    // Got these values from MS IL decompiler.
    const std::vector<std::byte> sourceBytes {
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x3F }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0B }, std::byte { 0x07 },
        std::byte { 0x0A }, std::byte { 0x06 }, std::byte { 0x45 }, std::byte { 0x05 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x02 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x09 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x09 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x10 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x17 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x2B },
        std::byte { 0x15 }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x03 },
        std::byte { 0x58 }, std::byte { 0x0C }, std::byte { 0x2B }, std::byte { 0x15 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x5A },
        std::byte { 0x0C }, std::byte { 0x2B }, std::byte { 0x0E }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x59 }, std::byte { 0x0C },
        std::byte { 0x2B }, std::byte { 0x07 }, std::byte { 0x00 }, std::byte { 0x02 },
        std::byte { 0x03 }, std::byte { 0x5B }, std::byte { 0x0C }, std::byte { 0x2B },
        std::byte { 0x00 }, std::byte { 0x08 }, std::byte { 0x2A }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // Recalculated sourceBytes manually to get this.
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x4A }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0B }, std::byte { 0x07 },
        std::byte { 0x0A }, std::byte { 0x06 }, std::byte { 0x45 }, std::byte { 0x05 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x02 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x13 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x20 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x2B },
        std::byte { 0x1E }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x03 },
        std::byte { 0x58 }, std::byte { 0x65 }, std::byte { 0x0C }, std::byte { 0x2B },
        std::byte { 0x1F }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x03 },
        std::byte { 0x5A }, std::byte { 0x19 }, std::byte { 0x5A }, std::byte { 0x0C },
        std::byte { 0x2B }, std::byte { 0x16 }, std::byte { 0x00 }, std::byte { 0x02 },
        std::byte { 0x03 }, std::byte { 0x59 }, std::byte { 0x20 }, std::byte { 0x2A },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x58 },
        std::byte { 0x0C }, std::byte { 0x2B }, std::byte { 0x09 }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x5B }, std::byte { 0x1B },
        std::byte { 0x5B }, std::byte { 0x0C }, std::byte { 0x2B }, std::byte { 0x00 },
        std::byte { 0x08 }, std::byte { 0x2A }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label case0Label{ sourceStreamLabelCreator.CreateLabel() };
    const Label case1Label{ sourceStreamLabelCreator.CreateLabel() };
    const Label case2Label{ sourceStreamLabelCreator.CreateLabel() };
    const Label case3Label{ sourceStreamLabelCreator.CreateLabel() };
    const Label defaultLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label defaultLabel2{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel1{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel2{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel3{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel4{ sourceStreamLabelCreator.CreateLabel() };
    // Got these values from MS IL decompiler.
    const InstructionStream expectedSourceStream{
        // switch (x)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_SWITCH{{ case0Label, case1Label, case2Label, case3Label, defaultLabel }},
        OpCode::CEE_BR_S{defaultLabel2},

        // case 0:
        case0Label,
        // return x + y; // <- x + y
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel1},

        // case 1:
        // case 2:
        case1Label,
        case2Label,
        // return x * y; // <- x * y
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_MUL{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel2},

        // case 3:
        case3Label,

        // return x - y; // <- x - y
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_SUB{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel3},

        // case 4:
        // default:
        defaultLabel,
        defaultLabel2,

        // return x / y; // <- x / y
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel4},

        // return
        endLabel1,
        endLabel2,
        endLabel3,
        endLabel4,
        OpCode::CEE_LDLOC_2{},
        OpCode::CEE_RET{}
    };

    const InstructionStream expectedInjectionStream{
        // switch (x)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_SWITCH{{ case0Label, case1Label, case2Label, case3Label, defaultLabel }},
        OpCode::CEE_BR_S{defaultLabel2},

        // case 0:
        case0Label,
        // return -(x + y); // <- -(x + y)
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_NEG{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel1},

        // case 1:
        // case 2:
        case1Label,
        case2Label,
        // return x * y * 3; // <- x * y * 3
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_MUL{},
        OpCode::CEE_LDC_I4_3{},
        OpCode::CEE_MUL{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel2},

        // case 3:
        case3Label,

        // return x - y + 42; // <- x - y + 42
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_SUB{},
        OpCode::CEE_LDC_I4{42},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel3},

        // case 4:
        // default:
        defaultLabel,
        defaultLabel2,

        // return x / y / 5; // <- x / y / 5
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_LDC_I4_5{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_2{},

        // break;
        OpCode::CEE_BR_S{endLabel4},

        // return
        endLabel1,
        endLabel2,
        endLabel3,
        endLabel4,
        OpCode::CEE_LDLOC_2{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 13, OpCode::CEE_NEG{});

    method.Insert(method.begin() + 22, OpCode::CEE_LDC_I4_3{});
    method.Insert(method.begin() + 23, OpCode::CEE_MUL{});

    method.Insert(method.begin() + 31, OpCode::CEE_LDC_I4{42});
    method.Insert(method.begin() + 32, OpCode::CEE_ADD{});

    method.Insert(method.begin() + 41, OpCode::CEE_LDC_I4_5{});
    method.Insert(method.begin() + 42, OpCode::CEE_DIV{});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks this injection:
// private static int MyInjectionTarget(int x, int y)
// {
//     try
//     {
//         return x / y;
//     }
//     catch (DivideByZeroException)
//     {
//         return x;
//     }
// }
// Will replace
//         return x / y;
// with
//         return x / (y + 1);
TEST(MethodBodyTests, InsertFunctionWithTryCatch)
{
    // Arrange
    // Got these values from MS IL decompiler.
    const std::vector<std::byte> sourceBytes {
        std::byte { 0x1B }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x03 },
        std::byte { 0x5B }, std::byte { 0x0A }, std::byte { 0xDE }, std::byte { 0x06 },
        std::byte { 0x26 }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x0A },
        std::byte { 0xDE }, std::byte { 0x00 }, std::byte { 0x06 }, std::byte { 0x2A },
        std::byte { 0x01 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x01 }, std::byte { 0x00 },
        std::byte { 0x07 }, std::byte { 0x08 }, std::byte { 0x00 }, std::byte { 0x06 },
        std::byte { 0x0D }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x01 }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // Recalculated sourceBytes manually to get this.
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x1B }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x12 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x03 },
        std::byte { 0x17 }, std::byte { 0x58 }, std::byte { 0x5B }, std::byte { 0x0A },
        std::byte { 0xDE }, std::byte { 0x06 }, std::byte { 0x26 }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x0A }, std::byte { 0xDE }, std::byte { 0x00 },
        std::byte { 0x06 }, std::byte { 0x2A }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x01 }, std::byte { 0x00 },
        std::byte { 0x09 }, std::byte { 0x0A }, std::byte { 0x00 }, std::byte { 0x06 },
        std::byte { 0x0D }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x01 }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label endLabel1{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel2{ sourceStreamLabelCreator.CreateLabel() };
    const Label tryLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label tryEndLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label handlerLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label handlerEndLabel{ sourceStreamLabelCreator.CreateLabel() };
    // Got these values from MS IL decompiler.
    const InstructionStream expectedSourceStream{
        OpCode::CEE_NOP{},

        // try 
        // {
        tryLabel,
        OpCode::CEE_NOP{},
        // return x / y; // <- x / y
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_0{},

        // } // end of try {
        OpCode::CEE_LEAVE_S{endLabel1},
        tryEndLabel,

        // catch (DivideByZeroException)
        // {
        handlerLabel,
        OpCode::CEE_POP{},
        OpCode::CEE_NOP{},

        // return x; // <- x
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_0{},

        // }
        OpCode::CEE_LEAVE_S{endLabel2},
        handlerEndLabel,

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_RET{}
    };

    const InstructionStream expectedInjectionStream{
        OpCode::CEE_NOP{},

        // try 
        // {
        tryLabel,
        OpCode::CEE_NOP{},
        // return x / (y + 1); // <- x / y
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_0{},

        // } // end of try {
        OpCode::CEE_LEAVE_S{endLabel1},
        tryEndLabel,

        // catch (DivideByZeroException)
        // {
        handlerLabel,
        OpCode::CEE_POP{},
        OpCode::CEE_NOP{},

        // return x; // <- x
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_STLOC_0{},

        // }
        OpCode::CEE_LEAVE_S{endLabel2},
        handlerEndLabel,

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 5, OpCode::CEE_LDC_I4_1{});
    method.Insert(method.begin() + 6, OpCode::CEE_ADD{});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(1, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks this injection:
// private static int MyInjectionTarget(int x, int y)
// {
//     int z = 0;
//     try
//     {
//         z = x / y;
//     }
//     finally
//     {
//         y = 42;
//         // Will inject
//         // x = 0;
//     }
//
//     return z + y;
// }
TEST(MethodBodyTests, InsertFunctionWithTryFinally)
{
    // Arrange
    // Got these values from MS IL decompiler.
    const std::vector<std::byte> sourceBytes {
        std::byte { 0x1B }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x1A }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x16 }, std::byte { 0x0A }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x5B }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0xDE }, std::byte { 0x07 }, std::byte { 0x00 },
        std::byte { 0x1F }, std::byte { 0x2A }, std::byte { 0x10 }, std::byte { 0x01 },
        std::byte { 0x00 }, std::byte { 0xDC }, std::byte { 0x06 }, std::byte { 0x03 },
        std::byte { 0x58 }, std::byte { 0x0B }, std::byte { 0x2B }, std::byte { 0x00 },
        std::byte { 0x07 }, std::byte { 0x2A }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x00 }, std::byte { 0x03 }, std::byte { 0x00 },
        std::byte { 0x08 }, std::byte { 0x0B }, std::byte { 0x00 }, std::byte { 0x07 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // Recalculated sourceBytes manually to get this.
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x1B }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x1D }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x16 }, std::byte { 0x0A }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x5B }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0xDE }, std::byte { 0x0A }, std::byte { 0x00 },
        std::byte { 0x1F }, std::byte { 0x2A }, std::byte { 0x10 }, std::byte { 0x01 },
        std::byte { 0x16 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0xDC }, std::byte { 0x06 }, std::byte { 0x03 }, std::byte { 0x58 },
        std::byte { 0x0B }, std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x07 },
        std::byte { 0x2A }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x00 }, std::byte { 0x03 }, std::byte { 0x00 },
        std::byte { 0x08 }, std::byte { 0x0B }, std::byte { 0x00 }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label endLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label retLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label tryLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label tryEndLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label handlerLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label handlerEndLabel{ sourceStreamLabelCreator.CreateLabel() };
    // Got these values from MS IL decompiler.
    const InstructionStream expectedSourceStream{
        // int z = 0;
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_0{},

        // try
        // {
        tryLabel,
        OpCode::CEE_NOP{},

        // z = x / y;
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_0{},

        // } // end of try {
        OpCode::CEE_NOP{},
        OpCode::CEE_LEAVE_S{endLabel},
        tryEndLabel,

        // finally
        // {
        handlerLabel,
        OpCode::CEE_NOP{},

        // y = 42;
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STARG_S{1},

        // } // end of finally {
        OpCode::CEE_NOP{},
        OpCode::CEE_ENDFINALLY{},
        handlerEndLabel,

        // return z + y; // <- z + y
        endLabel,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{retLabel},

        // return z + y; // <- return
        retLabel,
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_RET{}
    };

    const InstructionStream expectedInjectionStream{
        // int z = 0;
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_0{},

        // try
        // {
        tryLabel,
        OpCode::CEE_NOP{},

        // z = x / y;
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_0{},

        // } // end of try {
        OpCode::CEE_NOP{},
        OpCode::CEE_LEAVE_S{endLabel},
        tryEndLabel,

        // finally
        // {
        handlerLabel,
        OpCode::CEE_NOP{},

        // y = 42;
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STARG_S{1},

        // x = 0;
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STARG_S{0},

        // } // end of finally {
        OpCode::CEE_NOP{},
        OpCode::CEE_ENDFINALLY{},
        handlerEndLabel,

        // return z + y; // <- z + y
        endLabel,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{retLabel},

        // return z + y; // <- return
        retLabel,
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 16, OpCode::CEE_LDC_I4_0{});
    method.Insert(method.begin() + 17, OpCode::CEE_STARG_S{0});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(1, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks this injection:
// private static int MyInjectionTarget(int x, int y)
// {
//     int z = 0;
//     try
//     {
//         z = x / y;
//     }
//     catch (DivideByZeroException e) when(e.Message.Length % 2 == 0)
//     {
//         y = 42;
//     }
//     
//     return z + y;
// }
// Will replace
// catch (DivideByZeroException e) when(e.Message.Length % 2 == 0)
// with
// catch (DivideByZeroException e) when(e.Message.Length % 2 == (0 + 1))
TEST(MethodBodyTests, InsertFunctionWithTryCatchWhen)
{
    // Arrange
    // Got these values from MS IL decompiler.
    const std::vector<std::byte> sourceBytes {
        std::byte { 0x1B }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x40 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x16 }, std::byte { 0x0A }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x5B }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0xDE }, std::byte { 0x2D }, std::byte { 0x75 },
        std::byte { 0x0D }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x01 },
        std::byte { 0x25 }, std::byte { 0x2D }, std::byte { 0x04 }, std::byte { 0x26 },
        std::byte { 0x16 }, std::byte { 0x2B }, std::byte { 0x16 }, std::byte { 0x0B },
        std::byte { 0x07 }, std::byte { 0x6F }, std::byte { 0x0C }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x0A }, std::byte { 0x6F }, std::byte { 0x0D },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x0A }, std::byte { 0x18 },
        std::byte { 0x5D }, std::byte { 0x16 }, std::byte { 0xFE }, std::byte { 0x01 },
        std::byte { 0x0C }, std::byte { 0x08 }, std::byte { 0x16 }, std::byte { 0xFE },
        std::byte { 0x03 }, std::byte { 0xFE }, std::byte { 0x11 }, std::byte { 0x26 },
        std::byte { 0x00 }, std::byte { 0x1F }, std::byte { 0x2A }, std::byte { 0x10 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0xDE }, std::byte { 0x00 },
        std::byte { 0x06 }, std::byte { 0x03 }, std::byte { 0x58 }, std::byte { 0x0D },
        std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x09 }, std::byte { 0x2A },
        std::byte { 0x01 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x03 }, std::byte { 0x00 },
        std::byte { 0x08 }, std::byte { 0x2F }, std::byte { 0x00 }, std::byte { 0x09 },
        std::byte { 0x0B }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    // Recalculated sourceBytes manually to get this.
    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x1B }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x42 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x16 }, std::byte { 0x0A }, std::byte { 0x00 },
        std::byte { 0x02 }, std::byte { 0x03 }, std::byte { 0x5B }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0xDE }, std::byte { 0x2F }, std::byte { 0x75 },
        std::byte { 0x0D }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x01 },
        std::byte { 0x25 }, std::byte { 0x2D }, std::byte { 0x04 }, std::byte { 0x26 },
        std::byte { 0x16 }, std::byte { 0x2B }, std::byte { 0x18 }, std::byte { 0x0B },
        std::byte { 0x07 }, std::byte { 0x6F }, std::byte { 0x0C }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x0A }, std::byte { 0x6F }, std::byte { 0x0D },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x0A }, std::byte { 0x18 },
        std::byte { 0x5D }, std::byte { 0x16 }, std::byte { 0x17 }, std::byte { 0x58 },
        std::byte { 0xFE }, std::byte { 0x01 }, std::byte { 0x0C }, std::byte { 0x08 },
        std::byte { 0x16 }, std::byte { 0xFE }, std::byte { 0x03 }, std::byte { 0xFE },
        std::byte { 0x11 }, std::byte { 0x26 }, std::byte { 0x00 }, std::byte { 0x1F },
        std::byte { 0x2A }, std::byte { 0x10 }, std::byte { 0x01 }, std::byte { 0x00 },
        std::byte { 0xDE }, std::byte { 0x00 }, std::byte { 0x06 }, std::byte { 0x03 },
        std::byte { 0x58 }, std::byte { 0x0D }, std::byte { 0x2B }, std::byte { 0x00 },
        std::byte { 0x09 }, std::byte { 0x2A }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x10 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x03 }, std::byte { 0x00 },
        std::byte { 0x08 }, std::byte { 0x31 }, std::byte { 0x00 }, std::byte { 0x09 },
        std::byte { 0x0B }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label endLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label isInstanceLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label endFilterLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel2{ sourceStreamLabelCreator.CreateLabel() };
    const Label retLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label tryLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label tryEndLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label handlerLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label handlerEndLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label filterLabel{ sourceStreamLabelCreator.CreateLabel() };
    // Got these values from MS IL decompiler.
    const InstructionStream expectedSourceStream{
        // int z = 0;
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_0{},

        // try
        // {
        tryLabel,
        OpCode::CEE_NOP{},

        // z = x / y;
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_0{},

        // } // end of try {
        OpCode::CEE_NOP{},
        OpCode::CEE_LEAVE_S{endLabel},
        tryEndLabel,

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == 0) // <- when
        filterLabel,

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == 0) // <- DivideByZeroException e
        OpCode::CEE_ISINST{ 0x0100000D },
        OpCode::CEE_DUP{},
        OpCode::CEE_BRTRUE_S{ isInstanceLabel },
        OpCode::CEE_POP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_BR_S{endFilterLabel},

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == 0) // <- e.Message.Length % 2 == 0
        isInstanceLabel,
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_CALLVIRT{ 0x0A00000C },
        OpCode::CEE_CALLVIRT{ 0x0A00000D },
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_REM{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_2{},
        OpCode::CEE_LDLOC_2{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_CGT_UN{},
        endFilterLabel,
        OpCode::CEE_ENDFILTER{},

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == 0) // <- catch
        // {
        handlerLabel,
        OpCode::CEE_POP{},
        OpCode::CEE_NOP{},

        // y = 42;
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STARG_S{1},

        // } // end of catch {
        OpCode::CEE_NOP{},
        OpCode::CEE_LEAVE_S{endLabel2},
        handlerEndLabel,

        // return z + y; // <- z + y
        endLabel,
        endLabel2,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_3{},
        OpCode::CEE_BR_S{retLabel},

        // return z + y; // <- return
        retLabel,
        OpCode::CEE_LDLOC_3{},
        OpCode::CEE_RET{}
    };

    const InstructionStream expectedInjectionStream{
        // int z = 0;
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_0{},

        // try
        // {
        tryLabel,
        OpCode::CEE_NOP{},

        // z = x / y;
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_DIV{},
        OpCode::CEE_STLOC_0{},

        // } // end of try {
        OpCode::CEE_NOP{},
        OpCode::CEE_LEAVE_S{endLabel},
        tryEndLabel,

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == (0 + 1)) // <- when
        filterLabel,

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == (0 + 1)) // <- DivideByZeroException e
        OpCode::CEE_ISINST{ 0x0100000D },
        OpCode::CEE_DUP{},
        OpCode::CEE_BRTRUE_S{ isInstanceLabel },
        OpCode::CEE_POP{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_BR_S{endFilterLabel},

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == (0 + 1)) // <- e.Message.Length % 2 == (0 + 1)
        isInstanceLabel,
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_CALLVIRT{ 0x0A00000C },
        OpCode::CEE_CALLVIRT{ 0x0A00000D },
        OpCode::CEE_LDC_I4_2{},
        OpCode::CEE_REM{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_2{},
        OpCode::CEE_LDLOC_2{},
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_CGT_UN{},
        endFilterLabel,
        OpCode::CEE_ENDFILTER{},

        // catch (DivideByZeroException e) when(e.Message.Length % 2 == (0 + 1)) // <- catch
        // {
        handlerLabel,
        OpCode::CEE_POP{},
        OpCode::CEE_NOP{},

        // y = 42;
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STARG_S{1},

        // } // end of catch {
        OpCode::CEE_NOP{},
        OpCode::CEE_LEAVE_S{endLabel2},
        handlerEndLabel,

        // return z + y; // <- z + y
        endLabel,
        endLabel2,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_LDARG_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_3{},
        OpCode::CEE_BR_S{retLabel},

        // return z + y; // <- return
        retLabel,
        OpCode::CEE_LDLOC_3{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    method.Insert(method.begin() + 27, OpCode::CEE_LDC_I4_1{});
    method.Insert(method.begin() + 28, OpCode::CEE_ADD{});

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(1, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks this injection:
// private static int MyInjectionTarget(int x)
// {
//     if (x == 1)
//     {
//         return 42;
//         // Will insert 128 No Operation instructions here
//     }
// 
//     return 0;
// }
//
// The branching instruction correnponding to if
// must be converted from brfalse.s (short form)
// to brfalse (long form).
TEST(MethodBodyTests, TransformJumpToLongIfNeeded)
{
    // Arrange
    const std::vector<std::byte> sourceBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x15 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x17 }, std::byte { 0xFE },
        std::byte { 0x01 }, std::byte { 0x0A }, std::byte { 0x06 }, std::byte { 0x2C },
        std::byte { 0x06 }, std::byte { 0x00 }, std::byte { 0x1F }, std::byte { 0x2A },
        std::byte { 0x0B }, std::byte { 0x2B }, std::byte { 0x04 }, std::byte { 0x16 },
        std::byte { 0x0B }, std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x07 },
        std::byte { 0x2A }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x98 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x17 }, std::byte { 0xFE },
        std::byte { 0x01 }, std::byte { 0x0A }, std::byte { 0x06 }, std::byte { 0x39 },
        std::byte { 0x86 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x1F }, std::byte { 0x2A }, std::byte { 0x0B },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x2B }, std::byte { 0x04 }, std::byte { 0x16 }, std::byte { 0x0B },
        std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x07 }, std::byte { 0x2A }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label elseLabel{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel1{ sourceStreamLabelCreator.CreateLabel() };
    const Label endLabel2{ sourceStreamLabelCreator.CreateLabel() };
    const InstructionStream expectedSourceStream{
        // if (x == 1) // <- x == 1
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_LDLOC_0{},

        // if (x == 1) // <- if
        OpCode::CEE_BRFALSE_S{elseLabel},

        // return 42; // <- 42
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{endLabel1},

        // return 0; // <- 0
        elseLabel,
        OpCode::CEE_LDC_I4_0{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_BR_S{endLabel2},

        // return
        endLabel1,
        endLabel2,
        OpCode::CEE_LDLOC_1{},
        OpCode::CEE_RET{}
    };

    const size_t insertionLength{ 128 };
    const ptrdiff_t insertionPosition{ 10 };
    InstructionStream expectedInjectionStream(expectedSourceStream);
    expectedInjectionStream[6] = OpCode::CEE_BRFALSE{ elseLabel };
    expectedInjectionStream.insert(
        expectedInjectionStream.cbegin() + insertionPosition,
        insertionLength,
        OpCode::CEE_NOP{});

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    for (size_t i{ 0 }; i != insertionLength; ++i)
    {
        method.Insert(method.begin() + insertionPosition, OpCode::CEE_NOP{});
    }

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks this injection:
// private static int MyInjectionTarget(int x)
// {
//     // // Will insert
//     //if (x == 1)
//     //{
//     //    return 42;
//     //    // 128 No Operation instructions here
//     //}
// 
//     return x + 1;
// }
//
// We will inject brfalse.s (short form), but
// the injection mechanism should be able to
// convert it to the long form - brfalse.
TEST(MethodBodyTests, InsertTransformJumpToLongIfNeeded)
{
    // Arrange
    const std::vector<std::byte> sourceBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x09 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x17 }, std::byte { 0x58 },
        std::byte { 0x0A }, std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x06 },
        std::byte { 0x2A }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x9A }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x17 }, std::byte { 0xFE },
        std::byte { 0x01 }, std::byte { 0x0B }, std::byte { 0x07 }, std::byte { 0x39 },
        std::byte { 0x86 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x1F }, std::byte { 0x2A }, std::byte { 0x0A },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x2B }, std::byte { 0x06 }, std::byte { 0x02 }, std::byte { 0x17 },
        std::byte { 0x58 }, std::byte { 0x0A }, std::byte { 0x2B }, std::byte { 0x00 },
        std::byte { 0x06 }, std::byte { 0x2A }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label endLabel{ sourceStreamLabelCreator.CreateLabel() };
    const InstructionStream expectedSourceStream{
        // return x + 1; // <- x + 1
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_BR_S{endLabel},

        // return x + 1; // <- return
        endLabel,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_RET{}
    };

    const Label elseLabel{ sourceStreamLabelCreator.CreateLabel() };
    const size_t insertionLength{ 128 };
    const ptrdiff_t insertionPosition{ 10 };
    InstructionStream expectedInjectionStream{
        // if (x == 1) // <- x == 1
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_LDLOC_1{},

        // if (x == 1) // <- if
        OpCode::CEE_BRFALSE{elseLabel},

        // return 42; // <- 42
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STLOC_0{},
        // 128 No Operation instruction will go here
        OpCode::CEE_BR_S{endLabel},

        // return x + 1; // <- x + 1
        elseLabel,
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_BR_S{endLabel},

        // return
        endLabel,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_RET{}
    };

    expectedInjectionStream.insert(
        expectedInjectionStream.cbegin() + insertionPosition,
        insertionLength,
        OpCode::CEE_NOP{});

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    // if (x == 1) // <- x == 1
    method.Insert(method.begin() + 1, OpCode::CEE_LDARG_0{});
    method.Insert(method.begin() + 2, OpCode::CEE_LDC_I4_1{});
    method.Insert(method.begin() + 3, OpCode::CEE_CEQ{});
    method.Insert(method.begin() + 4, OpCode::CEE_STLOC_1{});
    method.Insert(method.begin() + 5, OpCode::CEE_LDLOC_1{});

    // if (x == 1) // <- if
    const Label actualElseLabel{ method.CreateLabel() };
    method.Insert(method.begin() + 6, OpCode::CEE_BRFALSE_S{ actualElseLabel });

    // return 42; // <- 42
    method.Insert(method.begin() + 7, OpCode::CEE_NOP{});
    method.Insert(method.begin() + 8, OpCode::CEE_LDC_I4_S{42});
    method.Insert(method.begin() + 9, OpCode::CEE_STLOC_0{});

    // 128 No Operation instructions
    for (size_t i{ 0 }; i != insertionLength; ++i)
    {
        method.Insert(method.begin() + insertionPosition, OpCode::CEE_NOP{});
    }

    method.Insert(method.begin() + 138, OpCode::CEE_BR_S{ endLabel });
    method.MarkLabel(method.begin() + 139, actualElseLabel);

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks injection containing branching:
// private static int MyInjectionTarget(int x)
// {
//     // // Will insert
//     //if (x == 1)
//     //{
//     //    return 42;
//     //}
// 
//     return x + 1;
// }
//
// Will use methods CreateLabel and MarkLabel
// to add the new branching instruction.
TEST(MethodBodyTests, ÑreateAndMarkLabel)
{
    // Arrange
    const std::vector<std::byte> sourceBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x09 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x17 }, std::byte { 0x58 },
        std::byte { 0x0A }, std::byte { 0x2B }, std::byte { 0x00 }, std::byte { 0x06 },
        std::byte { 0x2A }
    };

    const std::vector<std::byte> expectedRoundtripBytes(sourceBytes);

    const std::vector<std::byte> expectedInjectionBytes{
        std::byte { 0x13 }, std::byte { 0x30 }, std::byte { 0x02 }, std::byte { 0x00 },
        std::byte { 0x17 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
        std::byte { 0x01 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x11 },
        std::byte { 0x00 }, std::byte { 0x02 }, std::byte { 0x17 }, std::byte { 0xFE },
        std::byte { 0x01 }, std::byte { 0x0B }, std::byte { 0x07 }, std::byte { 0x2C },
        std::byte { 0x06 }, std::byte { 0x00 }, std::byte { 0x1F }, std::byte { 0x2A },
        std::byte { 0x0A }, std::byte { 0x2B }, std::byte { 0x06 }, std::byte { 0x02 },
        std::byte { 0x17 }, std::byte { 0x58 }, std::byte { 0x0A }, std::byte { 0x2B },
        std::byte { 0x00 }, std::byte { 0x06 }, std::byte { 0x2A }
    };

    LabelCreator sourceStreamLabelCreator{};
    const Label endLabel{ sourceStreamLabelCreator.CreateLabel() };
    const InstructionStream expectedSourceStream{
        // return x + 1; // <- x + 1
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_BR_S{endLabel},

        // return x + 1; // <- return
        endLabel,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_RET{}
    };

    const Label elseLabel{ sourceStreamLabelCreator.CreateLabel() };
    InstructionStream expectedInjectionStream{
        // if (x == 1) // <- x == 1
        OpCode::CEE_NOP{},
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_CEQ{},
        OpCode::CEE_STLOC_1{},
        OpCode::CEE_LDLOC_1{},

        // if (x == 1) // <- if
        OpCode::CEE_BRFALSE_S{elseLabel},

        // return 42; // <- 42
        OpCode::CEE_NOP{},
        OpCode::CEE_LDC_I4_S{42},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_BR_S{endLabel},

        // return x + 1; // <- x + 1
        elseLabel,
        OpCode::CEE_LDARG_0{},
        OpCode::CEE_LDC_I4_1{},
        OpCode::CEE_ADD{},
        OpCode::CEE_STLOC_0{},
        OpCode::CEE_BR_S{endLabel},

        // return
        endLabel,
        OpCode::CEE_LDLOC_0{},
        OpCode::CEE_RET{}
    };

    // Act
    MethodBody method(sourceBytes);

    const InstructionStream actualSourceStream(method.Stream());
    const std::vector<std::byte> actualRoundtripBytes(method.Compile());
    const std::vector<ExceptionsSection> actualExceptionSections(method.ExceptionSections());

    // if (x == 1) // <- x == 1
    method.Insert(method.begin() + 1, OpCode::CEE_LDARG_0{});
    method.Insert(method.begin() + 2, OpCode::CEE_LDC_I4_1{});
    method.Insert(method.begin() + 3, OpCode::CEE_CEQ{});
    method.Insert(method.begin() + 4, OpCode::CEE_STLOC_1{});
    method.Insert(method.begin() + 5, OpCode::CEE_LDLOC_1{});

    // if (x == 1) // <- if
    const Label actualElseLabel{ method.CreateLabel() };
    method.Insert(method.begin() + 6, OpCode::CEE_BRFALSE_S{ actualElseLabel });

    // return 42; // <- 42
    method.Insert(method.begin() + 7, OpCode::CEE_NOP{});
    method.Insert(method.begin() + 8, OpCode::CEE_LDC_I4_S{ 42 });
    method.Insert(method.begin() + 9, OpCode::CEE_STLOC_0{});

    method.Insert(method.begin() + 10, OpCode::CEE_BR_S{ endLabel });
    method.MarkLabel(method.begin() + 11, actualElseLabel);

    const InstructionStream actualInjectionStream(method.Stream());
    const std::vector<std::byte> actualInjectionBytes(method.Compile());

    // Assert
    EXPECT_EQ(expectedSourceStream, actualSourceStream);
    EXPECT_EQ(expectedRoundtripBytes, actualRoundtripBytes);
    EXPECT_EQ(0, actualExceptionSections.size());
    EXPECT_EQ(expectedInjectionStream, actualInjectionStream);
    EXPECT_EQ(expectedInjectionBytes, actualInjectionBytes);
}

// Checks that method Compile() throws an
// std::logic_error if there is a branching
// instruction with some label, but MarkLabel()
// was not used to define which instruction the
// label points to.
TEST(MethodBodyTests, CompileThrowsOnUnresolvedLabel)
{
    // Arrange
    std::vector<std::byte> rawBytes(CreateSimpleFunction());
    MethodBody method(rawBytes);

    // Act
    const Label label { method.CreateLabel() };
    method.Insert(
        method.begin() + 2,
        OpCode::CEE_BR_S{ label });

    // Assert
    EXPECT_THROW(method.Compile(), std::logic_error);
}

// Checks that method MarkLabel() throws an
// std::logic_error if it was called again for
// the same MethodBody with the same label.
TEST(MethodBodyTests, MarkLabelThrowsOnLabelMarkedTwice)
{
    // Arrange
    std::vector<std::byte> rawBytes(CreateSimpleFunction());
    MethodBody method(rawBytes);

    // Act
    const Label label { method.CreateLabel() };
    method.MarkLabel(method.begin(), label);

    // Assert
    EXPECT_THROW(
        method.MarkLabel(method.begin() + 2, label),
        std::logic_error);
}
