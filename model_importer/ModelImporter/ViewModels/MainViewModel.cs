using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using Microsoft.Win32;
using Dolas.ModelImporter.Commands;
using System.IO;

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
                
                // 模拟转换过程
                StatusMessage = $"转换完成！输出文件：{outputPath}";
            }
            catch (System.Exception ex)
            {
                StatusMessage = $"转换失败: {ex.Message}";
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

