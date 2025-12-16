using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text.Json.Nodes;

namespace Dolas.AssetEditor.Services;

public sealed class RsdSchemaRegistry
{
    private readonly Dictionary<string, RsdSchema> _bySuffix;
    private readonly List<RsdSchema> _all;

    private RsdSchemaRegistry(List<RsdSchema> schemas)
    {
        _all = schemas;
        _bySuffix = schemas
            .GroupBy(s => NormalizeSuffix(s.FileSuffix))
            .ToDictionary(g => g.Key, g => g.First(), StringComparer.OrdinalIgnoreCase);
    }

    public IReadOnlyList<RsdSchema> AllSchemas => new ReadOnlyCollection<RsdSchema>(_all);

    public static RsdSchemaRegistry LoadFromRepoRoot(string repoRoot)
    {
        var rsdDir = Path.Combine(repoRoot, "rsd");
        if (!Directory.Exists(rsdDir))
            throw new DirectoryNotFoundException($"未找到 rsd 目录：{rsdDir}");

        var schemas = new List<RsdSchema>();
        foreach (var file in Directory.EnumerateFiles(rsdDir, "*.rsd").OrderBy(Path.GetFileName))
        {
            schemas.Add(LoadOne(file));
        }

        if (schemas.Count == 0)
            throw new InvalidOperationException($"rsd 目录下没有任何 .rsd 文件：{rsdDir}");

        return new RsdSchemaRegistry(schemas);
    }

    public bool TryGetBySuffix(string assetSuffix, out RsdSchema? schema)
    {
        schema = null;
        var key = NormalizeSuffix(assetSuffix);
        return _bySuffix.TryGetValue(key, out schema);
    }

    private static string NormalizeSuffix(string s)
    {
        s = s.Trim();
        if (!s.StartsWith('.')) s = "." + s;
        return s.ToLowerInvariant();
    }

    private static RsdSchema LoadOne(string rsdFile)
    {
        var text = File.ReadAllText(rsdFile);
        var node = JsonNode.Parse(text) as JsonObject
                   ?? throw new InvalidOperationException($"RSD 不是 JSON object：{rsdFile}");

        string? className = null;
        string? fileSuffix = null;
        var fields = new Dictionary<string, string>(StringComparer.Ordinal);

        foreach (var kv in node)
        {
            if (kv.Value is not JsonValue v || !v.TryGetValue<string>(out var typeStr))
                throw new InvalidOperationException($"RSD 字段值必须是字符串：{rsdFile} -> {kv.Key}");

            if (kv.Key == "class_name")
                className = typeStr;
            else if (kv.Key == "file_suffix")
                fileSuffix = typeStr;
            else
                fields[kv.Key] = typeStr;
        }

        if (string.IsNullOrWhiteSpace(className))
            throw new InvalidOperationException($"RSD 缺少必需字段 class_name：{rsdFile}");
        if (string.IsNullOrWhiteSpace(fileSuffix))
            throw new InvalidOperationException($"RSD 缺少必需字段 file_suffix：{rsdFile}");

        return new RsdSchema
        {
            ClassName = className.Trim(),
            FileSuffix = fileSuffix.Trim(),
            Fields = fields
        };
    }
}


