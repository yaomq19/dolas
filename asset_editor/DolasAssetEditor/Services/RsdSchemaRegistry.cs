using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Xml.Linq;

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
        var doc = XDocument.Load(rsdFile, LoadOptions.PreserveWhitespace);
        var root = doc.Root ?? throw new InvalidOperationException($"RSD XML 没有根节点：{rsdFile}");
        if (!string.Equals(root.Name.LocalName, "rsd", StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException($"RSD 根节点必须是 <rsd>：{rsdFile}");

        var className = root.Attribute("class_name")?.Value?.Trim();
        var fileSuffix = root.Attribute("file_suffix")?.Value?.Trim();
        if (string.IsNullOrWhiteSpace(className))
            throw new InvalidOperationException($"RSD 缺少必需属性 class_name：{rsdFile}");
        if (string.IsNullOrWhiteSpace(fileSuffix))
            throw new InvalidOperationException($"RSD 缺少必需属性 file_suffix：{rsdFile}");

        var fields = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var f in root.Elements("field"))
        {
            var name = f.Attribute("name")?.Value?.Trim();
            var type = f.Attribute("type")?.Value?.Trim();
            if (string.IsNullOrWhiteSpace(name) || string.IsNullOrWhiteSpace(type))
                throw new InvalidOperationException($"RSD <field> 缺少 name/type：{rsdFile}");
            fields[name] = type;
        }

        return new RsdSchema
        {
            ClassName = className.Trim(),
            FileSuffix = fileSuffix.Trim(),
            Fields = fields
        };
    }
}


