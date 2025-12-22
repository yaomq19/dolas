using System.Collections.Generic;

namespace Dolas.AssetEditor.Services;

public sealed class RsdEnumValue
{
    public required string Name { get; init; }
    public string Display { get; init; } = string.Empty;
    public string Alias { get; init; } = string.Empty;
}

public sealed class RsdEnumDef
{
    public required string Name { get; init; }
    public required IReadOnlyList<RsdEnumValue> Values { get; init; }
}

public sealed class RsdSchema
{
    public required string ClassName { get; init; }
    public string? FileSuffix { get; init; }

    /// <summary>
    /// 字段名 -> 类型规格字符串（例如 "Float", "Vector3", "DynamicArray<Int>", "Enum&lt;MyEnum&gt;", ...）
    /// </summary>
    public required IReadOnlyDictionary<string, string> Fields { get; init; }

    /// <summary>
    /// 枚举名 -> 枚举定义（只包含在同一 .rsd 文件中声明的枚举；用于编辑器下拉框显示）。
    /// </summary>
    public required IReadOnlyDictionary<string, RsdEnumDef> Enums { get; init; }

    public bool IsAssetSchema => !string.IsNullOrWhiteSpace(FileSuffix);
}


