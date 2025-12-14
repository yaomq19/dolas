using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json.Nodes;
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
            // 只收集可解析为 JSON 的资源文件（例如 *.camera/*.entity/*.material/*.scene 等）
            if (!LooksLikeJsonAsset(file))
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

    private static bool LooksLikeJsonAsset(string filePath)
    {
        // 快速跳过明显不是 JSON 的大头（比如 hlsl/hlsli）
        var ext = Path.GetExtension(filePath).ToLowerInvariant();
        if (ext is ".hlsl" or ".hlsli" or ".dds" or ".hdr" or ".jpg" or ".png" or ".mtl" or ".obj")
            return false;

        try
        {
            // 有些资源扩展名不是 .json，但内容是 JSON；这里直接尝试解析。
            var text = File.ReadAllText(filePath);
            _ = JsonNode.Parse(text);
            return true;
        }
        catch
        {
            return false;
        }
    }
}


