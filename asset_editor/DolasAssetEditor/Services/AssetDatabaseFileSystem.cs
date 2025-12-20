using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using Dolas.AssetEditor.Models;

namespace Dolas.AssetEditor.Services;

public sealed class AssetDatabaseFileSystem : IAssetDatabase
{
    private readonly string _contentRoot;

    public AssetDatabaseFileSystem(string contentRoot)
    {
        _contentRoot = contentRoot;
    }

    public IReadOnlyList<AssetItem> GetRootItems()
    {
        if (!Directory.Exists(_contentRoot))
            return Array.Empty<AssetItem>();

        var root = new AssetItem
        {
            Name = "content",
            FullPath = _contentRoot,
            IsDirectory = true
        };

        foreach (var dir in Directory.EnumerateDirectories(_contentRoot).OrderBy(Path.GetFileName))
        {
            root.Children.Add(BuildDirectoryTree(dir));
        }

        return new[] { root };
    }

    private AssetItem BuildDirectoryTree(string dirPath)
    {
        var node = new AssetItem
        {
            Name = Path.GetFileName(dirPath) ?? dirPath,
            FullPath = dirPath,
            IsDirectory = true
        };

        foreach (var subDir in Directory.EnumerateDirectories(dirPath).OrderBy(Path.GetFileName))
        {
            node.Children.Add(BuildDirectoryTree(subDir));
        }

        foreach (var file in Directory.EnumerateFiles(dirPath).OrderBy(Path.GetFileName))
        {
            // 只收集可解析为 XML 的资源文件（例如 *.camera/*.entity/*.material/*.scene 等）
            if (!LooksLikeXmlAsset(file))
                continue;

            node.Children.Add(new AssetItem
            {
                Name = Path.GetFileName(file) ?? file,
                FullPath = file,
                IsDirectory = false
            });
        }

        return node;
    }

    private static bool LooksLikeXmlAsset(string filePath)
    {
        // 快速跳过明显不是 XML 资产的大头（比如 hlsl/hlsli）
        var ext = Path.GetExtension(filePath).ToLowerInvariant();
        if (ext is ".hlsl" or ".hlsli" or ".dds" or ".hdr" or ".jpg" or ".png" or ".mtl" or ".obj")
            return false;

        try
        {
            var text = File.ReadAllText(filePath);
            _ = XDocument.Parse(text);
            return true;
        }
        catch
        {
            return false;
        }
    }
}


