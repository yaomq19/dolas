using System;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text.Json.Nodes;
using System.Xml.Linq;

namespace Dolas.AssetEditor.Services;

public static class XmlAssetCodec
{
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

                var arr = new JsonArray();
                foreach (var child in container.Elements())
                {
                    arr.Add(child.ToString(SaveOptions.DisableFormatting));
                }
                obj[fieldName] = arr;
                continue;
            }

            var el = root.Element(fieldName);
            if (el is null) continue;

            if (t == "String")
            {
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
            else if (t == "Xml")
            {
                obj[fieldName] = el.ToString(SaveOptions.DisableFormatting);
            }
            else
            {
                // 兜底：保留原始 xml 片段，避免丢数据
                obj[fieldName] = el.ToString(SaveOptions.DisableFormatting);
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

            if (t.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase))
            {
                var container = new XElement(fieldName);
                if (node is JsonArray arr)
                {
                    foreach (var item in arr)
                    {
                        if (item is JsonValue v && v.TryGetValue<string>(out var xmlFragment))
                        {
                            try
                            {
                                container.Add(XElement.Parse(xmlFragment, LoadOptions.PreserveWhitespace));
                            }
                            catch
                            {
                                // 如果不是合法 xml，就当文本保存
                                container.Add(new XElement("item", xmlFragment));
                            }
                        }
                    }
                }
                root.Add(container);
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


