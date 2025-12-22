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
                    var v = (string?)child.Attribute("file") ?? child.Value ?? string.Empty;
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
                    itemVm.SetVector3(ReadAttrDoubleOr0(child, "x"), ReadAttrDoubleOr0(child, "y"), ReadAttrDoubleOr0(child, "z"));
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
                    itemVm.SetVector4(ReadAttrDoubleOr0(child, "x"), ReadAttrDoubleOr0(child, "y"), ReadAttrDoubleOr0(child, "z"), ReadAttrDoubleOr0(child, "w"));
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

            // Map<String, Vector4>: <vec4 name="..." x="" y="" z="" w=""/>
            if (t.Contains("Vector4", StringComparison.OrdinalIgnoreCase))
            {
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

            // Map<String, Float>: <float name="..." value="..."/>
            if (t.Contains("Float", StringComparison.OrdinalIgnoreCase))
            {
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

            // Map<String, String>: <texture name="..." file="..."/>
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
            vm.SetScalar(el.Value ?? string.Empty, EditorType.String);
            return;
        }

        if (t == "Bool")
        {
            if (bool.TryParse(el.Value, out var b)) vm.SetBool(b);
            else
            {
                mutated = true;
                warnings.Add($"字段解析失败：{fieldName} 期望 Bool，实际 '{el.Value}'（已使用默认值）");
            }
            return;
        }
        if (t == "Int")
        {
            if (long.TryParse(el.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var i)) vm.SetScalar(i.ToString(CultureInfo.InvariantCulture), EditorType.Int);
            else { mutated = true; warnings.Add($"字段解析失败：{fieldName} 期望 Int，实际 '{el.Value}'（已使用默认值）"); }
            return;
        }
        if (t == "UInt")
        {
            if (ulong.TryParse(el.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var u)) vm.SetScalar(u.ToString(CultureInfo.InvariantCulture), EditorType.UInt);
            else { mutated = true; warnings.Add($"字段解析失败：{fieldName} 期望 UInt，实际 '{el.Value}'（已使用默认值）"); }
            return;
        }
        if (t == "Float")
        {
            if (double.TryParse(el.Value, NumberStyles.Float, CultureInfo.InvariantCulture, out var f)) vm.SetScalar(f.ToString(CultureInfo.InvariantCulture), EditorType.Float);
            else { mutated = true; warnings.Add($"字段解析失败：{fieldName} 期望 Float，实际 '{el.Value}'（已使用默认值）"); }
            return;
        }

        if (t == "Vector3")
        {
            vm.SetVector3(ReadAttrDoubleOr0(el, "x"), ReadAttrDoubleOr0(el, "y"), ReadAttrDoubleOr0(el, "z"));
            return;
        }
        if (t == "Vector4")
        {
            vm.SetVector4(ReadAttrDoubleOr0(el, "x"), ReadAttrDoubleOr0(el, "y"), ReadAttrDoubleOr0(el, "z"), ReadAttrDoubleOr0(el, "w"));
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
            var isStringLike = elemSpec.Equals("String", StringComparison.OrdinalIgnoreCase) ||
                               elemSpec.Equals("RawReference", StringComparison.OrdinalIgnoreCase) ||
                               IsAssetRefSpec(elemSpec);

            if (isStringLike)
            {
                foreach (var item in vm.Children)
                    container.Add(new XElement("item", item.ScalarText ?? string.Empty));
                root.Add(container);
                return;
            }

            if (elemSpec.Equals("Vector3", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    var el = new XElement("item");
                    el.SetAttributeValue("x", item.XText);
                    el.SetAttributeValue("y", item.YText);
                    el.SetAttributeValue("z", item.ZText);
                    container.Add(el);
                }
                root.Add(container);
                return;
            }

            if (elemSpec.Equals("Vector4", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    var el = new XElement("item");
                    el.SetAttributeValue("x", item.XText);
                    el.SetAttributeValue("y", item.YText);
                    el.SetAttributeValue("z", item.ZText);
                    el.SetAttributeValue("w", item.WText);
                    container.Add(el);
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

            if (t.Contains("Vector4", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    var v = new XElement("vec4");
                    v.SetAttributeValue("name", item.Name);
                    v.SetAttributeValue("x", item.XText);
                    v.SetAttributeValue("y", item.YText);
                    v.SetAttributeValue("z", item.ZText);
                    v.SetAttributeValue("w", item.WText);
                    container.Add(v);
                }
                root.Add(container);
                return;
            }

            if (t.Contains("Float", StringComparison.OrdinalIgnoreCase))
            {
                foreach (var item in vm.Children)
                {
                    var f = new XElement("float");
                    f.SetAttributeValue("name", item.Name);
                    f.SetAttributeValue("value", item.ScalarText ?? "0");
                    container.Add(f);
                }
                root.Add(container);
                return;
            }

            // Map<String,String>
            foreach (var item in vm.Children)
            {
                var tex = new XElement("texture");
                tex.SetAttributeValue("name", item.Name);
                tex.SetAttributeValue("file", item.ScalarText ?? string.Empty);
                container.Add(tex);
            }
            root.Add(container);
            return;
        }

        if (t.StartsWith("Enum<", StringComparison.OrdinalIgnoreCase))
        {
            root.Add(new XElement(fieldName, vm.EnumSelectedName ?? string.Empty));
            return;
        }

        if (t == "Vector3")
        {
            var el = new XElement(fieldName);
            el.SetAttributeValue("x", vm.XText);
            el.SetAttributeValue("y", vm.YText);
            el.SetAttributeValue("z", vm.ZText);
            root.Add(el);
            return;
        }

        if (t == "Vector4")
        {
            var el = new XElement(fieldName);
            el.SetAttributeValue("x", vm.XText);
            el.SetAttributeValue("y", vm.YText);
            el.SetAttributeValue("z", vm.ZText);
            el.SetAttributeValue("w", vm.WText);
            root.Add(el);
            return;
        }

        if (t == "Bool")
        {
            root.Add(new XElement(fieldName, vm.BoolValue ? "true" : "false"));
            return;
        }

        // String-like & scalar text types
        root.Add(new XElement(fieldName, vm.ScalarText ?? string.Empty));
    }

    private static double ReadAttrDoubleOr0(XElement el, string attr)
    {
        var a = el.Attribute(attr)?.Value;
        if (a is null) return 0;
        return double.TryParse(a, NumberStyles.Float, CultureInfo.InvariantCulture, out var d) ? d : 0;
    }

}

