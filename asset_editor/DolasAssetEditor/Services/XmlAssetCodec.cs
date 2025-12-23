using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using Dolas.AssetEditor.ViewModels;

namespace Dolas.AssetEditor.Services;

/// <summary>
/// XML &lt;-&gt; ViewModel 的资产编解码（不使用任何 JSON 相关库/中间表示）。
/// </summary>
public static class XmlAssetCodec
{
    private static string ExtractGenericArg(string spec, string genericName)
    {
        // e.g. DynamicArray<T>, Enum<T>
        var s = spec.Trim();
        if (!s.StartsWith(genericName, StringComparison.OrdinalIgnoreCase))
            return string.Empty;
        var lt = s.IndexOf('<');
        var gt = s.LastIndexOf('>');
        if (lt < 0 || gt <= lt) return string.Empty;
        return s.Substring(lt + 1, gt - lt - 1).Trim();
    }

    private static bool IsAssetRefSpec(string spec)
        => spec.Trim().StartsWith("AssetReference<", StringComparison.OrdinalIgnoreCase);

    private static IReadOnlyList<RsdEnumValue>? TryGetEnumValues(RsdSchema schema, string typeSpec)
    {
        var enumName = ExtractGenericArg(typeSpec, "Enum");
        if (string.IsNullOrWhiteSpace(enumName)) return null;
        return schema.Enums.TryGetValue(enumName, out var def) ? def.Values : null;
    }

    private static string ReadScalarTextCompat(XElement el)
    {
        var url = el.Element("url")?.Value;
        if (!string.IsNullOrWhiteSpace(url)) return url.Trim();
        var v = el.Element("value")?.Value;
        if (!string.IsNullOrWhiteSpace(v)) return v.Trim();
        return (el.Value ?? string.Empty).Trim();
    }

    private static double ReadVectorComp(XElement el, string name)
    {
        var a = (string?)el.Attribute(name);
        if (!string.IsNullOrWhiteSpace(a) && double.TryParse(a, NumberStyles.Float, CultureInfo.InvariantCulture, out var av))
            return av;
        var c = el.Element(name)?.Value;
        if (!string.IsNullOrWhiteSpace(c) && double.TryParse(c, NumberStyles.Float, CultureInfo.InvariantCulture, out var cv))
            return cv;
        return 0.0;
    }

    private static XElement WriteVector3New(string fieldName, AssetNodeViewModel vm)
        => new XElement(fieldName,
            new XAttribute("type", "Vector3"),
            new XElement("x", vm.XText ?? "0"),
            new XElement("y", vm.YText ?? "0"),
            new XElement("z", vm.ZText ?? "0"));

    private static XElement WriteVector4New(string fieldName, AssetNodeViewModel vm)
        => new XElement(fieldName,
            new XAttribute("type", "Vector4"),
            new XElement("x", vm.XText ?? "0"),
            new XElement("y", vm.YText ?? "0"),
            new XElement("z", vm.ZText ?? "0"),
            new XElement("w", vm.WText ?? "0"));

    private static EditorType TypeSpecToEditorType(string typeSpec, RsdSchemaRegistry? registry)
    {
        var s = typeSpec.Trim();
        if (s.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase)) return EditorType.Array;
        if (s.StartsWith("StaticArray", StringComparison.OrdinalIgnoreCase)) return EditorType.Array;
        if (s.StartsWith("Set", StringComparison.OrdinalIgnoreCase)) return EditorType.Array;
        if (s.StartsWith("Map", StringComparison.OrdinalIgnoreCase)) return EditorType.Object;
        if (s.StartsWith("Enum<", StringComparison.OrdinalIgnoreCase)) return EditorType.Enum;
        if (IsAssetRefSpec(s)) return EditorType.String;
        if (s.Equals("RawReference", StringComparison.OrdinalIgnoreCase)) return EditorType.String;
        if (registry?.TryGetByClassName(s, out var objSchema) == true && objSchema is not null)
            return EditorType.Object;

