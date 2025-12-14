using System;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace Dolas.AssetEditor.ViewModels.Json;

public sealed class JsonNodeViewModel : ViewModelBase
{
    private JsonNode? _node;
    private readonly JsonObject? _parentObject;
    private readonly string? _objectKey;
    private readonly JsonArray? _parentArray;
    private readonly int? _arrayIndex;
    private readonly Action? _markDirty;

    private string _scalarText = string.Empty;
    private bool _boolValue;
    private string _xText = "0";
    private string _yText = "0";
    private string _zText = "0";
    private string _wText = "0";

    public JsonNodeViewModel(
        string name,
        JsonNode? node,
        Action? markDirty,
        JsonObject? parentObject = null,
        string? objectKey = null,
        JsonArray? parentArray = null,
        int? arrayIndex = null)
    {
        Name = name;
        _node = node;
        _markDirty = markDirty;
        _parentObject = parentObject;
        _objectKey = objectKey;
        _parentArray = parentArray;
        _arrayIndex = arrayIndex;
        Children = new ObservableCollection<JsonNodeViewModel>();

        (EditorType, _scalarText, _boolValue, _xText, _yText, _zText, _wText) = InferAndInit(node);

        if (EditorType is JsonEditorType.Object && node is JsonObject obj)
        {
            foreach (var kv in obj.OrderBy(k => k.Key))
                Children.Add(new JsonNodeViewModel(kv.Key, kv.Value, markDirty, parentObject: obj, objectKey: kv.Key));
        }
        else if (EditorType is JsonEditorType.Array && node is JsonArray arr)
        {
            for (var i = 0; i < arr.Count; i++)
                Children.Add(new JsonNodeViewModel($"[{i}]", arr[i], markDirty, parentArray: arr, arrayIndex: i));
        }
    }

    public string Name { get; }

    public JsonEditorType EditorType { get; }

    public ObservableCollection<JsonNodeViewModel> Children { get; }

    public string ScalarText
    {
        get => _scalarText;
        set
        {
            if (!SetField(ref _scalarText, value)) return;
            if (TryApplyScalar(value)) _markDirty?.Invoke();
        }
    }

    public bool BoolValue
    {
        get => _boolValue;
        set
        {
            if (!SetField(ref _boolValue, value)) return;
            if (TryReplaceInParent(JsonValue.Create(value))) _markDirty?.Invoke();
        }
    }

    public string XText
    {
        get => _xText;
        set
        {
            if (!SetField(ref _xText, value)) return;
            if (TryApplyVector()) _markDirty?.Invoke();
        }
    }

    public string YText
    {
        get => _yText;
        set
        {
            if (!SetField(ref _yText, value)) return;
            if (TryApplyVector()) _markDirty?.Invoke();
        }
    }

    public string ZText
    {
        get => _zText;
        set
        {
            if (!SetField(ref _zText, value)) return;
            if (TryApplyVector()) _markDirty?.Invoke();
        }
    }

    public string WText
    {
        get => _wText;
        set
        {
            if (!SetField(ref _wText, value)) return;
            if (TryApplyVector()) _markDirty?.Invoke();
        }
    }

    public string Summary
    {
        get
        {
            return EditorType switch
            {
                JsonEditorType.Object => $"Object ({Children.Count})",
                JsonEditorType.Array => $"Array ({Children.Count})",
                JsonEditorType.Vector3 => "Vector3",
                JsonEditorType.Vector4 => "Vector4",
                JsonEditorType.String => "String",
                JsonEditorType.Bool => "Bool",
                JsonEditorType.Int => "Int",
                JsonEditorType.Float => "Float",
                JsonEditorType.Null => "Null",
                _ => EditorType.ToString()
            };
        }
    }

