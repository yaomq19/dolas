using Assimp;
using Dolas.ModelImporter.Commands;
using Microsoft.Win32;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using static System.Formats.Asn1.AsnWriter;

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
                // TODO: 在这里添加实际的模型转换逻辑
                // 1. 调用 C++ 编写的转换库（可以使用 P/Invoke 或 CLI/C++）
                // 2. 或者在 C# 中使用 AssimpNet 进行解析
                
                string outputPath = Path.ChangeExtension(InputFilePath, ".mesh");

                // 创建 Assimp 上下文
                using (var importer = new AssimpContext())
                {
                    try
                    {
                        // 导入 FBX 文件
                        // PostProcessSteps 定义后处理操作
                        Scene scene = importer.ImportFile(InputFilePath,
                            PostProcessSteps.Triangulate |           // 三角化多边形
                            PostProcessSteps.GenerateNormals |       // 生成法线
                            PostProcessSteps.CalculateTangentSpace | // 计算切线空间
                            PostProcessSteps.FlipUVs);               // 翻转UV（OpenGL需要）

                        // 处理导入的场景
                        ProcessScene(scene);
                    }
                    catch (AssimpException ex)
                    {
                        Console.WriteLine($"导入失败: {ex.Message}");
                    }
                }

                // 模拟转换过程
                StatusMessage = $"转换完成！输出文件：{outputPath}";
            }
            catch (System.Exception ex)
            {
                StatusMessage = $"转换失败: {ex.Message}";
            }
        }

        private void ProcessScene(Scene scene)
        {
            Console.WriteLine($"网格数量: {scene.MeshCount}");
            Console.WriteLine($"材质数量: {scene.MaterialCount}");
            Console.WriteLine($"动画数量: {scene.AnimationCount}");

            // 遍历所有网格
            foreach (var mesh in scene.Meshes)
            {
                Console.WriteLine($"网格 '{mesh.Name}':");
                Console.WriteLine($"  顶点数: {mesh.VertexCount}");
                Console.WriteLine($"  面数: {mesh.FaceCount}");
                Console.WriteLine($"  材质索引: {mesh.MaterialIndex}");

                // 检查是否有纹理坐标
                if (mesh.HasTextureCoords(0))
                {
                    Console.WriteLine($"  有UV坐标");
                }

                // 检查是否有骨骼
                if (mesh.HasBones)
                {
                    Console.WriteLine($"  骨骼数: {mesh.BoneCount}");
                }
            }

            // 处理材质
            foreach (var material in scene.Materials)
            {
                Console.WriteLine($"材质: {material.Name}");

                // 获取漫反射纹理
                if (material.HasTextureDiffuse)
                {
                    var texture = material.TextureDiffuse;
                    Console.WriteLine($"  漫反射贴图: {texture.FilePath}");
                }
            }
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