        return s switch
        {
            "String" => EditorType.String,
            "Bool" => EditorType.Bool,
            "Int" => EditorType.Int,
            "UInt" => EditorType.UInt,
            "Float" => EditorType.Float,
            "Vector3" => EditorType.Vector3,
            "Vector4" => EditorType.Vector4,
            _ => EditorType.String
        };
    }

    public static AssetNodeViewModel CreateDefaultAssetViewModel(RsdSchema schema, RsdSchemaRegistry? registry, Action? markDirty)
    {
        var root = new AssetNodeViewModel(schema.ClassName, EditorType.Object, markDirty);
        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            root.Children.Add(CreateDefaultField(schema, registry, fieldName, typeSpec, markDirty));
        }
        return root;
    }

    public static AssetNodeViewModel LoadAssetAsViewModel(
        string filePath,
        RsdSchema schema,
        RsdSchemaRegistry? registry,
        Action? markDirty,
        out bool mutated,
        out List<string> warnings)
    {
        var doc = XDocument.Load(filePath, LoadOptions.PreserveWhitespace);
        var rootEl = doc.Root ?? throw new InvalidOperationException("XML 没有根节点");

        mutated = false;
        warnings = new List<string>();

        var rootVm = new AssetNodeViewModel(schema.ClassName, EditorType.Object, markDirty);

        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            var vm = CreateDefaultField(schema, registry, fieldName, typeSpec, markDirty);
            ApplyFromXml(rootEl, schema, registry, fieldName, typeSpec, vm, markDirty, ref mutated, warnings);
            rootVm.Children.Add(vm);
        }

        return rootVm;
    }

    public static void SaveAssetFromViewModel(string filePath, RsdSchema schema, RsdSchemaRegistry? registry, AssetNodeViewModel rootVm)
    {
        var root = new XElement("asset");

        // 只按 schema.Fields 写回（和 UI 展示一致）
        var byName = rootVm.Children.ToDictionary(c => c.Name, StringComparer.Ordinal);

        foreach (var (fieldName, typeSpec) in schema.Fields)
        {
            if (!byName.TryGetValue(fieldName, out var vm))
                continue;

            WriteField(schema, registry, fieldName, typeSpec, vm, root);
        }

        var doc = new XDocument(new XDeclaration("1.0", "utf-8", "yes"), root);
        File.WriteAllText(filePath, doc.ToString(SaveOptions.None));
    }

    // ----------------- Build VM -----------------

    private static AssetNodeViewModel CreateDefaultField(RsdSchema schema, RsdSchemaRegistry? registry, string fieldName, string typeSpec, Action? markDirty)
    {
        var t = typeSpec.Trim();
        var et = TypeSpecToEditorType(t, registry);
        var vm = new AssetNodeViewModel(fieldName, et, markDirty);

        if (et == EditorType.Enum)
        {
            vm.SetEnumSelected(string.Empty, TryGetEnumValues(schema, t));
            return vm;
        }

        if (et == EditorType.Bool) { vm.SetBool(false); return vm; }
        if (et == EditorType.Int) { vm.SetScalar("0", EditorType.Int); return vm; }
        if (et == EditorType.UInt) { vm.SetScalar("0", EditorType.UInt); return vm; }
        if (et == EditorType.Float) { vm.SetScalar("0", EditorType.Float); return vm; }
        if (et == EditorType.Vector3) { vm.SetVector3(0, 0, 0); return vm; }
        if (et == EditorType.Vector4) { vm.SetVector4(0, 0, 0, 0); return vm; }

        if (et == EditorType.Array)
        {
            // empty by default
            vm.SetAs(EditorType.Array);
            return vm;
        }
        if (et == EditorType.Object)
        {
            vm.SetAs(EditorType.Object);
            if (registry?.TryGetByClassName(t, out var sub) == true && sub is not null)
            {
                vm.Children.Clear();
                foreach (var (fn, ts) in sub.Fields)
                    vm.Children.Add(CreateDefaultField(sub, registry, fn, ts, markDirty));
            }
            return vm;
        }

        // String-like
        vm.SetScalar(string.Empty, EditorType.String);
        return vm;
    }

    // ----------------- XML -> VM -----------------

    private static void ApplyFromXml(
        XElement root,
        RsdSchema schema,
        RsdSchemaRegistry? registry,
        string fieldName,
        string typeSpec,
        AssetNodeViewModel vm,
        Action? markDirty,
        ref bool mutated,
        List<string> warnings)
    {
        var t = typeSpec.Trim();

        if (t.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase))
        {
            vm.SetAs(EditorType.Array);
            vm.Children.Clear();

            var container = root.Element(fieldName);
            if (container is null) return; // keep default empty

            var elemSpec = ExtractGenericArg(t, "DynamicArray");
            var isStringLike = elemSpec.Equals("String", StringComparison.OrdinalIgnoreCase) ||
                               elemSpec.Equals("RawReference", StringComparison.OrdinalIgnoreCase) ||
                               IsAssetRefSpec(elemSpec);

            if (isStringLike)
            {
                var idx = 0;
                foreach (var child in container.Elements())
                {
                    var v = (string?)child.Attribute("file") ?? ReadScalarTextCompat(child);
                    var itemVm = new AssetNodeViewModel($"[{idx++}]", EditorType.String, markDirty);
                    itemVm.SetScalar(v, EditorType.String);
                    vm.Children.Add(itemVm);
                }
                return;
            }

            if (elemSpec.Equals("Vector3", StringComparison.OrdinalIgnoreCase))
            {
                var idx = 0;
                foreach (var child in container.Elements())
                {
                    var itemVm = new AssetNodeViewModel($"[{idx++}]", EditorType.Vector3, markDirty);
                    itemVm.SetVector3(ReadVectorComp(child, "x"), ReadVectorComp(child, "y"), ReadVectorComp(child, "z"));
                    vm.Children.Add(itemVm);
                }
                return;
            }

            if (elemSpec.Equals("Vector4", StringComparison.OrdinalIgnoreCase))
            {
                var idx = 0;
                foreach (var child in container.Elements())
                {
                    var itemVm = new AssetNodeViewModel($"[{idx++}]", EditorType.Vector4, markDirty);
                    itemVm.SetVector4(ReadVectorComp(child, "x"), ReadVectorComp(child, "y"), ReadVectorComp(child, "z"), ReadVectorComp(child, "w"));
                    vm.Children.Add(itemVm);
                }
                return;
            }

            // Array of object type: DynamicArray<OneEntity>
            if (registry?.TryGetByClassName(elemSpec, out var itemSchema) == true && itemSchema is not null)
            {
                var idx = 0;
                foreach (var child in container.Elements())
                {
                    var objVm = new AssetNodeViewModel($"[{idx++}]", EditorType.Object, markDirty);
                    foreach (var (fn, ts) in itemSchema.Fields)
                    {
                        var fvm = CreateDefaultField(itemSchema, registry, fn, ts, markDirty);
                        ApplyFromXml(child, itemSchema, registry, fn, ts, fvm, markDirty, ref mutated, warnings);
                        objVm.Children.Add(fvm);
                    }
                    vm.Children.Add(objVm);
                }
                return;
            }

            // Unknown element type: keep empty but warn once
            mutated = true;
            warnings.Add($"字段 {fieldName} 的数组元素类型不支持：{typeSpec}（已忽略内容）");
            return;
        }

        // Map (only the 3 existing conventions are supported)
        if (t.StartsWith("Map<", StringComparison.OrdinalIgnoreCase))
        {
            vm.SetAs(EditorType.Object);
            vm.Children.Clear();

            var container = root.Element(fieldName);
            if (container is null) return;

            // New style: <item key="..."><x>..</x><y>..</y><z>..</z><w>..</w></item>
            if (t.Contains("Vector4", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var it in container.Elements("item"))
                {
                    var key = (string?)it.Attribute("key") ?? (string?)it.Attribute("name");
                    if (string.IsNullOrWhiteSpace(key)) continue;
                    var itemVm = new AssetNodeViewModel(key.Trim(), EditorType.Vector4, markDirty);
                    itemVm.SetVector4(ReadVectorComp(it, "x"), ReadVectorComp(it, "y"), ReadVectorComp(it, "z"), ReadVectorComp(it, "w"));
                    vm.Children.Add(itemVm);
                }
                foreach (var v in container.Elements("vec4"))
                {
                    var key = v.Attribute("name")?.Value?.Trim();
                    if (string.IsNullOrWhiteSpace(key)) continue;
                    var itemVm = new AssetNodeViewModel(key, EditorType.Vector4, markDirty);
                    itemVm.SetVector4(ReadAttrDoubleOr0(v, "x"), ReadAttrDoubleOr0(v, "y"), ReadAttrDoubleOr0(v, "z"), ReadAttrDoubleOr0(v, "w"));
                    vm.Children.Add(itemVm);
                }
                return;
            }

            // Map<String, Float>: old <float name="..." value="..."/> / new <item key="...">1.0</item>
            if (t.Contains("Float", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var it in container.Elements("item"))
                {
                    var key = (string?)it.Attribute("key") ?? (string?)it.Attribute("name");
                    if (string.IsNullOrWhiteSpace(key)) continue;
                    var val = (string?)it.Attribute("value") ?? ReadScalarTextCompat(it);
                    var itemVm = new AssetNodeViewModel(key.Trim(), EditorType.Float, markDirty);
                    itemVm.SetScalar(string.IsNullOrWhiteSpace(val) ? "0" : val, EditorType.Float);
                    vm.Children.Add(itemVm);
                }
                foreach (var f in container.Elements("float"))
                {
                    var key = f.Attribute("name")?.Value?.Trim();
                    var val = f.Attribute("value")?.Value?.Trim() ?? "0";
                    if (string.IsNullOrWhiteSpace(key)) continue;
                    var itemVm = new AssetNodeViewModel(key, EditorType.Float, markDirty);
                    itemVm.SetScalar(val, EditorType.Float);
                    vm.Children.Add(itemVm);
                }
                return;
            }

            // Map<String, String>: old <texture name="..." file="..."/> / new <item key="..."><url>...</url></item>
            foreach (var it in container.Elements("item"))
            {
                var key = (string?)it.Attribute("key") ?? (string?)it.Attribute("name");
                if (string.IsNullOrWhiteSpace(key)) continue;
                var file = (string?)it.Attribute("file") ?? ReadScalarTextCompat(it);
                var itemVm = new AssetNodeViewModel(key.Trim(), EditorType.String, markDirty);
                itemVm.SetScalar(file, EditorType.String);
                vm.Children.Add(itemVm);
            }
            foreach (var tex in container.Elements("texture"))
            {
                var key = tex.Attribute("name")?.Value?.Trim();
                var file = tex.Attribute("file")?.Value?.Trim() ?? string.Empty;
                if (string.IsNullOrWhiteSpace(key)) continue;
                var itemVm = new AssetNodeViewModel(key, EditorType.String, markDirty);
                itemVm.SetScalar(file, EditorType.String);
                vm.Children.Add(itemVm);
            }
            return;
        }

        var el = root.Element(fieldName);
        if (el is null)
        {
            // keep default but mark as mutated so user can save to materialize missing fields
            mutated = true;
            warnings.Add($"字段缺失：{fieldName}，已使用默认值（未保存）");
            return;
        }

        if (t.StartsWith("Enum<", StringComparison.OrdinalIgnoreCase))
        {
            vm.SetEnumSelected(el.Value?.Trim() ?? string.Empty, TryGetEnumValues(schema, t));
            return;
        }

        if (t == "String" || IsAssetRefSpec(t) || t.Equals("RawReference", StringComparison.OrdinalIgnoreCase))
        {
            vm.SetScalar(ReadScalarTextCompat(el), EditorType.String);
            return;
        }

        if (t == "Bool")
        {
            if (bool.TryParse(ReadScalarTextCompat(el), out var b)) vm.SetBool(b);
            else
            {
                mutated = true;
                warnings.Add($"字段解析失败：{fieldName} 期望 Bool，实际 '{el.Value}'（已使用默认值）");
            }
            return;
        }
        if (t == "Int")
        {
            var raw = ReadScalarTextCompat(el);
            if (long.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var i)) vm.SetScalar(i.ToString(CultureInfo.InvariantCulture), EditorType.Int);
            else { mutated = true; warnings.Add($"字段解析失败：{fieldName} 期望 Int，实际 '{raw}'（已使用默认值）"); }
            return;
        }
        if (t == "UInt")
        {
            var raw = ReadScalarTextCompat(el);
            if (ulong.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var u)) vm.SetScalar(u.ToString(CultureInfo.InvariantCulture), EditorType.UInt);
            else { mutated = true; warnings.Add($"字段解析失败：{fieldName} 期望 UInt，实际 '{raw}'（已使用默认值）"); }
            return;
        }
        if (t == "Float")
        {
            var raw = ReadScalarTextCompat(el);
            if (double.TryParse(raw, NumberStyles.Float, CultureInfo.InvariantCulture, out var f)) vm.SetScalar(f.ToString(CultureInfo.InvariantCulture), EditorType.Float);
            else { mutated = true; warnings.Add($"字段解析失败：{fieldName} 期望 Float，实际 '{raw}'（已使用默认值）"); }
            return;
        }

        if (t == "Vector3")
        {
            vm.SetVector3(ReadVectorComp(el, "x"), ReadVectorComp(el, "y"), ReadVectorComp(el, "z"));
            return;
        }
        if (t == "Vector4")
        {
            vm.SetVector4(ReadVectorComp(el, "x"), ReadVectorComp(el, "y"), ReadVectorComp(el, "z"), ReadVectorComp(el, "w"));
            return;
        }

        // fallback: treat as string
        vm.SetScalar(el.Value ?? string.Empty, EditorType.String);
    }

    // ----------------- VM -> XML -----------------

    private static void WriteField(RsdSchema schema, RsdSchemaRegistry? registry, string fieldName, string typeSpec, AssetNodeViewModel vm, XElement root)
    {
        var t = typeSpec.Trim();

        if (t.StartsWith("DynamicArray", StringComparison.OrdinalIgnoreCase))
        {
            var container = new XElement(fieldName);
            var elemSpec = ExtractGenericArg(t, "DynamicArray");
            container.SetAttributeValue("type", "DynamicArray");
            container.SetAttributeValue("element", elemSpec);
            var isStringLike = elemSpec.Equals("String", StringComparison.OrdinalIgnoreCase) ||
                               elemSpec.Equals("RawReference", StringComparison.OrdinalIgnoreCase) ||
                               IsAssetRefSpec(elemSpec);

            if (isStringLike)
            {
                foreach (var item in vm.Children)
                    container.Add(new XElement("item", new XElement("value", item.ScalarText ?? string.Empty)));
                root.Add(container);
                return;
            }

            if (elemSpec.Equals("Vector3", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    container.Add(new XElement("item",
                        new XAttribute("type", "Vector3"),
                        new XElement("x", item.XText ?? "0"),
                        new XElement("y", item.YText ?? "0"),
                        new XElement("z", item.ZText ?? "0")));
                }
                root.Add(container);
                return;
            }

            if (elemSpec.Equals("Vector4", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    container.Add(new XElement("item",
                        new XAttribute("type", "Vector4"),
                        new XElement("x", item.XText ?? "0"),
                        new XElement("y", item.YText ?? "0"),
                        new XElement("z", item.ZText ?? "0"),
                        new XElement("w", item.WText ?? "0")));
                }
                root.Add(container);
                return;
            }

            // Array of object type
            if (registry?.TryGetByClassName(elemSpec, out var itemSchema) == true && itemSchema is not null)
            {
                foreach (var item in vm.Children)
                {
                    var itemEl = new XElement("item");
                    var byName = item.Children.ToDictionary(c => c.Name, StringComparer.Ordinal);
                    foreach (var (fn, ts) in itemSchema.Fields)
                    {
                        if (!byName.TryGetValue(fn, out var childVm))
                            continue;
                        WriteField(itemSchema, registry, fn, ts, childVm, itemEl);
                    }
                    container.Add(itemEl);
                }
                root.Add(container);
                return;
            }

            // unsupported -> write empty container
            root.Add(container);
            return;
        }

        if (t.StartsWith("Map<", StringComparison.OrdinalIgnoreCase))
        {
            var container = new XElement(fieldName);
            container.SetAttributeValue("type", "Map");

            if (t.Contains("Vector4", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    container.Add(new XElement("item",
                        new XAttribute("key", item.Name),
                        new XAttribute("type", "Vector4"),
                        new XElement("x", item.XText ?? "0"),
                        new XElement("y", item.YText ?? "0"),
                        new XElement("z", item.ZText ?? "0"),
                        new XElement("w", item.WText ?? "0")));
                }
                root.Add(container);
                return;
            }

            if (t.Contains("Float", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    container.Add(new XElement("item",
                        new XAttribute("key", item.Name),
                        new XAttribute("type", "Float"),
                        new XElement("value", item.ScalarText ?? "0")));
                }
                root.Add(container);
                return;
            }

            // Map<String,String>
            foreach (var item in vm.Children)
            {
                container.Add(new XElement("item",
                    new XAttribute("key", item.Name),
                    new XAttribute("type", "String"),
                    new XElement("url", item.ScalarText ?? string.Empty)));
            }
            root.Add(container);
            return;
        }

        if (t.StartsWith("Enum<", StringComparison.OrdinalIgnoreCase))
        {
            root.Add(new XElement(fieldName,
                new XAttribute("type", "Enum"),
                new XElement("value", vm.EnumSelectedName ?? string.Empty)));
            return;
        }

        if (IsAssetRefSpec(t))
        {
            root.Add(new XElement(fieldName,
                new XAttribute("type", "AssetReference"),
                new XElement("url", vm.ScalarText ?? string.Empty)));
            return;
        }

        if (t == "Vector3")
        {
            root.Add(WriteVector3New(fieldName, vm));
            return;
        }

        if (t == "Vector4")
        {
            root.Add(WriteVector4New(fieldName, vm));
            return;
        }

        if (t == "Bool")
        {
            root.Add(new XElement(fieldName,
                new XAttribute("type", "Bool"),
                new XElement("value", vm.BoolValue ? "true" : "false")));
            return;
        }

        // String-like & scalar text types
        root.Add(new XElement(fieldName,
            new XAttribute("type", t),
            new XElement("value", vm.ScalarText ?? string.Empty)));
    }

    private static double ReadAttrDoubleOr0(XElement el, string attr)
    {
        var a = el.Attribute(attr)?.Value;
        if (a is null) return 0;
        return double.TryParse(a, NumberStyles.Float, CultureInfo.InvariantCulture, out var d) ? d : 0;
    }

}

