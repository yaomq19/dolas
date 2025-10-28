#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
统计 source 目录下所有 .h 和 .cpp 文件的行数
"""

import os
from pathlib import Path
from typing import Dict, List, Tuple


def count_lines_in_file(file_path: Path) -> int:
    """
    统计单个文件的行数
    
    Args:
        file_path: 文件路径
        
    Returns:
        文件的行数
    """
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            return len(f.readlines())
    except Exception as e:
        print(f"警告: 无法读取文件 {file_path}: {e}")
        return 0


def scan_source_files(source_dir: Path) -> List[Tuple[Path, int]]:
    """
    递归扫描目录，查找所有 .h 和 .cpp 文件
    
    Args:
        source_dir: 源代码目录路径
        
    Returns:
        (文件路径, 行数) 的列表
    """
    file_stats = []
    
    # 递归遍历所有 .h 和 .cpp 文件
    for pattern in ['**/*.h', '**/*.cpp']:
        for file_path in source_dir.glob(pattern):
            if file_path.is_file():
                line_count = count_lines_in_file(file_path)
                file_stats.append((file_path, line_count))
    
    # 按文件路径排序
    file_stats.sort(key=lambda x: str(x[0]))
    
    return file_stats

def scan_shader_files(shader_dir: Path) -> List[Tuple[Path, int]]:
    """
    递归扫描目录，查找所有 .hlsl 和 .hlsli 文件
    
    Args:
        shader_dir: Shader 目录路径
        
    Returns:
        (文件路径, 行数) 的列表
    """
    file_stats = []
    
    # 递归遍历所有 .hlsl 和 .hlsli 文件
    for pattern in ['**/*.hlsl', '**/*.hlsli']:
        for file_path in shader_dir.glob(pattern):
            if file_path.is_file():
                line_count = count_lines_in_file(file_path)
                file_stats.append((file_path, line_count))
    
    # 按文件路径排序
    file_stats.sort(key=lambda x: str(x[0]))
    
    return file_stats

def print_statistics(file_stats: List[Tuple[Path, int]], source_dir: Path) -> Tuple[int, int]:
    """
    打印统计信息
    
    Args:
        file_stats: 文件统计列表
        source_dir: 源代码目录路径
        
    Returns:
        (总文件数, 总行数)
    """
    if not file_stats:
        print("未找到任何 .h 或 .cpp 文件")
        return (0, 0)
    
    print("=" * 80)
    print(f"源代码统计 - {source_dir}")
    print("=" * 80)
    print()
    
    # 按目录分组统计
    dir_stats: Dict[str, List[Tuple[Path, int]]] = {}
    
    for file_path, line_count in file_stats:
        relative_path = file_path.relative_to(source_dir)
        dir_name = str(relative_path.parent)
        
        if dir_name not in dir_stats:
            dir_stats[dir_name] = []
        dir_stats[dir_name].append((relative_path, line_count))
    
    # 打印每个目录的文件
    total_lines = 0
    total_files = 0
    
    for dir_name in sorted(dir_stats.keys()):
        files = dir_stats[dir_name]
        dir_lines = sum(line_count for _, line_count in files)
        
        print(f"\n[{dir_name}/]")
        print("-" * 80)
        
        for file_name, line_count in sorted(files):
            print(f"  {file_name.name:<40} {line_count:>6} 行")
        
        print(f"  {'小计:':<40} {dir_lines:>6} 行 ({len(files)} 个文件)")
        
        total_lines += dir_lines
        total_files += len(files)
    
    # 打印总计
    print()
    print("=" * 80)
    print(f"总计: {total_files} 个文件, {total_lines} 行代码")
    print("=" * 80)
    
    # 统计文件类型
    h_files = [f for f, _ in file_stats if f.suffix == '.h']
    cpp_files = [f for f, _ in file_stats if f.suffix == '.cpp']
    h_lines = sum(lc for f, lc in file_stats if f.suffix == '.h')
    cpp_lines = sum(lc for f, lc in file_stats if f.suffix == '.cpp')
    
    print()
    print("文件类型统计:")
    print(f"  .h   文件: {len(h_files):>4} 个, {h_lines:>6} 行")
    print(f"  .cpp 文件: {len(cpp_files):>4} 个, {cpp_lines:>6} 行")
    
    return (total_files, total_lines)

def print_shader_statistics(file_stats: List[Tuple[Path, int]], shader_dir: Path) -> Tuple[int, int]:
    """
    打印 Shader 统计信息
    
    Args:
        file_stats: 文件统计列表
        shader_dir: Shader 目录路径
        
    Returns:
        (总文件数, 总行数)
    """
    if not file_stats:
        print("未找到任何 .hlsl 或 .hlsli 文件")
        return (0, 0)
    
    print("=" * 80)
    print(f"Shader 统计 - {shader_dir}")
    print("=" * 80)
    print()
    
    # 按目录分组统计
    dir_stats: Dict[str, List[Tuple[Path, int]]] = {}
    
    for file_path, line_count in file_stats:
        relative_path = file_path.relative_to(shader_dir)
        dir_name = str(relative_path.parent)
        
        if dir_name not in dir_stats:
            dir_stats[dir_name] = []
        dir_stats[dir_name].append((relative_path, line_count))
    
    # 打印每个目录的文件
    total_lines = 0
    total_files = 0
    
    for dir_name in sorted(dir_stats.keys()):
        files = dir_stats[dir_name]
        dir_lines = sum(line_count for _, line_count in files)
        
        print(f"\n[{dir_name}/]")
        print("-" * 80)
        
        for file_name, line_count in sorted(files):
            print(f"  {file_name.name:<40} {line_count:>6} 行")
        
        print(f"  {'小计:':<40} {dir_lines:>6} 行 ({len(files)} 个文件)")
        
        total_lines += dir_lines
        total_files += len(files)
    
    # 打印总计
    print()
    print("=" * 80)
    print(f"总计: {total_files} 个文件, {total_lines} 行代码")
    print("=" * 80)
    
    # 统计文件类型
    hlsl_files = [f for f, _ in file_stats if f.suffix == '.hlsl']
    hlsli_files = [f for f, _ in file_stats if f.suffix == '.hlsli']
    hlsl_lines = sum(lc for f, lc in file_stats if f.suffix == '.hlsl')
    hlsli_lines = sum(lc for f, lc in file_stats if f.suffix == '.hlsli')
    
    print()
    print("文件类型统计:")
    print(f"  .hlsl  文件: {len(hlsl_files):>4} 个, {hlsl_lines:>6} 行")
    print(f"  .hlsli 文件: {len(hlsli_files):>4} 个, {hlsli_lines:>6} 行")
    
    return (total_files, total_lines)

def main():
    """主函数"""
    # 获取项目根目录（脚本在 script/ 目录下）
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    source_dir = project_root / 'source'
    shader_dir = project_root / 'content/shader'

    # 检查 source 目录是否存在
    if not source_dir.exists():
        print(f"错误: source 目录不存在: {source_dir}")
        return
    
    if not source_dir.is_dir():
        print(f"错误: {source_dir} 不是一个目录")
        return
    
    # 检查 shader 目录是否存在
    if not shader_dir.exists():
        print(f"错误: shader 目录不存在: {shader_dir}")
        return
    
    if not shader_dir.is_dir():
        print(f"错误: {shader_dir} 不是一个目录")
        return

    # 扫描并统计
    print(f"正在扫描 {source_dir} ...")
    print()
    file_stats = scan_source_files(source_dir)
    shader_file_stats = scan_shader_files(shader_dir)

    # 打印详细统计
    source_total_files, source_total_lines = print_statistics(file_stats, source_dir)
    print("\n")
    shader_total_files, shader_total_lines = print_shader_statistics(shader_file_stats, shader_dir)
    
    # 打印最终总结（醒目显示）
    print("\n\n")
    print("█" * 80)
    print("█" + " " * 78 + "█")
    print("█" + "                        📊 项目代码统计总结 📊                           ".center(78) + "█")
    print("█" + " " * 78 + "█")
    print("█" * 80)
    print("█" + " " * 78 + "█")
    print("█" + f"  C++ 源代码 (.h + .cpp):      {source_total_lines:>10,} 行  ({source_total_files:>3} 个文件)".ljust(78) + "█")
    print("█" + " " * 78 + "█")
    print("█" + f"  HLSL 着色器 (.hlsl + .hlsli): {shader_total_lines:>10,} 行  ({shader_total_files:>3} 个文件)".ljust(78) + "█")
    print("█" + " " * 78 + "█")
    print("█" + "─" * 78 + "█")
    print("█" + " " * 78 + "█")
    total_lines = source_total_lines + shader_total_lines
    total_files = source_total_files + shader_total_files
    print("█" + f"  🎯 总代码行数:                {total_lines:>10,} 行  ({total_files:>3} 个文件)".ljust(78) + "█")
    print("█" + " " * 78 + "█")
    print("█" * 80)

if __name__ == '__main__':
    main()

