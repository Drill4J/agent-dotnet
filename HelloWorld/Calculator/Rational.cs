using System;

namespace Calculator
{
    /// <summary>
    /// Represents a rational number as an
    /// integral part, numerator and denominator.
    /// </summary>
    public class Rational
    {
        /// <summary>
        /// Create a value equal to 0.
        /// </summary>
        public Rational()
            : this(false)
        {
        }

        /// <summary>
        /// Creates a value equal to 0 and sets
        /// the <see cref="IsReadonly" /> flag.
        /// </summary>
        /// <param name="isReadonly">
        /// The value for the <see cref="IsReadonly" /> flag.
        /// </param>
        private Rational(bool isReadonly)
        {
            IsReadonly = isReadonly;
            Denominator = 1;
        }

        /// <summary>
        /// Creates a new value with
        /// <see cref="IsReadonly" /> flag set.
        /// </summary>
        /// <returns>
        /// A value equal to 0 with
        /// <see cref="IsReadonly" /> flag set.
        /// </returns>
        public static Rational CreateReadonly()
        {
            return new Rational(true);
        }

        /// <summary>
        /// Creates a copy of this value with
        /// <see cref="IsReadonly" /> flag set.
        /// </summary>
        /// <returns>
        /// A value equal to this value with
        /// <see cref="IsReadonly" /> flag set.
        /// </returns>
        public Rational AsReadOnly()
        {
            return new Rational(true) {
                Integer = Integer,
                Numerator = Numerator,
                Denominator = Denominator
            };
        }

        /// <summary>
        /// Flag indicating the user is not intended to
        /// edit content of this object via user interface.
        /// It does not prevent changing other properties
        /// from usual code, though.
        /// </summary>
        public bool IsReadonly { get; }

        /// <summary>
        /// The integer part of the number.
        /// </summary>
        public int Integer { get; set; }

        /// <summary>
        /// The top part of the fraction.
        /// </summary>
        public int Numerator { get; set; }

        /// <summary>
        /// The bottom part of the fraction.
        /// </summary>
        public int Denominator { get; set; }
    }
}
