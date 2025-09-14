#!/usr/bin/env python3
"""
HLSL Shader Compilation Script
==============================
This script automatically compiles HLSL shader files found in the shaders/ directory.
It preserves directory structure and handles multiple entry points per file.

Usage:
    python scripts/CompileShader.py [--output-dir <dir>] [--verbose]

Entry Point Naming Convention:
    VS_Entry*  -> Vertex Shader   (compiled with vs_5_0)
    PS_Entry*  -> Pixel Shader    (compiled with ps_5_0)
    CS_Entry*  -> Compute Shader  (compiled with cs_5_0)

Output Naming:
    filename_EntryPointName.cso
"""

import os
import sys
import re
import subprocess
import argparse
from pathlib import Path
from typing import List, Tuple, Dict


class ShaderCompiler:
    """HLSL Shader Compiler with automatic entry point detection"""
    
    def __init__(self, project_root: str, output_dir: str = None, verbose: bool = False):
        self.project_root = Path(project_root).resolve()
        self.shaders_dir = self.project_root / "shaders"
        self.verbose = verbose
        
        # 确定输出目录
        if output_dir:
            self.output_dir = Path(output_dir).resolve()
        else:
            # 默认使用 build/bin/shaders
            self.output_dir = self.project_root / "build" / "bin" / "shaders"
        
        # 着色器类型配置
        self.shader_types = {
            'VS_Entry': {'profile': 'vs_5_0', 'description': 'Vertex Shader'},
            'PS_Entry': {'profile': 'ps_5_0', 'description': 'Pixel Shader'},
            'CS_Entry': {'profile': 'cs_5_0', 'description': 'Compute Shader'}
        }
        
        self.log(f"Project Root: {self.project_root}")
        self.log(f"Shaders Directory: {self.shaders_dir}")
        self.log(f"Output Directory: {self.output_dir}")
    
    def log(self, message: str):
        """打印日志信息"""
        if self.verbose:
            print(f"[ShaderCompiler] {message}")
    
    def find_hlsl_files(self) -> List[Path]:
        """递归搜索所有HLSL文件"""
        if not self.shaders_dir.exists():
            print(f"ERROR: Shaders directory not found: {self.shaders_dir}")
            return []
        
        hlsl_files = list(self.shaders_dir.rglob("*.hlsl"))
        self.log(f"Found {len(hlsl_files)} HLSL files")
        
        for file in hlsl_files:
            self.log(f"  - {file.relative_to(self.project_root)}")
        
        return hlsl_files
    
    def extract_entry_points(self, hlsl_file: Path) -> Dict[str, List[str]]:
        """从HLSL文件中提取所有入口点函数"""
        try:
            with open(hlsl_file, 'r', encoding='utf-8') as f:
                content = f.read()
        except Exception as e:
            print(f"ERROR: Failed to read {hlsl_file}: {e}")
            return {}
        
        entry_points = {}
        
        for prefix in self.shader_types.keys():
            # 匹配模式: void/type VS_Entry_xxx( 或 [attributes] void/type VS_Entry_xxx(
            # 支持返回类型（如PS_INPUT, float4等）和void
            pattern = rf'(?:\[[^\]]*\]\s*)?(?:void|[a-zA-Z_][a-zA-Z0-9_]*)\s+({prefix}[a-zA-Z0-9_]*)\s*\('
            matches = re.findall(pattern, content, re.MULTILINE | re.IGNORECASE)
            
            if matches:
                entry_points[prefix] = matches
                self.log(f"  Found {len(matches)} {prefix} entry points: {matches}")
        
        return entry_points
    
    def ensure_output_directory(self, hlsl_file: Path):
        """确保输出目录存在，保持与源码相同的目录结构"""
        # 计算相对路径
        relative_path = hlsl_file.relative_to(self.shaders_dir)
        output_subdir = self.output_dir / relative_path.parent
        
        # 创建目录
        output_subdir.mkdir(parents=True, exist_ok=True)
        return output_subdir
    
    def compile_shader(self, hlsl_file: Path, entry_point: str, shader_type: str) -> bool:
        """编译单个着色器入口点"""
        output_subdir = self.ensure_output_directory(hlsl_file)
        
        # 生成输出文件名：filename_EntryPointName.cso
        filename_stem = hlsl_file.stem
        output_filename = f"{filename_stem}_{entry_point}.cso"
        output_file = output_subdir / output_filename
        
        # 获取着色器配置
        config = self.shader_types[shader_type]
        profile = config['profile']
        description = config['description']
        
        # 构建fxc编译命令
        cmd = [
            'fxc.exe',
            '/T', profile,              # 目标配置文件
            '/E', entry_point,          # 入口点函数名
            '/Fo', str(output_file),    # 输出文件
            str(hlsl_file)              # 源文件
        ]
        
        self.log(f"Compiling {description}: {entry_point}")
        self.log(f"  Command: {' '.join(cmd)}")
        
        try:
            # 执行编译
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            
            print(f"yes {description}: {hlsl_file.name} -> {entry_point} -> {output_filename}")
            
            if result.stdout and self.verbose:
                self.log(f"  Output: {result.stdout.strip()}")
            
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"x FAILED to compile {entry_point} in {hlsl_file.name}")
            print(f"  Error Code: {e.returncode}")
            if e.stdout:
                print(f"  Stdout: {e.stdout}")
            if e.stderr:
                print(f"  Stderr: {e.stderr}")
            return False
        
        except FileNotFoundError:
            print("ERROR: fxc.exe not found! Please ensure DirectX SDK is installed and in PATH.")
            return False
    
    def compile_all_shaders(self) -> bool:
        """编译所有找到的着色器文件"""
        hlsl_files = self.find_hlsl_files()
        
        if not hlsl_files:
            print("No HLSL files found to compile.")
            return True
        
        total_compiled = 0
        total_failed = 0
        
        print(f"\n=== Compiling HLSL Shaders ===")
        print(f"Output Directory: {self.output_dir}")
        print(f"Found {len(hlsl_files)} HLSL files to process\n")
        
        for hlsl_file in hlsl_files:
            relative_path = hlsl_file.relative_to(self.project_root)
            print(f"Processing: {relative_path}")
            
            entry_points = self.extract_entry_points(hlsl_file)
            
            if not entry_points:
                print(f"  No entry points found in {hlsl_file.name}")
                continue
            
            file_compiled = 0
            file_failed = 0
            
            # 编译每种类型的入口点
            for shader_type, entries in entry_points.items():
                for entry_point in entries:
                    if self.compile_shader(hlsl_file, entry_point, shader_type):
                        file_compiled += 1
                        total_compiled += 1
                    else:
                        file_failed += 1
                        total_failed += 1
            
            print(f"  Result: {file_compiled} compiled, {file_failed} failed\n")
        
        # 打印总结
        print(f"=== Compilation Summary ===")
        print(f"Total Entry Points Compiled: {total_compiled}")
        print(f"Total Failures: {total_failed}")
        
        return total_failed == 0


def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="Compile HLSL shaders with automatic entry point detection",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument(
        '--output-dir', '-o',
        help='Output directory for compiled shaders (default: build/bin/shaders)'
    )
    
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Enable verbose output'
    )
    
    parser.add_argument(
        '--project-root',
        default='.',
        help='Project root directory (default: current directory)'
    )
    
    args = parser.parse_args()
    
    # 创建编译器实例
    compiler = ShaderCompiler(
        project_root=args.project_root,
        output_dir=args.output_dir,
        verbose=args.verbose
    )
    
    # 执行编译
    success = compiler.compile_all_shaders()
    
    if success:
        print("\n🎉 All shaders compiled successfully!")
        sys.exit(0)
    else:
        print("\n❌ Some shaders failed to compile.")
        sys.exit(1)


if __name__ == "__main__":
    main() 