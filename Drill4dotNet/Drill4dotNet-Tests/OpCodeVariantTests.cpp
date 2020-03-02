#include "pch.h"

#include "OpCodes.h"

using namespace Drill4dotNet;

// For the given variant, asserts methods
// HoldsAlternative and Visit work with the
// given TOpCode.
template <typename TOpCode>
static void AssertVariantHolds(const OpCodeVariant& variant)
{
    const bool holds { variant.HoldsAlternative<TOpCode>() };
    bool visitCalledWith { false };
    variant.Visit([&visitCalledWith](const auto& x)
    {
        visitCalledWith = std::is_same_v<TOpCode, std::decay_t<decltype(x)>>;
    });

    // Assert
    EXPECT_TRUE(holds);
    EXPECT_TRUE(visitCalledWith);
}

// Checks the default constructed OpCodeVariant
// contains a No Operation instruction.
TEST(OpCodeVariantTests, Create)
{
    // Arrange

    // Act
    const OpCodeVariant variant {};

    // Assert
    AssertVariantHolds<OpCode_CEE_NOP>(variant);
    EXPECT_EQ(1, variant.SizeWithArgument());
    EXPECT_TRUE(variant.GetIf<OpCode_CEE_NOP>().has_value());
}

// Checks OpCodeVariant is created with a specific
// opcode without inline argument, and the opcode
// is retrieved again from the OpCodeVariant.
TEST(OpCodeVariantTests, CreateWithoutArgument)
{
    // Arrange

    // Act
    const OpCodeVariant variant { OpCode_CEE_ADD { } };

    // Assert
    AssertVariantHolds<OpCode_CEE_ADD>(variant);
    EXPECT_EQ(1, variant.SizeWithArgument());
    EXPECT_TRUE(variant.GetIf<OpCode_CEE_ADD>().has_value());
}

// Checks OpCodeVariant is created with a specific
// opcode with inline argument, and the opcode and argument
// are retrieved again from the OpCodeVariant.
TEST(OpCodeVariantTests, CreateWithArgument)
{
    // Arrange
    const OpCodeArgumentType::InlineI expectedConstant { 42 };

    // Act
    const OpCodeVariant variant { OpCode_CEE_LDC_I4 { expectedConstant } };
    const std::optional<OpCode_CEE_LDC_I4> actualOpCode = variant.GetIf<OpCode_CEE_LDC_I4>();

    // Assert
    AssertVariantHolds<OpCode_CEE_LDC_I4>(variant);
    EXPECT_EQ(5, variant.SizeWithArgument());
    ASSERT_TRUE(variant.GetIf<OpCode_CEE_LDC_I4>().has_value());
    EXPECT_EQ(expectedConstant, variant.GetIf<OpCode_CEE_LDC_I4>()->Argument());
}
