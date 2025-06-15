#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DDS Texture Generator
Convert image files in content/textures directory to DDS format with mipmaps
Supported formats: .jpg, .png, .bmp
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path
import glob

class DDSGenerator:
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.project_root = self.script_dir.parent
        self.content_textures_dir = self.project_root / "content" / "textures"
        self.texconv_exe = None
        
        # Supported input formats
        self.supported_formats = ['.jpg', '.jpeg', '.png', '.bmp', '.tga', '.tiff']
        
        # Find texconv.exe
        self._find_texconv()
    
    def _find_texconv(self):
        """Find texconv.exe tool"""
        # Common DirectXTex tool paths
        possible_paths = [
            # Project local paths
            self.project_root / "tools" / "texconv.exe",
            self.script_dir / "texconv.exe",
            
            # Windows SDK paths
            "C:/Program Files (x86)/Windows Kits/10/bin/x64/texconv.exe",
            "C:/Program Files (x86)/Windows Kits/10/bin/x86/texconv.exe",
            
            # DirectXTex installation paths
            "C:/Program Files/Microsoft DirectX SDK/Utilities/bin/x64/texconv.exe",
            "C:/Program Files/Microsoft DirectX SDK/Utilities/bin/x86/texconv.exe",
        ]
        
        # Check PATH environment variable
        try:
            result = subprocess.run(['where', 'texconv.exe'], 
                                  capture_output=True, text=True, shell=True)
            if result.returncode == 0:
                self.texconv_exe = result.stdout.strip().split('\n')[0]
                print(f"✓ Found texconv.exe in PATH: {self.texconv_exe}")
                return
        except:
            pass
        
        # Check possible paths
        for path in possible_paths:
            if isinstance(path, str):
                path = Path(path)
            if path.exists():
                self.texconv_exe = str(path)
                print(f"✓ Found texconv.exe: {self.texconv_exe}")
                return
        
        print("❌ Error: texconv.exe tool not found")
        print("\nPlease obtain texconv.exe by:")
        print("1. Download from https://github.com/Microsoft/DirectXTex/releases")
        print("2. Place texconv.exe in one of these locations:")
        print(f"   - {self.project_root}/tools/texconv.exe")
        print(f"   - {self.script_dir}/texconv.exe")
        print("3. Or ensure texconv.exe is in system PATH")
        
    def create_output_directory(self):
        """Create output directory"""
        if not self.content_textures_dir.exists():
            print(f"Creating textures directory: {self.content_textures_dir}")
            self.content_textures_dir.mkdir(parents=True, exist_ok=True)
        
    def find_image_files(self):
        """Find all supported image files"""
        image_files = []
        
        print(f"🔍 Scanning directory: {self.content_textures_dir}")
        
        for format_ext in self.supported_formats:
            # Recursively search all supported formats
            pattern = f"**/*{format_ext}"
            files = list(self.content_textures_dir.glob(pattern))
            files.extend(list(self.content_textures_dir.glob(pattern.upper())))
            image_files.extend(files)
        
        # Remove duplicates and sort
        image_files = list(set(image_files))
        image_files.sort()
        
        print(f"📁 Found {len(image_files)} image files")
        return image_files
    
    def convert_to_dds(self, input_file, force_update=False):
        """Convert single image file to DDS format"""
        input_path = Path(input_file)
        output_path = input_path.with_suffix('.dds')
        
        # Check if update is needed
        if not force_update and output_path.exists():
            if output_path.stat().st_mtime > input_path.stat().st_mtime:
                print(f"⏭️  Skip {input_path.name} (DDS file is up to date)")
                return True
        
        print(f"🔄 Converting: {input_path.name} -> {output_path.name}")
        
        try:
            # texconv command line arguments
            cmd = [
                self.texconv_exe,
                "-f", "BC3_UNORM",      # DXT5 compression format
                "-m", "0",              # Generate full mipmap chain
                "-o", str(output_path.parent),  # Output directory
                str(input_path)         # Input file
            ]
            
            # Execute conversion
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print(f"✅ Successfully converted: {output_path.name}")
                return True
            else:
                print(f"❌ Conversion failed: {input_path.name}")
                print(f"   Error output: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"❌ Conversion exception: {input_path.name} - {str(e)}")
            return False
    
    def generate_all(self, force_update=False):
        """Generate all DDS files"""
        if not self.texconv_exe:
            print("❌ Cannot execute conversion: texconv.exe not found")
            return False
        
        self.create_output_directory()
        image_files = self.find_image_files()
        
        if not image_files:
            print("📭 No image files found to convert")
            return True
        
        print(f"\n🚀 Starting conversion of {len(image_files)} files...")
        print("=" * 50)
        
        success_count = 0
        total_count = len(image_files)
        
        for i, image_file in enumerate(image_files, 1):
            print(f"[{i}/{total_count}] ", end="")
            if self.convert_to_dds(image_file, force_update):
                success_count += 1
        
        print("=" * 50)
        print(f"🎉 Conversion completed: {success_count}/{total_count} files successful")
        
        if success_count < total_count:
            print(f"⚠️  {total_count - success_count} files failed to convert")
            return False
        
        return True
    
    def clean_dds_files(self):
        """Clean all generated DDS files"""
        dds_files = list(self.content_textures_dir.glob("**/*.dds"))
        
        if not dds_files:
            print("📭 No DDS files found to clean")
            return
        
        print(f"🗑️  Cleaning {len(dds_files)} DDS files...")
        
        for dds_file in dds_files:
            try:
                dds_file.unlink()
                print(f"🗑️  Deleted: {dds_file.name}")
            except Exception as e:
                print(f"❌ Failed to delete: {dds_file.name} - {str(e)}")
        
        print("✅ Cleanup completed")
    
    def print_usage_info(self):
        """Print usage information"""
        print("\n" + "=" * 60)
        print("📖 DDS Texture Generator Usage Guide")
        print("=" * 60)
        print(f"Textures directory: {self.content_textures_dir}")
        print(f"Supported formats: {', '.join(self.supported_formats)}")
        print("DDS settings: BC3_UNORM compression + full mipmap chain")
        print("\nCommand line arguments:")
        print("  python DDSGenerate.py           # Generate all DDS files")
        print("  python DDSGenerate.py --force   # Force regenerate all files")
        print("  python DDSGenerate.py --clean   # Clean all DDS files")
        print("  python DDSGenerate.py --help    # Show help information")

def main():
    parser = argparse.ArgumentParser(description='DDS Texture Generator')
    parser.add_argument('--force', action='store_true', 
                       help='Force regenerate all DDS files')
    parser.add_argument('--clean', action='store_true', 
                       help='Clean all generated DDS files')
    parser.add_argument('--info', action='store_true', 
                       help='Show usage information')
    
    args = parser.parse_args()
    
    generator = DDSGenerator()
    
    if args.info:
        generator.print_usage_info()
        return
    
    if args.clean:
        generator.clean_dds_files()
        return
    
    # Default behavior: generate DDS files
    success = generator.generate_all(force_update=args.force)
    
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main() 