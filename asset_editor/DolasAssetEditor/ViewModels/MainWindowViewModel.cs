using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using System.Windows.Input;
using Dolas.AssetEditor.Commands;
using Dolas.AssetEditor.Models;
using Dolas.AssetEditor.Services;
using Dolas.AssetEditor.ViewModels.Json;

namespace Dolas.AssetEditor.ViewModels;

public sealed class MainWindowViewModel : ViewModelBase
{
    private readonly IAssetDatabase _assetDatabase;
    private readonly string? _repoRoot;
    private readonly RsdSchemaRegistry? _rsdRegistry;
    private RsdSchema? _matchedSchema;

    private string _statusText = "就绪";
    private string _assetFilterText = string.Empty;
    private string _selectedAssetName = "（未选择）";
    private string _selectedAssetPath = string.Empty;
    private bool _isDirty;
    private JsonNode? _jsonRootNode;
    private JsonNodeViewModel? _jsonRoot;

    public MainWindowViewModel()
        : this(CreateDefaultAssetDatabase())
    {
    }

    public MainWindowViewModel(IAssetDatabase assetDatabase)
    {
        _assetDatabase = assetDatabase;

        _repoRoot = RepoLocator.TryFindRepoRoot(AppContext.BaseDirectory);
        if (_repoRoot is not null)
        {
            try
            {
                _rsdRegistry = RsdSchemaRegistry.LoadFromRepoRoot(_repoRoot);
            }
            catch
            {
                _rsdRegistry = null;
            }
        }

        OpenProjectCommand = new RelayCommand(OpenProject);
        ExitCommand = new RelayCommand(() => Application.Current.Shutdown());
        AboutCommand = new RelayCommand(ShowAbout);
        SaveCommand = new RelayCommand(SaveSelectedAsset, () => CanSave);
        ReloadCommand = new RelayCommand(ReloadSelectedAsset, () => CanReload);

        AssetRoots = new ObservableCollection<AssetItem>(_assetDatabase.GetRootItems());
        Logs = new ObservableCollection<string>
        {
            "DolasAssetEditor 启动完成。",
            "提示：这是基础框架，后续可接入 content/ 扫描、引擎资源导入与预览。"
        };

        if (_repoRoot is null)
            Logs.Add("警告：未定位到仓库根目录（无法读取 rsd/*.rsd 进行类型匹配）。");
        else if (_rsdRegistry is null)
            Logs.Add("警告：rsd schema 加载失败（无法按后缀匹配资产类型）。");
    }

    public ObservableCollection<AssetItem> AssetRoots { get; }

    public ObservableCollection<string> Logs { get; }

    public ICommand OpenProjectCommand { get; }
    public ICommand ExitCommand { get; }
    public ICommand AboutCommand { get; }
    public ICommand SaveCommand { get; }
    public ICommand ReloadCommand { get; }

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

    public string SelectedAssetPath
    {
        get => _selectedAssetPath;
        private set => SetField(ref _selectedAssetPath, value);
    }

    public JsonNodeViewModel? JsonRoot
    {
        get => _jsonRoot;
        private set => SetField(ref _jsonRoot, value);
    }

    public bool IsDirty
    {
        get => _isDirty;
        private set
        {
            if (!SetField(ref _isDirty, value)) return;
            // 触发按钮状态刷新
            (SaveCommand as RelayCommand)?.RaiseCanExecuteChanged();
        }
    }

    public bool HasSelectedJsonAsset => JsonRoot is not null;

    public bool CanSave => HasSelectedJsonAsset && IsDirty && !string.IsNullOrWhiteSpace(SelectedAssetPath);

    public bool CanReload => HasSelectedJsonAsset && !string.IsNullOrWhiteSpace(SelectedAssetPath);

    private void OpenProject()
    {
        // 基础框架：后续这里可以打开引擎工程文件（或选择 content 根目录）
        StatusText = "打开工程（占位）";
        Logs.Add("OpenProject: TODO");
    }

