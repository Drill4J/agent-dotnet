namespace Calculator
{
    /// <summary>
    /// Stores a member of the expression to calculate.
    /// The member can be a number or an operation.
    /// </summary>
    public class RationalOrOperation
    {
        /// <summary>
        /// Stores the value of the <see cref="Value" /> property.
        /// </summary>
        private object value;

        /// <summary>
        /// Creates a new instance of the
        /// <see cref="RationalOrOperation" /> object
        /// storing the given value.
        /// </summary>
        /// <param name="rational">
        /// The <see cref="Rational" /> number to store.
        /// </param>
        public RationalOrOperation(Rational rational)
        {
            value = rational;
        }

        /// <summary>
        /// Creates a new instance of the
        /// <see cref="RationalOrOperation" /> object
        /// storing the given value.
        /// </summary>
        /// <param name="operation">
        /// The <see cref="Operation" /> to store.
        /// </param>
        public RationalOrOperation(Operation operation)
        {
            value = operation;
        }

        /// <summary>
        /// The member of the expression.
        /// Can be a <see cref="Rational" />
        /// or an <see cref="Operation" />.
        /// </summary>
        public object Value => value;
    }
}
