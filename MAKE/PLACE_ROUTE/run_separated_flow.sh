#!/bin/bash

# 分离的布局和布线流程演示脚本

echo "=== 分离的布局和布线流程 ==="

# 设置路径
CSYN_FP_DIR="./csyn_fp"
BUILD_DIR="$CSYN_FP_DIR/build"
PLACEMENT_ONLY="$BUILD_DIR/placement_only"
ROUTING_ONLY="$BUILD_DIR/routing_only"

# 输入文件
CDL_FILE="DATA/input/asap7sc7p5t_split.sp"
DR_FILE="DATA/input/placement_file.style"
OUTPUT_DIR="output_separated"

# 创建输出目录
mkdir -p "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR/placement_results"
mkdir -p "$OUTPUT_DIR/routing_results"

echo "1. 编译项目..."
cd "$CSYN_FP_DIR"
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
make -j4

if [ ! -f "placement_only" ] || [ ! -f "routing_only" ]; then
    echo "错误: 编译失败，无法找到可执行文件"
    exit 1
fi

echo "2. 执行布局阶段..."
cd ../..
$PLACEMENT_ONLY -i "$CDL_FILE" -d "$DR_FILE" -o "$OUTPUT_DIR"

if [ $? -ne 0 ]; then
    echo "错误: 布局阶段失败"
    exit 1
fi

echo "3. 查找布局结果文件..."
PLACEMENT_FILES=$(find "$OUTPUT_DIR/placement_results" -name "*.placement" | head -3)

if [ -z "$PLACEMENT_FILES" ]; then
    echo "错误: 未找到布局结果文件"
    exit 1
fi

echo "找到以下布局结果文件:"
echo "$PLACEMENT_FILES"

echo "4. 执行布线阶段..."
for placement_file in $PLACEMENT_FILES; do
    echo "处理布局文件: $placement_file"
    
    # 提取文件名（不含路径和扩展名）
    filename=$(basename "$placement_file" .placement)
    
    # 执行布线
    $ROUTING_ONLY -i "$CDL_FILE" -d "$DR_FILE" -p "$placement_file" -o "$OUTPUT_DIR/routing_results"
    
    if [ $? -eq 0 ]; then
        echo "布线成功: $filename"
    else
        echo "布线失败: $filename"
    fi
done

echo "5. 流程完成！"
echo "布局结果保存在: $OUTPUT_DIR/placement_results/"
echo "布线结果保存在: $OUTPUT_DIR/routing_results/"

echo "=== 分离流程演示完成 ==="
