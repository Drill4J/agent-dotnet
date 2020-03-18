using System;
using System.Numerics;

namespace Calculator
{
    /// <summary>
    /// Represents a rational number as a ratio
    /// of an integer and a positive numbers.
    /// </summary>
    public class Ratio
    {
        /// <summary>
        /// The top part of the fraction.
        /// If this is negative, the overall value is negative.
        /// </summary>
        public int Numerator { get; }

        /// <summary>
        /// The bottom part of the fraction.
        /// This is always a nonzero positive number.
        /// </summary>
        public int Denominator { get; }

        /// <summary>
        /// Creates a new instance of the <see cref="Ratio"/>
        /// class with the given values.
        /// Both <paramref name="integer" /> and <paramref name="numerator" />
        /// cannot be negative at once.
        /// </summary>
        /// <param name="integer">
        /// The integral part of the value.
        /// If this is negative, resulting value will be negative.
        /// </param>
        /// <param name="numerator">
        /// The top part of the fraction.
        /// If this is negative, resulting value will be negative.
        /// </param>
        /// <param name="denominator">
        /// The bottom part of the fraction.
        /// Must be a nonzero positive.
        /// </param>
        /// <exception cref="ArgumentException">
        /// The given values are out of allowed range.
        /// </exception>
        public Ratio(
            int integer,
            int numerator,
            int denominator)
        {
            if (denominator == 0)
            {
                throw new ArgumentException("Denominator must not be null", nameof(denominator));
            }

            if (denominator < 0)
            {
                throw new ArgumentException("Denominator must be positive", nameof(denominator));
            }

            if (integer < 0 && numerator < 0)
            {
                throw new ArgumentException("Both Integer and Numerator part cannot be negative", nameof(numerator));
            }

            if (integer < 0)
            {
                Numerator = integer * denominator - numerator;
                Denominator = denominator;
            }
            else if (numerator < 0)
            {
                Numerator = (-integer) * denominator + numerator;
                Denominator = denominator;
            }
            else
            {
                Numerator = integer * denominator + numerator;
                Denominator = denominator;
            }
        }

        /// <summary>
        /// Creates a new instance of the <see cref="Ratio"/>
        /// class with the given values.
        /// </summary>
        /// <param name="numerator">
        /// The top part of the fraction.
        /// If this is negative, resulting value will be negative.
        /// </param>
        /// <param name="denominator">
        /// The bottom part of the fraction.
        /// Must be a nonzero positive.
        /// </param>
        public Ratio(int numerator, int denominator)
            : this(
                  0,
                  numerator,
                  denominator)
        {
        }

        /// <summary>
        /// Creates a new instance of the <see cref="Ratio"/>
        /// class with the given value.
        /// </summary>
        /// <param name="rational">
        /// The value should represent a valid value.
        /// The <see cref="Rational.Denominator" /> must be
        /// a nonzero positive value. Both <see cref="Rational.Numerator" />
        /// and <see cref="Rational.Integer" /> cannot be
        /// negative at once.
        /// </param>
        public Ratio(Rational rational)
            : this(
                  rational.Integer,
                  rational.Numerator,
                  rational.Denominator)
        {
        }

        /// <summary>
        /// Represents this value as a value with an integral part.
        /// </summary>
        /// <returns>
        /// The value equal to this value.
        /// </returns>
        public Rational AsRational()
        {
            int divisor = (int)BigInteger.GreatestCommonDivisor(
                new BigInteger(Numerator),
                new BigInteger(Denominator));

            int numerator = Math.Abs(Numerator / divisor);
            int denominator = Denominator / divisor;

            int integer = Math.DivRem(
                numerator,
                denominator,
                out numerator);

            if (Numerator >= 0)
            {
                return new Rational {
                    Integer = integer,
                    Numerator = numerator,
                    Denominator = denominator };
            }

            if (integer == 0)
            {
                return new Rational {
                    Integer = 0,
                    Numerator = -numerator,
                    Denominator = denominator };
            }

            return new Rational {
                Integer = -integer,
                Numerator = numerator,
                Denominator = denominator };
        }

        /// <summary>
        /// Calculate the sum of two given ratios.
        /// </summary>
        /// <param name="first">
        /// The left operand.
        /// </param>
        /// <param name="second">
        /// The right operand.
        /// </param>
        /// <returns>
        /// The sum of two given ratios.
        /// </returns>
        public static Ratio operator+(Ratio first, Ratio second)
        {
            return new Ratio(
                first.Numerator * second.Denominator + second.Numerator * first.Denominator,
                first.Denominator * second.Denominator);
        }

        /// <summary>
        /// Calculate the difference of two given ratios.
        /// </summary>
        /// <param name="first">
        /// The left operand.
        /// </param>
        /// <param name="second">
        /// The right operand.
        /// </param>
        /// <returns>
        /// The difference of two given ratios.
        /// </returns>
        public static Ratio operator-(Ratio first, Ratio second)
        {
            return new Ratio(
                first.Numerator * second.Denominator - second.Numerator * first.Denominator,
                first.Denominator * second.Denominator);
        }

        /// <summary>
        /// Calculate the production of two given ratios.
        /// </summary>
        /// <param name="first">
        /// The left operand.
        /// </param>
        /// <param name="second">
        /// The right operand.
        /// </param>
        /// <returns>
        /// The production of two given ratios.
        /// </returns>
        public static Ratio operator*(Ratio first, Ratio second)
        {
            return new Ratio(
                first.Numerator * second.Numerator,
                first.Denominator * second.Denominator);
        }

        /// <summary>
        /// Calculate the division of two given ratios.
        /// </summary>
        /// <param name="first">
        /// The left operand.
        /// </param>
        /// <param name="second">
        /// The right operand.
        /// </param>
        /// <returns>
        /// The division of two given ratios.
        /// </returns>
        public static Ratio operator/(Ratio first, Ratio second)
        {
            if (second.Numerator == 0)
            {
                throw new DivideByZeroException();
            }

            return new Ratio(
                first.Numerator * second.Denominator,
                first.Denominator * second.Numerator);
        }
    }
}
