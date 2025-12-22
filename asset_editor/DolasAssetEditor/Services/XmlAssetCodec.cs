using System;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text.Json.Nodes;
using System.Xml.Linq;

namespace Dolas.AssetEditor.Services;

public static class XmlAssetCodec
{
    private static string ExtractDynamicArrayElementSpec(string t)
    {
        // DynamicArray<T>
        var lt = t.IndexOf('<');
        var gt = t.LastIndexOf('>');
        if (lt < 0 || gt <= lt) return string.Empty;
        return t.Substring(lt + 1, gt - lt - 1).Trim();
    }

    private static bool IsAssetRefSpec(string elemSpec)
        => elemSpec.StartsWith("AssetReference<", StringComparison.OrdinalIgnoreCase);

    private static double ReadAttrDoubleOr0(XElement el, string attr)
    {
        var a = el.Attribute(attr)?.Value;
        if (a is null) return 0;
        return double.TryParse(a, NumberStyles.Float, CultureInfo.InvariantCulture, out var d) ? d : 0;
    }

    public static JsonObject LoadAssetAsJsonObject(string filePath, RsdSchema schema)
    {
        var doc = XDocument.Load(filePath, LoadOptions.PreserveWhitespace);
        var root = doc.Root ?? throw new InvalidOperationException("XML 没有根节点");

        var obj = new JsonObject();

        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            var t = typeSpec.Trim();

            // 容器：DynamicArray<Xml> / DynamicArray<T>
            if (t.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase))
            {
                // 约定：xml 里用 <fieldName> 容器包起来
                var container = root.Element(fieldName);
                if (container is null) continue;

                var elemSpec = ExtractDynamicArrayElementSpec(t);
                var arr = new JsonArray();

                // Array of strings: DynamicArray<String> / DynamicArray<AssetReference<...>> / DynamicArray<RawReference>
                if (elemSpec.Equals("String", StringComparison.OrdinalIgnoreCase) ||
                    elemSpec.Equals("RawReference", StringComparison.OrdinalIgnoreCase) ||
                    IsAssetRefSpec(elemSpec))
                {
                    foreach (var child in container.Elements())
                    {
                        // support <item>text</item> or <item file="..."/>
                        var s = (string?)child.Attribute("file") ?? child.Value;
                        arr.Add(s ?? string.Empty);
                    }
                    obj[fieldName] = arr;
                    continue;
                }

                // Array of vectors: DynamicArray<Vector3/Vector4> as <item x="" y="" z="" (w="") />
                if (elemSpec.Equals("Vector3", StringComparison.OrdinalIgnoreCase))
                {
                    foreach (var child in container.Elements())
                    {
                        arr.Add(new JsonArray(
                            ReadAttrDoubleOr0(child, "x"),
                            ReadAttrDoubleOr0(child, "y"),
                            ReadAttrDoubleOr0(child, "z")));
                    }
                    obj[fieldName] = arr;
                    continue;
                }
                if (elemSpec.Equals("Vector4", StringComparison.OrdinalIgnoreCase))
                {
                    foreach (var child in container.Elements())
                    {
                        arr.Add(new JsonArray(
                            ReadAttrDoubleOr0(child, "x"),
                            ReadAttrDoubleOr0(child, "y"),
                            ReadAttrDoubleOr0(child, "z"),
                            ReadAttrDoubleOr0(child, "w")));
                    }
                    obj[fieldName] = arr;
                    continue;
                }

                // Fallback: keep each child element as raw XML fragment string
                foreach (var child in container.Elements())
                {
                    arr.Add(child.ToString(SaveOptions.None));
                }
                obj[fieldName] = arr;
                continue;
            }

            var el = root.Element(fieldName);
            if (el is null) continue;

