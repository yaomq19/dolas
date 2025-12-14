using System.Collections.ObjectModel;

namespace Dolas.AssetEditor.Models;

public sealed class AssetItem
{
    public string Name { get; init; } = string.Empty;

    /// <summary>
    /// 绝对路径：目录或文件。
    /// </summary>
    public string FullPath { get; init; } = string.Empty;

    public bool IsDirectory { get; init; }

    public ObservableCollection<AssetItem> Children { get; } = new();
}


