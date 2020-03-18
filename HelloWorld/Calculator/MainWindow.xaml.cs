using System;
using System.Collections.ObjectModel;
using System.Windows;

namespace Calculator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        /// <summary>
        /// Caption for the error message displayed if
        /// the user formed an invalid expression.
        /// </summary>
        const string errorCaption = "Error";

        /// <summary>
        /// The members of the expression.
        /// </summary>
        private ObservableCollection<RationalOrOperation> items
            = new ObservableCollection<RationalOrOperation> {
                new RationalOrOperation(
                    new Rational {
                        Integer = 5,
                        Numerator = 3,
                        Denominator = 4 }),
                new RationalOrOperation(
                    Operation.Minus()),
                new RationalOrOperation(
                    new Rational {
                        Integer = 6,
                        Numerator = 2,
                        Denominator = 3 })
            };

        /// <summary>
        /// Creates a new instance of the
        /// <see cref="MainWindow" /> class.
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();
            actions.ItemsSource = items;

            Rational example = Rational.CreateReadonly();
            example.Integer = 0;
            example.Numerator = -11;
            example.Denominator = 12;
            result.Content = example;
        }

        /// <summary>
        /// Removes the member of the expression.
        /// </summary>
        /// <param name="sender">
        /// The Delete button corresponding to one
        /// of the expression members.
        /// </param>
        /// <param name="e">
        /// This parameter is ignored.
        /// </param>
        private void DeleteClick(object sender, RoutedEventArgs e)
        {
            items.Remove((RationalOrOperation)
                ((FrameworkElement)sender)
                    .DataContext);
        }

        /// <summary>
        /// Adds a plus operation and an operand equal to 0.
        /// </summary>
        /// <param name="sender">
        /// This parameter is ignored.
        /// </param>
        /// <param name="e">
        /// This parameter is ignored.
        /// </param>
        private void PlusClick(object sender, RoutedEventArgs e)
        {
            items.Add(new RationalOrOperation(Operation.Plus()));
            items.Add(
                new RationalOrOperation(
                    new Rational {
                        Integer = 0,
                        Numerator = 0,
                        Denominator = 1 } ));
        }

        /// <summary>
        /// Adds a minus operation and an operand equal to 0.
        /// </summary>
        /// <param name="sender">
        /// This parameter is ignored.
        /// </param>
        /// <param name="e">
        /// This parameter is ignored.
        /// </param>
        private void MinusClick(object sender, RoutedEventArgs e)
        {
            items.Add(new RationalOrOperation(Operation.Minus()));
            items.Add(
                new RationalOrOperation(
                    new Rational {
                        Integer = 0,
                        Numerator = 0,
                        Denominator = 1 } ));
        }

        /// <summary>
        /// Adds a multiply operation and an operand equal to 1.
        /// </summary>
        /// <param name="sender">
        /// This parameter is ignored.
        /// </param>
        /// <param name="e">
        /// This parameter is ignored.
        /// </param>
        private void MultiplyClick(object sender, RoutedEventArgs e)
        {
            items.Add(new RationalOrOperation(Operation.Multiply()));
            items.Add(
                new RationalOrOperation(
                    new Rational {
                        Integer = 1,
                        Numerator = 0,
                        Denominator = 1 } ));
        }

        /// <summary>
        /// Adds a division operation and an operand equal to 1.
        /// </summary>
        /// <param name="sender">
        /// This parameter is ignored.
        /// </param>
        /// <param name="e">
        /// This parameter is ignored.
        /// </param>
        private void DivideClick(object sender, RoutedEventArgs e)
        {
            items.Add(new RationalOrOperation(Operation.Divide()));
            items.Add(
                new RationalOrOperation(
                    new Rational {
                        Integer = 1,
                        Numerator = 0,
                        Denominator = 1 } ));
        }

        /// <summary>
        /// Converts the given <see cref="Rational" />
        /// value to an equivalent <see cref="Ratio"/>.
        /// Displays a message box and returns <c>null</c>
        /// if the conversion was not successful.
        /// </summary>
        /// <param name="value">
        /// The value to convert.
        /// </param>
        /// <returns>
        /// The converted value, or <c>null</c> if the
        /// conversion was not successful.
        /// </returns>
        private static Ratio TryGetRatio(Rational value)
        {
            try
            {
                return new Ratio(value);
            }
            catch (Exception e)
            {
                MessageBox.Show(
                    e.Message,
                    errorCaption,
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);

                return null;
            }
        }

        /// <summary>
        /// Calculates the expression formed by the user.
        /// Updates the <see cref="result" />, if the calculation
        /// was ok. Displays a message box in case of an error.
        /// </summary>
        /// <param name="sender">
        /// This parameter is ignored.
        /// </param>
        /// <param name="e">
        /// This parameter is ignored.
        /// </param>
        private void CalculateClick(object sender, RoutedEventArgs e)
        {
            if (items.Count == 0)
            {
                MessageBox.Show(
                    "Expression must not be empty",
                    errorCaption,
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);

                return;
            }

            Ratio currentSum = new Ratio(0, 1);
            bool allowNoSign = true;
            int i = 0;
            while (i < items.Count)
            {
                RationalOrOperation current = items[i];
                Ratio currentMultiplication;
                Ratio sign = new Ratio(1, 1);
                if (current.Value is Rational rational)
                {
                    if (allowNoSign)
                    {
                        currentMultiplication = TryGetRatio(rational);
                        if (currentMultiplication == null)
                        {
                            return;
                        }
                    }
                    else
                    {
                        MessageBox.Show(
                            "A plus or minus is expected",
                            errorCaption,
                            MessageBoxButton.OK,
                            MessageBoxImage.Error);
                        return;
                    }
                }
                else
                {
                    var operation = (Operation)current.Value;
                    if (operation.Name == Operation.Plus().Name)
                    {
                    }
                    else if (operation.Name == Operation.Minus().Name)
                    {
                        sign = new Ratio(-1, 1);
                    }
                    else
                    {
                        MessageBox.Show(
                            "Only addition and subtraction are expected here",
                            errorCaption,
                            MessageBoxButton.OK,
                            MessageBoxImage.Error);
                        return;
                    }

                    i++;
                    if (i >= items.Count || items[i].Value is Operation)
                    {
                        MessageBox.Show(
                            "The expression ends unexpectedly. Number expected.",
                            errorCaption,
                            MessageBoxButton.OK,
                            MessageBoxImage.Error);
                        return;
                    }

                    currentMultiplication = TryGetRatio((Rational)items[i].Value);
                    if (currentMultiplication == null)
                    {
                        return;
                    }
                }
                i++;
                while (i < items.Count)
                {
                    bool multiply;

                    if (i >= items.Count || items[i].Value is Rational)
                    {
                        MessageBox.Show(
                            "The expression ends unexpectedly. An operation sign expected.",
                            errorCaption,
                            MessageBoxButton.OK,
                            MessageBoxImage.Error);
                        return;
                    }

                    var multiplicative = (Operation)items[i].Value;
                    if (multiplicative.Name == Operation.Multiply().Name)
                    {
                        multiply = true;
                    }
                    else if (multiplicative.Name == Operation.Divide().Name)
                    {
                        multiply = false;
                    }
                    else
                    {
                        break;
                    }

                    i++;
                    if (i >= items.Count || items[i].Value is Operation)
                    {
                        MessageBox.Show(
                            "The expression ends unexpectedly. A number sign expected.",
                            errorCaption,
                            MessageBoxButton.OK,
                            MessageBoxImage.Error);
                        return;
                    }

                    Ratio operand = TryGetRatio((Rational)items[i].Value);
                    if (operand == null)
                    {
                        return;
                    }

                    if (multiply)
                    {
                        currentMultiplication *= operand;
                    }
                    else
                    {
                        try
                        {
                            currentMultiplication /= operand;
                        }
                        catch (DivideByZeroException)
                        {
                            MessageBox.Show(
                                "Division by zero occurred.",
                                errorCaption,
                                MessageBoxButton.OK,
                                MessageBoxImage.Error);
                            return;
                        }
                    }

                    i++;
                }

                currentSum += sign * currentMultiplication;
                allowNoSign = false;
            }

            result.Content = currentSum
                .AsRational()
                .AsReadOnly();
        }
    }
}