            // MaterialRSD 的 map 约定（XML 形态固定）
            if (t.StartsWith("Map<", StringComparison.OrdinalIgnoreCase) || t.StartsWith("Map<", StringComparison.Ordinal))
            {
                // Map<String, Vector4>
                if (t.Contains("Vector4", StringComparison.OrdinalIgnoreCase))
                {
                    var map = new JsonObject();
                    foreach (var v in el.Elements("vec4"))
                    {
                        var name = v.Attribute("name")?.Value;
                        if (string.IsNullOrWhiteSpace(name)) continue;
                        map[name] = new JsonArray(
                            ReadAttrDouble(v, "x"),
                            ReadAttrDouble(v, "y"),
                            ReadAttrDouble(v, "z"),
                            ReadAttrDouble(v, "w"));
                    }
                    obj[fieldName] = map;
                    continue;
                }

                // Map<String, String>（textures）
                if (t.Contains("String", StringComparison.OrdinalIgnoreCase) && t.LastIndexOf("String", StringComparison.OrdinalIgnoreCase) != t.IndexOf("String", StringComparison.OrdinalIgnoreCase))
                {
                    var map = new JsonObject();
                    foreach (var tex in el.Elements("texture"))
                    {
                        var name = tex.Attribute("name")?.Value;
                        var file = tex.Attribute("file")?.Value;
                        if (string.IsNullOrWhiteSpace(name) || file is null) continue;
                        map[name] = file;
                    }
                    obj[fieldName] = map;
                    continue;
                }

                // Map<String, Float>（parameter）
                if (t.Contains("Float", StringComparison.OrdinalIgnoreCase))
                {
                    var map = new JsonObject();
                    foreach (var f in el.Elements("float"))
                    {
                        var name = f.Attribute("name")?.Value;
                        var value = f.Attribute("value")?.Value;
                        if (string.IsNullOrWhiteSpace(name) || value is null) continue;
                        if (double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out var d))
                            map[name] = d;
                    }
                    obj[fieldName] = map;
                    continue;
                }
            }

            if (t == "String" || t.Equals("RawReference", StringComparison.OrdinalIgnoreCase) || t.StartsWith("AssetReference<", StringComparison.OrdinalIgnoreCase))
            {
                obj[fieldName] = el.Value;
            }
            else if (t.StartsWith("Enum<", StringComparison.OrdinalIgnoreCase))
            {
                // Enum value is stored as plain text in XML (e.g. "Perspective" or legacy alias like "perspective").
                obj[fieldName] = el.Value;
            }
            else if (t == "Bool")
            {
                if (bool.TryParse(el.Value, out var b))
                    obj[fieldName] = b;
            }
            else if (t == "Int")
            {
                if (long.TryParse(el.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var i))
                    obj[fieldName] = i;
            }
            else if (t == "UInt")
            {
                if (ulong.TryParse(el.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var u))
                    obj[fieldName] = u;
            }
            else if (t == "Float")
            {
                if (double.TryParse(el.Value, NumberStyles.Float, CultureInfo.InvariantCulture, out var f))
                    obj[fieldName] = f;
            }
            else if (t == "Vector3")
            {
                obj[fieldName] = new JsonArray(
                    ReadAttrDouble(el, "x"),
                    ReadAttrDouble(el, "y"),
                    ReadAttrDouble(el, "z"));
            }
            else if (t == "Vector4")
            {
                obj[fieldName] = new JsonArray(
                    ReadAttrDouble(el, "x"),
                    ReadAttrDouble(el, "y"),
                    ReadAttrDouble(el, "z"),
                    ReadAttrDouble(el, "w"));
            }
            else if (t.Equals("RawReference", StringComparison.OrdinalIgnoreCase) || t == "Xml")
            {
                // 用格式化输出，便于在编辑器里阅读/编辑
                obj[fieldName] = el.ToString(SaveOptions.None);
            }
            else
            {
                // 兜底：保留原始 xml 片段，避免丢数据
                obj[fieldName] = el.ToString(SaveOptions.None);
            }
        }

        return obj;
    }

    public static void SaveAssetFromJsonObject(string filePath, RsdSchema schema, JsonObject data)
    {
        var root = new XElement("asset");

        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            if (!data.TryGetPropertyValue(fieldName, out var node) || node is null)
                continue;

            var t = typeSpec.Trim();

            // Map 类型写回（按 MaterialRSD 的 XML 约定）
            if (t.StartsWith("Map<", StringComparison.OrdinalIgnoreCase) && node is JsonObject mapObj)
            {
                var container = new XElement(fieldName);

                if (t.Contains("Vector4", StringComparison.OrdinalIgnoreCase))
                {
                    foreach (var kv in mapObj)
                    {
                        if (kv.Value is not JsonArray arr || arr.Count < 4) continue;
                        var v = new XElement("vec4");
                        v.SetAttributeValue("name", kv.Key);
                        v.SetAttributeValue("x", arr[0]?.ToString() ?? "0");
                        v.SetAttributeValue("y", arr[1]?.ToString() ?? "0");
                        v.SetAttributeValue("z", arr[2]?.ToString() ?? "0");
                        v.SetAttributeValue("w", arr[3]?.ToString() ?? "0");
                        container.Add(v);
                    }
                    root.Add(container);
                    continue;
                }

                if (t.Contains("Float", StringComparison.OrdinalIgnoreCase))
                {
                    foreach (var kv in mapObj)
                    {
                        var f = new XElement("float");
                        f.SetAttributeValue("name", kv.Key);
                        f.SetAttributeValue("value", kv.Value?.ToString() ?? "0");
                        container.Add(f);
                    }
                    root.Add(container);
                    continue;
                }

                // Map<String,String>
                foreach (var kv in mapObj)
                {
                    var tex = new XElement("texture");
                    tex.SetAttributeValue("name", kv.Key);
                    tex.SetAttributeValue("file", kv.Value?.ToString() ?? "");
                    container.Add(tex);
                }
                root.Add(container);
                continue;
            }

            if (t.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase))
            {
                var container = new XElement(fieldName);
                if (node is JsonArray arr)
                {
                    var elemSpec = ExtractDynamicArrayElementSpec(t);

                    // String-like arrays: emit <item>text</item>
                    if (elemSpec.Equals("String", StringComparison.OrdinalIgnoreCase) ||
                        elemSpec.Equals("RawReference", StringComparison.OrdinalIgnoreCase) ||
                        IsAssetRefSpec(elemSpec))
                    {
                        foreach (var item in arr)
                        {
                            var itemText = item?.ToString() ?? string.Empty;
                            container.Add(new XElement("item", itemText));
                        }
                        root.Add(container);
                        continue;
                    }

                    // Vector arrays: emit <item x="" y="" z="" (w="") />
                    if (elemSpec.Equals("Vector3", StringComparison.OrdinalIgnoreCase) ||
                        elemSpec.Equals("Vector4", StringComparison.OrdinalIgnoreCase))
                    {
                        foreach (var item in arr)
                        {
                            if (item is not JsonArray vArr) continue;
                            var el = new XElement("item");
                            if (vArr.Count > 0) el.SetAttributeValue("x", vArr[0]?.ToString() ?? "0");
                            if (vArr.Count > 1) el.SetAttributeValue("y", vArr[1]?.ToString() ?? "0");
                            if (vArr.Count > 2) el.SetAttributeValue("z", vArr[2]?.ToString() ?? "0");
                            if (elemSpec.Equals("Vector4", StringComparison.OrdinalIgnoreCase) && vArr.Count > 3)
                                el.SetAttributeValue("w", vArr[3]?.ToString() ?? "0");
                            container.Add(el);
                        }
                        root.Add(container);
                        continue;
                    }

                    // Fallback: try parse as XML fragment, else save as <item>text</item>
                    foreach (var item in arr)
                    {
                        if (item is JsonValue v && v.TryGetValue<string>(out var xmlFragment))
                        {
                            try { container.Add(XElement.Parse(xmlFragment, LoadOptions.PreserveWhitespace)); }
                            catch { container.Add(new XElement("item", xmlFragment)); }
                        }
                    }
                }
                root.Add(container);
                continue;
            }

            if (t.StartsWith("Enum<", StringComparison.OrdinalIgnoreCase))
            {
                // Enum is stored as text. We keep whatever string is in JSON.
                if (node is JsonValue enumValue && enumValue.TryGetValue<string>(out var sEnum))
                    root.Add(new XElement(fieldName, sEnum));
                else
                    root.Add(new XElement(fieldName, node.ToJsonString()));
                continue;
            }

            if (t == "Vector3" || t == "Vector4")
            {
                if (node is not JsonArray arr) continue;
                var el = new XElement(fieldName);
                if (arr.Count > 0) el.SetAttributeValue("x", arr[0]?.ToString() ?? "0");
                if (arr.Count > 1) el.SetAttributeValue("y", arr[1]?.ToString() ?? "0");
                if (arr.Count > 2) el.SetAttributeValue("z", arr[2]?.ToString() ?? "0");
                if (t == "Vector4" && arr.Count > 3) el.SetAttributeValue("w", arr[3]?.ToString() ?? "0");
                root.Add(el);
                continue;
            }

            if (node is JsonValue sv && sv.TryGetValue<string>(out var s))
            {
                // 如果是 Xml 片段，尽量原样写回
                if (t == "Xml")
                {
                    try { root.Add(XElement.Parse(s, LoadOptions.PreserveWhitespace)); }
                    catch { root.Add(new XElement(fieldName, s)); }
                }
                else
                {
                    root.Add(new XElement(fieldName, s));
                }
                continue;
            }

            // 其它类型：用 JsonNode 的 ToJsonString 作为文本兜底
            root.Add(new XElement(fieldName, node.ToJsonString()));
        }

        var doc = new XDocument(new XDeclaration("1.0", "utf-8", "yes"), root);
        File.WriteAllText(filePath, doc.ToString(SaveOptions.None));
    }

    private static double ReadAttrDouble(XElement el, string attr)
    {
        var a = el.Attribute(attr)?.Value;
        if (a is null) return 0;
        return double.TryParse(a, NumberStyles.Float, CultureInfo.InvariantCulture, out var d) ? d : 0;
    }
}


