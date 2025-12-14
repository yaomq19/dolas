using System.Collections.Generic;
using Dolas.AssetEditor.Models;

namespace Dolas.AssetEditor.Services;

/// <summary>
/// 占位用资产数据库：后续可以替换为扫描 `content/` 或读取引擎工程文件的实现。
/// </summary>
public sealed class AssetDatabaseStub : IAssetDatabase
{
    public IReadOnlyList<AssetItem> GetRootItems()
    {
        var root = new AssetItem { Name = "content" };

        root.Children.Add(new AssetItem { Name = "scene" });
        root.Children.Add(new AssetItem { Name = "entity" });
        root.Children.Add(new AssetItem { Name = "material" });
        root.Children.Add(new AssetItem { Name = "shader" });
        root.Children.Add(new AssetItem { Name = "model" });
        root.Children.Add(new AssetItem { Name = "texture" });

        return new[] { root };
    }
}


