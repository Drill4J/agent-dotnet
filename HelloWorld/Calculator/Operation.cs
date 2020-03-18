using System.Collections.Generic;
using System.Collections.Immutable;

namespace Calculator
{
    /// <summary>
    /// Represents an arithmetical operation.
    /// </summary>
    public class Operation
    {
        /// <summary>
        /// The list of possible operation signs.
        /// </summary>
        public static ImmutableList<string> PossibleOperations = ImmutableList<string>.Empty
            .Add("+")
            .Add("-")
            .Add("*")
            .Add("/");

        /// <summary>
        /// The selected operation sign.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Creates an operation with the given sign.
        /// </summary>
        /// <param name="name">
        /// The sign of the operation.
        /// </param>
        private Operation(string name)
        {
            Name = name;
        }

        /// <summary>
        /// Creates an operation object for addition.
        /// </summary>
        /// <returns>
        /// The new operation object.
        /// </returns>
        public static Operation Plus()
        {
            return new Operation("+");
        }

        /// <summary>
        /// Creates an operation object for subtraction.
        /// </summary>
        /// <returns>
        /// The new operation object.
        /// </returns>
        public static Operation Minus()
        {
            return new Operation("-");
        }

        /// <summary>
        /// Creates an operation object for multiplication.
        /// </summary>
        /// <returns>
        /// The new operation object.
        /// </returns>
        public static Operation Multiply()
        {
            return new Operation("*");
        }

        /// <summary>
        /// Creates an operation object for division.
        /// </summary>
        /// <returns>
        /// The new operation object.
        /// </returns>
        public static Operation Divide()
        {
            return new Operation("/");
        }

        /// <inheritdoc />
        public override string ToString()
        {
            return Name;
        }
    }
}
