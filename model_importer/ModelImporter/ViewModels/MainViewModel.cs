using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using Microsoft.Win32;
using Dolas.ModelImporter.Commands;
using System.IO;
using System.Xml.Linq;
using Assimp;
using System.Linq;
using System.Collections.Generic;
using System;

namespace Dolas.ModelImporter.ViewModels
{
    public class MainViewModel : INotifyPropertyChanged
    {
        private string _inputFilePath = string.Empty;
        private string _statusMessage = "就绪";

        public string InputFilePath
        {
            get => _inputFilePath;
            set
            {
                if (SetProperty(ref _inputFilePath, value))
                {
                    OnPropertyChanged(nameof(CanConvert));
                    ((RelayCommand)ConvertCommand).RaiseCanExecuteChanged();
                }
            }
        }

        public string StatusMessage
        {
            get => _statusMessage;
            set => SetProperty(ref _statusMessage, value);
        }

        public bool CanConvert => !string.IsNullOrWhiteSpace(InputFilePath) && File.Exists(InputFilePath);

        public ICommand BrowseCommand { get; }
        public ICommand ConvertCommand { get; }

        public MainViewModel()
        {
            BrowseCommand = new RelayCommand(ExecuteBrowse);
            ConvertCommand = new RelayCommand(ExecuteConvert, () => CanConvert);
        }

        private void ExecuteBrowse()
        {
            var dialog = new OpenFileDialog
            {
                Filter = "模型文件 (*.fbx;*.obj;*.gltf;*.glb)|*.fbx;*.obj;*.gltf;*.glb|所有文件 (*.*)|*.*",
                Title = "选择输入的模型文件"
            };

            if (dialog.ShowDialog() == true)
            {
                InputFilePath = dialog.FileName;
            }
        }

        private void ExecuteConvert()
        {
            StatusMessage = "正在转换模型...";
            
            try
            {
                // 创建 Assimp 上下文
                using (var importer = new AssimpContext())
                {
                    // 常见的预处理步骤：
                    // Triangulate: 确保所有面都是三角形
                    // GenerateNormals: 如果模型没法线则生成
                    // JoinIdenticalVertices: 合并重复顶点以压缩数据
                    // FlipUVs: DirectX 需要翻转 V 坐标
                    Scene scene = importer.ImportFile(InputFilePath,
                        PostProcessSteps.Triangulate |           
                        PostProcessSteps.GenerateNormals |       
                        PostProcessSteps.CalculateTangentSpace |
                        PostProcessSteps.JoinIdenticalVertices | 
                        PostProcessSteps.FlipUVs);

                    if (scene == null || !scene.HasMeshes)
                    {
                        StatusMessage = "错误：模型中没有有效的网格数据。";
                        return;
                    }

                    ProcessScene(scene);
                }
            }
            catch (Exception ex)
            {
                StatusMessage = $"转换失败: {ex.Message}";
            }
        }

        private void ProcessScene(Scene scene)
        {
            int meshCount = scene.MeshCount;
            string baseFileName = Path.GetFileNameWithoutExtension(InputFilePath);
            string outputDir = Path.GetDirectoryName(InputFilePath) ?? string.Empty;

            for (int i = 0; i < meshCount; i++)
            {
                var mesh = scene.Meshes[i];
                string meshName = string.IsNullOrWhiteSpace(mesh.Name) ? $"mesh_{i}" : mesh.Name;
                // 移除非法字符
                foreach (char c in Path.GetInvalidFileNameChars()) { meshName = meshName.Replace(c, '_'); }
                
                string outPath = Path.Combine(outputDir, $"{baseFileName}_{meshName}.mesh");

                XElement root = new XElement("asset");

                // 1. 导出位置 (X, Y, Z)
                XElement posEl = new XElement("position", new XAttribute("type", "DynamicArray"));
                foreach (var v in mesh.Vertices)
                {
                    posEl.Add(new XElement("value", v.X));
                    posEl.Add(new XElement("value", v.Y));
                    posEl.Add(new XElement("value", v.Z));
                }
                root.Add(posEl);

                // 2. 导出法线 (X, Y, Z)
                if (mesh.HasNormals)
                {
                    XElement normEl = new XElement("normal", new XAttribute("type", "DynamicArray"));
                    foreach (var n in mesh.Normals)
                    {
                        normEl.Add(new XElement("value", n.X));
                        normEl.Add(new XElement("value", n.Y));
                        normEl.Add(new XElement("value", n.Z));
                    }
                    root.Add(normEl);
                }

                // 2.5 导出切线 (X, Y, Z)
                if (mesh.HasTangentBasis)
                {
                    XElement tangEl = new XElement("tangent", new XAttribute("type", "DynamicArray"));
                    foreach (var t in mesh.Tangents)
                    {
                        tangEl.Add(new XElement("value", t.X));
                        tangEl.Add(new XElement("value", t.Y));
                        tangEl.Add(new XElement("value", t.Z));
                    }
                    root.Add(tangEl);
                }

                // 3. 导出 UV0 (U, V)
                if (mesh.HasTextureCoords(0))
                {
                    XElement uv0El = new XElement("uv0", new XAttribute("type", "DynamicArray"));
                    foreach (var uv in mesh.TextureCoordinateChannels[0])
                    {
                        uv0El.Add(new XElement("value", uv.X));
                        uv0El.Add(new XElement("value", uv.Y));
                    }
                    root.Add(uv0El);
                }

                // 4. 导出 UV1 (如果有)
                if (mesh.HasTextureCoords(1))
                {
                    XElement uv1El = new XElement("uv1", new XAttribute("type", "DynamicArray"));
                    foreach (var uv in mesh.TextureCoordinateChannels[1])
                    {
                        uv1El.Add(new XElement("value", uv.X));
                        uv1El.Add(new XElement("value", uv.Y));
                    }
                    root.Add(uv1El);
                }

                // 5. 导出顶点颜色 (R, G, B, A)
                if (mesh.HasVertexColors(0))
                {
                    XElement colorEl = new XElement("color", new XAttribute("type", "DynamicArray"));
                    foreach (var c in mesh.VertexColorChannels[0])
                    {
                        colorEl.Add(new XElement("value", c.R));
                        colorEl.Add(new XElement("value", c.G));
                        colorEl.Add(new XElement("value", c.B));
                        colorEl.Add(new XElement("value", c.A));
                    }
                    root.Add(colorEl);
                }

                // 6. 导出索引
                XElement indicesEl = new XElement("indices", new XAttribute("type", "DynamicArray"));
                foreach (var face in mesh.Faces)
                {
                    // Assimp 已执行 Triangulate，所以这里一定是 3 个索引
                    foreach (var idx in face.Indices)
                    {
                        indicesEl.Add(new XElement("value", (uint)idx));
                    }
                }
                root.Add(indicesEl);

                // 7. 拓扑结构 (默认为 triangleList)
                root.Add(new XElement("topology", new XAttribute("type", "Enum"), 
                    new XElement("value", "triangleList")));

                // 8. 材质引用 (占位默认值)
                root.Add(new XElement("material", new XAttribute("type", "AssetReference"), 
                    new XElement("value", "default.material")));

                // 保存文件
                XDocument doc = new XDocument(new XDeclaration("1.0", "utf-8", "yes"), root);
                doc.Save(outPath);
            }

            StatusMessage = $"转换完成！共生成 {meshCount} 个 .mesh 文件。";
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string? propertyName = null)
        {
            if (Equals(storage, value)) return false;
            storage = value;
            OnPropertyChanged(propertyName);
            return true;
        }
    }
}
