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

    private void NewAssetButton_Click(object sender, RoutedEventArgs e)
    {
        if (sender is not Button btn)
            return;

        // 左键点击按钮时弹出下拉菜单（ContextMenu）
        if (btn.ContextMenu is null)
            return;

        btn.ContextMenu.PlacementTarget = btn;
        btn.ContextMenu.IsOpen = true;
        e.Handled = true;
    }

    private void AssetTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (DataContext is not MainWindowViewModel vm)
            return;

        vm.SelectAsset(e.NewValue as AssetItem);
    }
}