    private (JsonEditorType type, string scalarText, bool boolValue, string x, string y, string z, string w) InferAndInit(JsonNode? node)
    {
        if (node is null)
            return (JsonEditorType.Null, "null", false, "0", "0", "0", "0");

        if (node is JsonObject)
            return (JsonEditorType.Object, string.Empty, false, "0", "0", "0", "0");

        if (node is JsonArray arr)
        {
            // Vector3/Vector4：数组长度为 3/4 且全是数字
            if (arr.Count is 3 or 4 && arr.All(IsNumberValue))
            {
                var comps = arr.Select(ReadNumberAsString).ToArray();
                return arr.Count == 3
                    ? (JsonEditorType.Vector3, string.Empty, false, comps[0], comps[1], comps[2], "0")
                    : (JsonEditorType.Vector4, string.Empty, false, comps[0], comps[1], comps[2], comps[3]);
            }

            return (JsonEditorType.Array, string.Empty, false, "0", "0", "0", "0");
        }

        if (node is JsonValue v)
        {
            if (v.TryGetValue<bool>(out var b))
                return (JsonEditorType.Bool, string.Empty, b, "0", "0", "0", "0");

            // 注意：TryGetValue<int> 对大整数可能失败；我们用 JsonElement 来判断 number/string
            if (TryGetJsonElement(v, out var el))
            {
                if (el.ValueKind == JsonValueKind.String)
                    return (JsonEditorType.String, el.GetString() ?? string.Empty, false, "0", "0", "0", "0");

                if (el.ValueKind == JsonValueKind.Number)
                {
                    var raw = el.GetRawText();
                    var isFloat = raw.Contains('.') || raw.Contains('e') || raw.Contains('E');
                    return isFloat
                        ? (JsonEditorType.Float, raw, false, "0", "0", "0", "0")
                        : (JsonEditorType.Int, raw, false, "0", "0", "0", "0");
                }
            }

            // fallback：当作字符串
            return (JsonEditorType.String, v.ToJsonString(), false, "0", "0", "0", "0");
        }

        return (JsonEditorType.String, node.ToJsonString(), false, "0", "0", "0", "0");
    }

    private bool TryApplyScalar(string text)
    {
        if (_node is not JsonValue)
            return false;

        switch (EditorType)
        {
            case JsonEditorType.String:
                return TryReplaceInParent(JsonValue.Create(text));
            case JsonEditorType.Int:
                if (long.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out var l))
                    return TryReplaceInParent(JsonValue.Create(l));
                return false;
            case JsonEditorType.Float:
                if (double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out var d))
                    return TryReplaceInParent(JsonValue.Create(d));
                return false;
            default:
                return false;
        }
    }

    private bool TryApplyVector()
    {
        if (_node is not JsonArray arr)
            return false;

        if (EditorType is not (JsonEditorType.Vector3 or JsonEditorType.Vector4))
            return false;

        if (!double.TryParse(XText, NumberStyles.Float, CultureInfo.InvariantCulture, out var x)) return false;
        if (!double.TryParse(YText, NumberStyles.Float, CultureInfo.InvariantCulture, out var y)) return false;
        if (!double.TryParse(ZText, NumberStyles.Float, CultureInfo.InvariantCulture, out var z)) return false;
        var w = 0d;
        if (EditorType == JsonEditorType.Vector4)
        {
            if (!double.TryParse(WText, NumberStyles.Float, CultureInfo.InvariantCulture, out w)) return false;
        }

        // 更新数组内容
        if (arr.Count >= 1) arr[0] = JsonValue.Create(x);
        if (arr.Count >= 2) arr[1] = JsonValue.Create(y);
        if (arr.Count >= 3) arr[2] = JsonValue.Create(z);
        if (EditorType == JsonEditorType.Vector4 && arr.Count >= 4) arr[3] = JsonValue.Create(w);
        return true;
    }

    private bool TryReplaceInParent(JsonNode? newValue)
    {
        if (_parentObject is not null && _objectKey is not null)
        {
            _parentObject[_objectKey] = newValue;
            _node = newValue;
            return true;
        }

        if (_parentArray is not null && _arrayIndex is not null)
        {
            _parentArray[_arrayIndex.Value] = newValue;
            _node = newValue;
            return true;
        }

        // 根节点是标量的情况：目前不支持就地替换（可按需扩展）
        return false;
    }

    // ------- helpers -------
    private static bool TryGetJsonElement(JsonValue value, out JsonElement el)
    {
        // JsonValue 可以 TryGetValue<JsonElement>
        if (value.TryGetValue<JsonElement>(out el))
            return true;

        el = default;
        return false;
    }

    private static bool IsNumberValue(JsonNode? n)
    {
        if (n is not JsonValue v) return false;
        if (!TryGetJsonElement(v, out var el)) return false;
        return el.ValueKind == JsonValueKind.Number;
    }

    private static string ReadNumberAsString(JsonNode? n)
    {
        if (n is JsonValue v && TryGetJsonElement(v, out var el) && el.ValueKind == JsonValueKind.Number)
            return el.GetRawText();
        return "0";
    }
}


