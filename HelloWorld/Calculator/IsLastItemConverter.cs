using System;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace Calculator
{
    /// <summary>
    /// Converter determining whether the current item is the
    /// last, and not only, item in the list with expression.
    /// </summary>
    public class IsLastItemConverter : IMultiValueConverter
    {
        /// <summary>
        /// Determines whether the current item is the last,
        /// and not only, item in the list with expression.
        /// </summary>
        /// <param name="values">
        /// Accepts three values.
        /// The first one is the current <see cref="ListBoxItem" />.
        /// The second one is the <see cref="ObservableCollection{T}"/>
        /// of the <see cref="RationalOrOperation" /> tokens.
        /// The third one is count of the expression tokens.
        /// </param>
        /// <param name="targetType">
        /// This parameter is ignored.
        /// </param>
        /// <param name="parameter">
        /// This parameter is ignored.
        /// </param>
        /// <param name="culture">
        /// This parameter is ignored.
        /// </param>
        /// <returns>
        /// <c>True</c>, if the given <see cref="ListBoxItem" /> is the last,
        /// and not only, item if the given <see cref="ObservableCollection{T}" />.
        /// </returns>
        public object Convert(
            object[] values,
            Type targetType,
            object parameter,
            CultureInfo culture)
        {
            var item = (FrameworkElement)values[0];
            var list = (ObservableCollection<RationalOrOperation>)values[1];
            int count = (int)values[2];
            int index = list.IndexOf((RationalOrOperation)item.DataContext);
            return index > 0 && index == count - 1;
        }

        /// <summary>
        /// Conversion in this direction is not supported.
        /// </summary>
        /// <param name="value">
        /// This parameter is ignored.
        /// </param>
        /// <param name="targetTypes">
        /// This parameter is ignored.
        /// </param>
        /// <param name="parameter">
        /// This parameter is ignored.
        /// </param>
        /// <param name="culture">
        /// This parameter is ignored.
        /// </param>
        /// <returns>
        /// No value returned.
        /// </returns>
        /// <exception cref="NotSupportedException">
        /// Always thrown.
        /// </exception>
        public object[] ConvertBack(
            object value,
            Type[] targetTypes,
            object parameter,
            CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
