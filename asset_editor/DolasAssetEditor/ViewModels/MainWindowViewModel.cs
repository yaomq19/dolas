using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using Dolas.AssetEditor.Commands;
using Dolas.AssetEditor.Models;
using Dolas.AssetEditor.Services;

namespace Dolas.AssetEditor.ViewModels;

public sealed class MainWindowViewModel : ViewModelBase
{
    private readonly IAssetDatabase _assetDatabase;

    private string _statusText = "就绪";
    private string _assetFilterText = string.Empty;
    private string _selectedAssetName = "（未选择）";

    public MainWindowViewModel()
        : this(new AssetDatabaseStub())
    {
    }

    public MainWindowViewModel(IAssetDatabase assetDatabase)
    {
        _assetDatabase = assetDatabase;

        OpenProjectCommand = new RelayCommand(OpenProject);
        ExitCommand = new RelayCommand(() => Application.Current.Shutdown());
        AboutCommand = new RelayCommand(ShowAbout);

        AssetRoots = new ObservableCollection<AssetItem>(_assetDatabase.GetRootItems());
        Logs = new ObservableCollection<string>
        {
            "DolasAssetEditor 启动完成。",
            "提示：这是基础框架，后续可接入 content/ 扫描、引擎资源导入与预览。"
        };
    }

    public ObservableCollection<AssetItem> AssetRoots { get; }

    public ObservableCollection<string> Logs { get; }

    public ICommand OpenProjectCommand { get; }
    public ICommand ExitCommand { get; }
    public ICommand AboutCommand { get; }

    public string StatusText
    {
        get => _statusText;
        set => SetField(ref _statusText, value);
    }

    public string AssetFilterText
    {
        get => _assetFilterText;
        set => SetField(ref _assetFilterText, value);
    }

    public string SelectedAssetName
    {
        get => _selectedAssetName;
        set => SetField(ref _selectedAssetName, value);
    }

    private void OpenProject()
    {
        // 基础框架：后续这里可以打开引擎工程文件（或选择 content 根目录）
        StatusText = "打开工程（占位）";
        Logs.Add("OpenProject: TODO");
    }

    private void ShowAbout()
    {
        MessageBox.Show(
            "Dolas 资产编辑器（基础框架）\n\n后续可接入：资源导入、预览视口、属性编辑、与引擎通信。",
            "关于",
            MessageBoxButton.OK,
            MessageBoxImage.Information);
    }
}


