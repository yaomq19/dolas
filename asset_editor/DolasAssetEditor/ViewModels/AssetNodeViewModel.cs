using System;
using System.Collections.ObjectModel;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Windows.Input;
using Dolas.AssetEditor.Commands;
using Dolas.AssetEditor.Services;

namespace Dolas.AssetEditor.ViewModels;

public sealed class AssetNodeViewModel : ViewModelBase
{
    private static string? _lastOpenedDirectory;
    private readonly Action? _markDirty;

    private string _scalarText = string.Empty;
    private bool _boolValue;
    private string _enumSelectedName = string.Empty;
    private bool _suppressEnumWrite;
    private string _xText = "0";
    private string _yText = "0";
    private string _zText = "0";
    private string _wText = "0";
    private string _targetRsdClassName = string.Empty;

    public AssetNodeViewModel(string name, EditorType editorType, Action? markDirty)
    {
        Name = name;
        EditorType = editorType;
        _markDirty = markDirty;
        Children = new ObservableCollection<AssetNodeViewModel>();
        BrowseCommand = new RelayCommand(Browse);
    }

    public string Name { get; }

    public EditorType EditorType { get; private set; }

    public string TargetRsdClassName
    {
        get => _targetRsdClassName;
        set => SetField(ref _targetRsdClassName, value);
    }

    public ICommand BrowseCommand { get; }

