using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Text.Json.Nodes;
using System.Windows;
using Microsoft.Win32;
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
        NewAssetCommand = new RelayCommand(CreateNewAsset, () => CanCreateNewAsset);
        NewAssetFromSchemaCommand = new RelayCommand<RsdSchema>(CreateNewAssetFromSchema, _ => CanCreateNewAsset);

        AssetRoots = new ObservableCollection<AssetItem>(_assetDatabase.GetRootItems());
        AvailableSchemas = new ObservableCollection<RsdSchema>(_rsdRegistry?.AllSchemas ?? Array.Empty<RsdSchema>());
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

    public ObservableCollection<RsdSchema> AvailableSchemas { get; }

    public ObservableCollection<string> Logs { get; }

    public ICommand OpenProjectCommand { get; }
    public ICommand ExitCommand { get; }
    public ICommand AboutCommand { get; }
    public ICommand SaveCommand { get; }
    public ICommand ReloadCommand { get; }
    public ICommand NewAssetCommand { get; }
    public ICommand NewAssetFromSchemaCommand { get; }

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

    public bool CanCreateNewAsset => _repoRoot is not null && _rsdRegistry is not null && AvailableSchemas.Count > 0;

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

        // 重复选择同一个节点时，不弹提示也不重新加载
        if (!string.IsNullOrWhiteSpace(SelectedAssetPath) &&
            string.Equals(SelectedAssetPath, item.FullPath, StringComparison.OrdinalIgnoreCase))
            return;

        // 切换资产前：如果当前有未保存修改，提示是否保存
        if (!PromptSaveIfDirty())
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

    private bool PromptSaveIfDirty()
    {
        if (!IsDirty || !CanReload) return true;

        var result = MessageBox.Show(
            "当前资产有未保存的改动，是否先保存？\n\n- 选择“是”：保存后继续\n- 选择“否”：放弃改动并继续\n- 选择“取消”：中止本次操作",
            "未保存的改动",
            MessageBoxButton.YesNoCancel,
            MessageBoxImage.Warning);

        if (result == MessageBoxResult.Cancel)
            return false;

        if (result == MessageBoxResult.Yes)
        {
            SaveSelectedAsset();
            // 如果保存失败，IsDirty 仍为 true；此时中止后续操作更安全
            return !IsDirty;
        }

        // No => discard
        IsDirty = false;
        return true;
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

            _matchedSchema = schema;

            // XML on disk -> JsonObject in memory（复用现有类型化编辑 UI）
            var rootObj = XmlAssetCodec.LoadAssetAsJsonObject(filePath, schema);
            var (boundRoot, vm, mutated, warnings) = RsdAssetBinder.BindToSchema(rootObj, schema, MarkDirty);
            _jsonRootNode = boundRoot;
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
            if (_matchedSchema is null)
                throw new InvalidOperationException("未匹配到 schema，无法保存。");
            if (_jsonRootNode is not JsonObject obj)
                throw new InvalidOperationException("根节点不是 object，无法保存。");

            XmlAssetCodec.SaveAssetFromJsonObject(SelectedAssetPath, _matchedSchema, obj);
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

    private void RefreshAssetTree()
    {
        AssetRoots.Clear();
        foreach (var r in _assetDatabase.GetRootItems())
            AssetRoots.Add(r);
    }

    private void CreateNewAsset()
    {
        // UI 上点击 "New Asset" 时，仅负责弹出下拉；真正创建走 CreateNewAssetFromSchema
        StatusText = "请选择一种 RSD 进行创建...";
    }

    private void CreateNewAssetFromSchema(RsdSchema? schema)
    {
        if (schema is null)
            return;

        if (!PromptSaveIfDirty())
            return;

        try
        {
            var contentRoot = RepoLocator.TryFindContentRoot();
            var dlg = new SaveFileDialog
            {
                Title = "新建资产 - 选择保存路径与文件名",
                AddExtension = true,
                DefaultExt = schema.FileSuffix,
                Filter = $"{schema.FileSuffix} 资产|*{schema.FileSuffix}|所有文件|*.*",
                InitialDirectory = contentRoot ?? _repoRoot ?? Environment.CurrentDirectory,
                FileName = $"new{schema.FileSuffix}"
            };

            if (dlg.ShowDialog() != true)
                return;

            var filePath = dlg.FileName;
            var dir = Path.GetDirectoryName(filePath);
            if (!string.IsNullOrWhiteSpace(dir))
                Directory.CreateDirectory(dir);

            // 用 schema 默认值生成一个可编辑对象，并写入磁盘（XML）
            var defaultObj = RsdAssetBinder.CreateDefaultAssetObject(schema);
            XmlAssetCodec.SaveAssetFromJsonObject(filePath, schema, defaultObj);

            // 刷新左侧树并打开新资产
            RefreshAssetTree();

            SelectedAssetName = Path.GetFileName(filePath) ?? filePath;
            SelectedAssetPath = filePath;
            LoadAssetByRsd(filePath);
            IsDirty = false;
            StatusText = $"已创建并打开（{schema.ClassName}）";
            Logs.Add($"新建资产：{filePath} ({schema.ClassName})");
        }
        catch (Exception ex)
        {
            StatusText = "新建失败";
            Logs.Add($"新建失败：{schema.ClassName} -> {ex.Message}");
        }
    }
}