    public void SelectAsset(AssetItem? item)
    {
        if (item is null)
            return;

        SelectedAssetName = item.Name;
        SelectedAssetPath = item.FullPath;

        if (item.IsDirectory)
        {
            JsonRoot = null;
            _jsonRootNode = null;
            _matchedSchema = null;
            IsDirty = false;
            OnPropertyChanged(nameof(HasSelectedJsonAsset));
            (ReloadCommand as RelayCommand)?.RaiseCanExecuteChanged();
            return;
        }

        LoadAssetByRsd(item.FullPath);
    }

    private void LoadAssetByRsd(string filePath)
    {
        try
        {
            if (_repoRoot is null || _rsdRegistry is null)
                throw new InvalidOperationException("无法定位/加载 rsd 目录：请确保从仓库 build/ 生成并运行，且根目录存在 rsd/。");

            var suffix = Path.GetExtension(filePath);
            if (string.IsNullOrWhiteSpace(suffix))
                throw new InvalidOperationException($"无法获取资产后缀名：{filePath}");

            if (!_rsdRegistry.TryGetBySuffix(suffix, out var schema) || schema is null)
                throw new InvalidOperationException($"没有任何 .rsd 的 file_suffix 匹配该资产：{suffix}（{filePath}）");

            var text = File.ReadAllText(filePath);
            var rootNode = JsonNode.Parse(text);
            if (rootNode is null)
                throw new InvalidOperationException("JsonNode.Parse returned null");

            _matchedSchema = schema;
            var (rootObj, vm, mutated, warnings) = RsdAssetBinder.BindToSchema(rootNode, schema, MarkDirty);
            _jsonRootNode = rootObj;
            JsonRoot = vm;

            if (!mutated) IsDirty = false;
            StatusText = $"已加载（{schema.ClassName} / {schema.FileSuffix}）";
            Logs.Add($"加载资产：{filePath} -> schema: {schema.ClassName} ({schema.FileSuffix})");
            foreach (var w in warnings)
                Logs.Add($"提示：{w}");
        }
        catch (Exception ex)
        {
            JsonRoot = null;
            _jsonRootNode = null;
            _matchedSchema = null;
            IsDirty = false;
            StatusText = "加载失败";
            Logs.Add($"加载失败：{filePath} -> {ex.Message}");
        }
        finally
        {
            OnPropertyChanged(nameof(HasSelectedJsonAsset));
            (ReloadCommand as RelayCommand)?.RaiseCanExecuteChanged();
            (SaveCommand as RelayCommand)?.RaiseCanExecuteChanged();
        }
    }

    private void MarkDirty() => IsDirty = true;

    private void SaveSelectedAsset()
    {
        if (_jsonRootNode is null || string.IsNullOrWhiteSpace(SelectedAssetPath))
            return;

        try
        {
            var json = _jsonRootNode.ToJsonString(new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(SelectedAssetPath, json);
            IsDirty = false;
            StatusText = "已保存";
            Logs.Add($"保存成功：{SelectedAssetPath}");
        }
        catch (Exception ex)
        {
            StatusText = "保存失败";
            Logs.Add($"保存失败：{SelectedAssetPath} -> {ex.Message}");
        }
    }

    private void ReloadSelectedAsset()
    {
        if (string.IsNullOrWhiteSpace(SelectedAssetPath))
            return;
        LoadAssetByRsd(SelectedAssetPath);
    }

    private void ShowAbout()
    {
        MessageBox.Show(
            "Dolas 资产编辑器（基础框架）\n\n后续可接入：资源导入、预览视口、属性编辑、与引擎通信。",
            "关于",
            MessageBoxButton.OK,
            MessageBoxImage.Information);
    }

    private static IAssetDatabase CreateDefaultAssetDatabase()
    {
        var content = RepoLocator.TryFindContentRoot();
        return content is null ? new AssetDatabaseStub() : new AssetDatabaseFileSystem(content);
    }
}