    private void Browse()
    {
        var contentRoot = RepoLocator.TryFindContentRoot();
        if (contentRoot == null)
        {
            System.Windows.MessageBox.Show("未找到引擎 content 目录，无法确定相对路径。", "错误", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            return;
        }

        var filter = "所有文件|*.*";
        if (EditorType == EditorType.AssetReference && !string.IsNullOrWhiteSpace(TargetRsdClassName))
        {
            var repoRoot = RepoLocator.TryFindRepoRoot(AppContext.BaseDirectory);
            if (repoRoot != null)
            {
                var registry = RsdSchemaRegistry.LoadFromRepoRoot(repoRoot);
                if (registry.TryGetByClassName(TargetRsdClassName, out var schema) && schema?.FileSuffix != null)
                {
                    var suffix = schema.FileSuffix;
                    filter = $"{TargetRsdClassName} 资产 (*{suffix})|*{suffix}|{filter}";
                }
            }
        }

        var dlg = new Microsoft.Win32.OpenFileDialog
        {
            Title = $"选择 {Name}",
            Filter = filter,
            InitialDirectory = (!string.IsNullOrEmpty(_lastOpenedDirectory) && Directory.Exists(_lastOpenedDirectory)) 
                ? _lastOpenedDirectory 
                : contentRoot
        };

        if (dlg.ShowDialog() == true)
        {
            var fullPath = dlg.FileName;
            if (!fullPath.StartsWith(contentRoot, StringComparison.OrdinalIgnoreCase))
            {
                System.Windows.MessageBox.Show("选中的文件必须位于 content 目录或其子目录下。", "路径错误", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
                return;
            }

            _lastOpenedDirectory = Path.GetDirectoryName(fullPath);

            var relativePath = Path.GetRelativePath(contentRoot, fullPath).Replace('\\', '/');
            ScalarText = relativePath;
        }
    }

    public ObservableCollection<AssetNodeViewModel> Children { get; }

    public ObservableCollection<RsdEnumValue> EnumValues { get; } = new();

    public string ScalarText
    {
        get => _scalarText;
        set
        {
            if (!SetField(ref _scalarText, value)) return;
            _markDirty?.Invoke();
        }
    }

    public bool BoolValue
    {
        get => _boolValue;
        set
        {
            if (!SetField(ref _boolValue, value)) return;
            _markDirty?.Invoke();
        }
    }

    public string EnumSelectedName
    {
        get => _enumSelectedName;
        set
        {
            if (!SetField(ref _enumSelectedName, value)) return;
            if (_suppressEnumWrite) return;
            if (EditorType != EditorType.Enum) return;
            _markDirty?.Invoke();
        }
    }

    public string XText
    {
        get => _xText;
        set
        {
            if (!SetField(ref _xText, value)) return;
            _markDirty?.Invoke();
        }
    }

    public string YText
    {
        get => _yText;
        set
        {
            if (!SetField(ref _yText, value)) return;
            _markDirty?.Invoke();
        }
    }

    public string ZText
    {
        get => _zText;
        set
        {
            if (!SetField(ref _zText, value)) return;
            _markDirty?.Invoke();
        }
    }

    public string WText
    {
        get => _wText;
        set
        {
            if (!SetField(ref _wText, value)) return;
            _markDirty?.Invoke();
        }
    }

    public string Summary
        => EditorType switch
        {
            EditorType.Object => $"Object ({Children.Count})",
            EditorType.Array => $"Array ({Children.Count})",
            EditorType.Vector3 => "Vector3",
            EditorType.Vector4 => "Vector4",
            EditorType.String => "String",
            EditorType.Enum => "Enum",
            EditorType.Bool => "Bool",
            EditorType.Int => "Int",
            EditorType.UInt => "UInt",
            EditorType.Float => "Float",
            EditorType.Null => "Null",
            _ => EditorType.ToString()
        };

    public void ForceEnumValues(System.Collections.Generic.IReadOnlyList<RsdEnumValue>? values)
    {
        EnumValues.Clear();
        if (values is not null)
        {
            foreach (var v in values)
                EnumValues.Add(v);
        }
    }

    public void SetAs(EditorType t)
    {
        if (EditorType == t) return;
        EditorType = t;
        OnPropertyChanged(nameof(EditorType));
        OnPropertyChanged(nameof(Summary));
    }

    public void SetScalar(string s, EditorType t)
    {
        SetAs(t);
        _scalarText = s ?? string.Empty;
        OnPropertyChanged(nameof(ScalarText));
    }

    public void SetBool(bool b)
    {
        SetAs(EditorType.Bool);
        _boolValue = b;
        OnPropertyChanged(nameof(BoolValue));
    }

    public void SetVector3(double x, double y, double z)
    {
        SetAs(EditorType.Vector3);
        _xText = x.ToString(CultureInfo.InvariantCulture);
        _yText = y.ToString(CultureInfo.InvariantCulture);
        _zText = z.ToString(CultureInfo.InvariantCulture);
        OnPropertyChanged(nameof(XText));
        OnPropertyChanged(nameof(YText));
        OnPropertyChanged(nameof(ZText));
    }

    public void SetVector4(double x, double y, double z, double w)
    {
        SetAs(EditorType.Vector4);
        _xText = x.ToString(CultureInfo.InvariantCulture);
        _yText = y.ToString(CultureInfo.InvariantCulture);
        _zText = z.ToString(CultureInfo.InvariantCulture);
        _wText = w.ToString(CultureInfo.InvariantCulture);
        OnPropertyChanged(nameof(XText));
        OnPropertyChanged(nameof(YText));
        OnPropertyChanged(nameof(ZText));
        OnPropertyChanged(nameof(WText));
    }

    public void SetEnumSelected(string name, System.Collections.Generic.IReadOnlyList<RsdEnumValue>? values)
    {
        SetAs(EditorType.Enum);
        ForceEnumValues(values);

        var canonical = name ?? string.Empty;
        var match = EnumValues.FirstOrDefault(ev =>
            string.Equals(ev.Name, canonical, StringComparison.OrdinalIgnoreCase) ||
            (!string.IsNullOrWhiteSpace(ev.Alias) && string.Equals(ev.Alias, canonical, StringComparison.OrdinalIgnoreCase)) ||
            (!string.IsNullOrWhiteSpace(ev.Display) && string.Equals(ev.Display, canonical, StringComparison.OrdinalIgnoreCase)));

        if (match is not null) canonical = match.Name;
        else if (EnumValues.Count > 0) canonical = EnumValues[0].Name;

        _suppressEnumWrite = true;
        _enumSelectedName = canonical;
        _suppressEnumWrite = false;
        OnPropertyChanged(nameof(EnumSelectedName));
    }
}


