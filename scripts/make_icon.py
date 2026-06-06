"""make_icon.py — 从原始 PNG 中心裁剪正方形，缩放到指定尺寸。

用法:
    # 生成单张指定尺寸的 PNG
    python scripts/make_icon.py 16 16
    python scripts/make_icon.py 256 256

    # 生成 Windows 标准 ICO（包含 16/32/48/256 四个尺寸）
    python scripts/make_icon.py --ico

    # 指定输入文件
    python scripts/make_icon.py 48 48 --input path/to/logo.png
"""

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("错误: 需要 Pillow 库。请运行: pip install Pillow")


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
DEFAULT_INPUT = PROJECT_ROOT / "rc" / "Dolas.png"

# Windows 应用图标标准正方形尺寸
STANDARD_ICO_SIZES = [16, 32, 48, 256]


def crop_center_square(img: Image.Image) -> Image.Image:
    """从图像中心裁剪出最大正方形。"""
    w, h = img.size
    side = min(w, h)
    left = (w - side) // 2
    top = (h - side) // 2
    return img.crop((left, top, left + side, top + side))


def make_icon(input_path: Path, width: int, height: int, output_path: Path) -> Path:
    """裁剪中心正方形 → 缩放到 W×H → 保存。"""

    if not input_path.exists():
        sys.exit(f"错误: 输入文件不存在: {input_path}")

    img = Image.open(input_path)
    square = crop_center_square(img)
    resized = square.resize((width, height), Image.LANCZOS)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    resized.save(output_path)
    print(f"已生成: {output_path} ({width}×{height})")
    return output_path


def make_ico(input_path: Path, output_path: Path):
    """生成包含多个正方形尺寸的 .ico 文件。"""

    if not input_path.exists():
        sys.exit(f"错误: 输入文件不存在: {input_path}")

    img = Image.open(input_path)
    square = crop_center_square(img)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    square.save(
        output_path,
        format="ICO",
        sizes=[(s, s) for s in STANDARD_ICO_SIZES],
    )
    print(f"已生成: {output_path} (包含尺寸: {', '.join(f'{s}×{s}' for s in STANDARD_ICO_SIZES)})")


def main():
    parser = argparse.ArgumentParser(description="从原始 PNG 中心裁剪正方形并缩放")
    parser.add_argument("width", nargs="?", type=int, help="输出宽度 (像素)")
    parser.add_argument("height", nargs="?", type=int, help="输出高度 (像素)")
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT, help=f"输入 PNG 路径 (默认: {DEFAULT_INPUT})")
    parser.add_argument("--output", type=Path, default=None, help="输出路径 (默认: 自动命名)")
    parser.add_argument("--ico", action="store_true", help="生成 Windows 标准 ICO 文件 (16/32/48/256)")

    args = parser.parse_args()

    # --ico 模式
    if args.ico:
        out = args.output or (PROJECT_ROOT / "rc" / "Dolas.ico")
        make_ico(args.input, out)
        return

    # 单尺寸模式
    if args.width is None or args.height is None:
        parser.error("请指定 width 和 height，或使用 --ico 生成标准 ICO")

    if args.output:
        out = args.output
    else:
        stem = args.input.stem
        out = args.input.parent / f"{stem}_{args.width}x{args.height}.png"

    make_icon(args.input, args.width, args.height, out)


if __name__ == "__main__":
    main()
