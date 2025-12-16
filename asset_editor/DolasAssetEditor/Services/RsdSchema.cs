using System.Collections.Generic;

namespace Dolas.AssetEditor.Services;

public sealed class RsdSchema
{
    public required string ClassName { get; init; }
    public required string FileSuffix { get; init; }

    /// <summary>
    /// 字段名 -> 类型规格字符串（例如 "Float", "Vector3", "DynamicArray<Int>", ...）
    /// </summary>
    public required IReadOnlyDictionary<string, string> Fields { get; init; }
}


