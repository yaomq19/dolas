using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text.Json.Nodes;
using Dolas.AssetEditor.ViewModels.Json;

namespace Dolas.AssetEditor.Services;

public static class RsdAssetBinder
{
    public static (JsonObject root, JsonNodeViewModel viewModel, bool mutated, List<string> warnings) BindToSchema(
        JsonNode rootNode,
        RsdSchema schema,
        Action markDirty)
    {
        if (rootNode is not JsonObject obj)
            throw new InvalidOperationException("资产 JSON 根必须是 object");

        var warnings = new List<string>();
        var mutated = false;

        // 确保 schema 定义的字段都存在，并尽量符合类型
        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            if (!obj.TryGetPropertyValue(fieldName, out var val) || val is null)
            {
                obj[fieldName] = DefaultNodeForType(typeSpec);
                warnings.Add($"字段缺失：{fieldName}，已填充默认值（未保存）");
                mutated = true;
                continue;
            }

            if (!IsCompatible(val, typeSpec))
            {
                var old = val.ToJsonString();
                obj[fieldName] = DefaultNodeForType(typeSpec);
                warnings.Add($"字段类型不匹配：{fieldName}，期望 {typeSpec}，实际 {old}，已用默认值替换（未保存）");
                mutated = true;
            }
        }

        if (mutated)
            markDirty();

        // 用 schema 定义的顺序构造展示树：只展示 schema 字段（更像“按约定读取”）
        var vmRoot = new JsonNodeViewModel(schema.ClassName, obj, markDirty);
        vmRoot.Children.Clear();
        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            obj.TryGetPropertyValue(fieldName, out var val);
            vmRoot.Children.Add(JsonNodeViewModel.CreateSchemaField(fieldName, val, markDirty, obj, fieldName, typeSpec));
        }

        return (obj, vmRoot, mutated, warnings);
    }

    private static bool IsCompatible(JsonNode node, string typeSpec)
    {
        if (typeSpec.Trim() == "Json")
            return true;
        if (typeSpec.Trim() == "Xml")
            return node is JsonValue v && v.TryGetValue<string>(out _);

        var t = TypeSpecToEditorType(typeSpec);
        return t switch
        {
            JsonEditorType.String => node is JsonValue v && v.TryGetValue<string>(out _),
            JsonEditorType.Bool => node is JsonValue v && v.TryGetValue<bool>(out _),
            JsonEditorType.Int => IsNumber(node, allowFloat: false, allowUnsigned: false),
            JsonEditorType.UInt => IsNumber(node, allowFloat: false, allowUnsigned: true),
            JsonEditorType.Float => IsNumber(node, allowFloat: true, allowUnsigned: true),
            JsonEditorType.Vector3 => IsNumberArray(node, 3),
            JsonEditorType.Vector4 => IsNumberArray(node, 4),
            JsonEditorType.Array => node is JsonArray,
            JsonEditorType.Object => node is JsonObject,
            _ => true
        };
    }

    private static bool IsNumber(JsonNode node, bool allowFloat, bool allowUnsigned)
    {
        if (node is not JsonValue v) return false;
        if (!v.TryGetValue<double>(out var d)) return false;
        if (!allowFloat && Math.Abs(d % 1) > double.Epsilon) return false;
        if (!allowUnsigned && d < 0) return false;
        return true;
    }

    private static bool IsNumberArray(JsonNode node, int len)
    {
        if (node is not JsonArray arr) return false;
        if (arr.Count != len) return false;
        return arr.All(n => n is JsonValue v && v.TryGetValue<double>(out _));
    }

    private static JsonNode DefaultNodeForType(string typeSpec)
    {
        if (typeSpec.Trim() == "Json")
            return new JsonObject();
        if (typeSpec.Trim() == "Xml")
            return JsonValue.Create(string.Empty)!;

        var t = TypeSpecToEditorType(typeSpec);
        return t switch
        {
            JsonEditorType.String => JsonValue.Create(string.Empty)!,
            JsonEditorType.Bool => JsonValue.Create(false)!,
            JsonEditorType.Int => JsonValue.Create(0)!,
            JsonEditorType.UInt => JsonValue.Create(0)!,
            JsonEditorType.Float => JsonValue.Create(0.0)!,
            JsonEditorType.Vector3 => new JsonArray(0.0, 0.0, 0.0),
            JsonEditorType.Vector4 => new JsonArray(0.0, 0.0, 0.0, 0.0),
            JsonEditorType.Array => new JsonArray(),
            JsonEditorType.Object => new JsonObject(),
            _ => JsonValue.Create((string?)null)!
        };
    }

    private static JsonEditorType TypeSpecToEditorType(string typeSpec)
    {
        var s = typeSpec.Trim();

        // 泛型容器：按 JSON 形态给一个大类（后续可以做更细粒度）
        if (s.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase)) return JsonEditorType.Array;
        if (s.StartsWith("StaticArray", StringComparison.OrdinalIgnoreCase)) return JsonEditorType.Array;
        if (s.StartsWith("Set", StringComparison.OrdinalIgnoreCase)) return JsonEditorType.Array;
        if (s.StartsWith("Map", StringComparison.OrdinalIgnoreCase)) return JsonEditorType.Object;
        if (s.Equals("Json", StringComparison.OrdinalIgnoreCase)) return JsonEditorType.Object;
        if (s.Equals("Xml", StringComparison.OrdinalIgnoreCase)) return JsonEditorType.String;

        return s switch
        {
            "String" => JsonEditorType.String,
            "Bool" => JsonEditorType.Bool,
            "Int" => JsonEditorType.Int,
            "UInt" => JsonEditorType.UInt,
            "Float" => JsonEditorType.Float,
            "Vector3" => JsonEditorType.Vector3,
            "Vector4" => JsonEditorType.Vector4,
            _ => JsonEditorType.String
        };
    }
}


