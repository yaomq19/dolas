using System.Windows;
using System.Windows.Controls;
using Dolas.AssetEditor.Models;
using Dolas.AssetEditor.ViewModels;

namespace Dolas.AssetEditor;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
    }

    private void AssetTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (DataContext is not MainWindowViewModel vm)
            return;

        vm.SelectAsset(e.NewValue as AssetItem);
    }
}


