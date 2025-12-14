using System.Collections.Generic;
using Dolas.AssetEditor.Models;

namespace Dolas.AssetEditor.Services;

public interface IAssetDatabase
{
    IReadOnlyList<AssetItem> GetRootItems();
}


