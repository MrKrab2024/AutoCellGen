#!/bin/bash

# 布局算法比较演示脚本

echo "=== 布局算法比较演示 ==="

# 设置路径
CSYN_FP_DIR="./csyn_fp"
BUILD_DIR="$CSYN_FP_DIR/build"
COMPARISON_PROGRAM="$BUILD_DIR/placement_comparison"

# 输入文件
CDL_FILE="DATA/input/asap7sc7p5t_split.sp"
DR_FILE="DATA/input/placement_file.style"
OUTPUT_DIR="output_comparison"

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

echo "1. 编译项目..."
cd "$CSYN_FP_DIR"
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
make -j4

if [ ! -f "placement_comparison" ]; then
    echo "错误: 编译失败，无法找到比较程序"
    exit 1
fi

echo "2. 运行布局算法比较..."
cd ../..
$COMPARISON_PROGRAM -i "$CDL_FILE" -d "$DR_FILE" -o "$OUTPUT_DIR"

if [ $? -ne 0 ]; then
    echo "错误: 布局算法比较失败"
    exit 1
fi

echo "3. 查看比较结果..."
echo "比较结果保存在: $OUTPUT_DIR/placement_comparison/"
echo "包含以下文件:"
ls -la "$OUTPUT_DIR/placement_comparison/"

echo "4. 生成比较报告..."
echo "每个单元的详细比较报告:"
for cell_dir in "$OUTPUT_DIR/placement_comparison"/*/; do
    if [ -d "$cell_dir" ]; then
        cell_name=$(basename "$cell_dir")
        echo "  单元: $cell_name"
        if [ -f "$cell_dir/placement_comparison.csv" ]; then
            echo "    CSV报告: $cell_dir/placement_comparison.csv"
        fi
        if [ -f "$cell_dir/placement_comparison.html" ]; then
            echo "    HTML报告: $cell_dir/placement_comparison.html"
        fi
    fi
done

echo "5. 比较完成！"
echo "=== 布局算法比较演示完成 ==="
