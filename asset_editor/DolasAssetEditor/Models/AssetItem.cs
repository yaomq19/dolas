using System.Collections.ObjectModel;

namespace Dolas.AssetEditor.Models;

public sealed class AssetItem
{
    public string Name { get; init; } = string.Empty;

    public ObservableCollection<AssetItem> Children { get; } = new();
}


