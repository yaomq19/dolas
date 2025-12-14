using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using Dolas.AssetEditor.ViewModels.Json;

namespace Dolas.AssetEditor.Converters;

/// <summary>
/// 把 JsonEditorType 映射到 Visibility；ConverterParameter 填目标枚举名（例如 "Bool"、"Vector3"）。
/// </summary>
public sealed class EditorTypeToVisibilityConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is not JsonEditorType t) return Visibility.Collapsed;
        if (parameter is not string s) return Visibility.Collapsed;
        return string.Equals(t.ToString(), s, StringComparison.OrdinalIgnoreCase)
            ? Visibility.Visible
            : Visibility.Collapsed;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        => throw new NotSupportedException();
}


