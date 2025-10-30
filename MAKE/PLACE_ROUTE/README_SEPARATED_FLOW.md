# 分离的布局和布线流程

本文档描述了如何使用分离的布局和布线功能，将原本集成的布局和布线过程拆分为两个独立的阶段。

## 概述

分离的布局和布线流程将原本的集成流程分为两个独立阶段：

1. **布局阶段**：生成布局结果文件（.placement格式）
2. **布线阶段**：读取布局结果文件并执行布线

这种分离提供了以下优势：
- 可以独立调试和优化布局和布线算法
- 可以重用布局结果进行不同的布线实验
- 便于并行处理和分布式计算
- 更好的模块化和可维护性

## 文件结构

### 新增文件

- `header/PlacementResult.h` - 布局结果文件格式定义
- `src/PlacementResult.cpp` - 布局结果文件处理实现
- `src/main_placement_only.cpp` - 独立的布局程序
- `src/main_routing_only.cpp` - 独立的布线程序
- `run_separated_flow.sh` - 分离流程演示脚本

### 修改的文件

- `header/Placer.h` - 添加布局结果保存功能
- `src/Placer.cpp` - 实现布局结果保存
- `header/Router.h` - 添加从布局文件加载功能
- `src/Router.cpp` - 实现布局文件加载
- `CMakeLists.txt` - 添加新的可执行文件

## 编译

```bash
cd MAKE/PLACE_ROUTE/csyn_fp
mkdir build
cd build
cmake ..
make -j4
```

编译后会生成以下可执行文件：
- `placement_only` - 独立布局程序
- `routing_only` - 独立布线程序
- `placement` - 原始集成程序
- `width_only` - 宽度优化程序

## 使用方法

### 1. 布局阶段

```bash
./placement_only -i <input_cdl> -d <design_rules> -o <output_dir>
```

参数说明：
- `-i`: 输入CDL文件路径
- `-d`: 设计规则文件路径
- `-o`: 输出目录路径

布局结果将保存在 `<output_dir>/placement_results/` 目录下，文件格式为：
`<cell_name>_w<width>_sol<solution_index>.placement`

### 2. 布线阶段

```bash
./routing_only -i <input_cdl> -d <design_rules> -p <placement_file> -o <output_dir>
```

参数说明：
- `-i`: 输入CDL文件路径
- `-d`: 设计规则文件路径
- `-p`: 布局结果文件路径（必需）
- `-o`: 输出目录路径

### 3. 使用演示脚本

```bash
./run_separated_flow.sh
```

该脚本会自动执行完整的分离流程。

## 布局结果文件格式

布局结果文件（.placement）采用文本格式，包含以下信息：

```
# Placement Result File
# Cell: <cell_name>
# Width: <width>
# Solution Index: <solution_index>
# Format: [Column] NMOS(name,left,gate,right,nfin) PMOS(name,left,gate,right,nfin)
BEGIN_PLACEMENT
[Column 1]
NMOS: <name>(<nfin>) [<left> <gate> <right>], PMOS: <name>(<nfin>) [<left> <gate> <right>]
...
END_PLACEMENT
```

## 示例

### 完整流程示例

```bash
# 1. 执行布局
./placement_only -i DATA/input/asap7sc7p5t_split.sp -d DATA/input/placement_file.style -o output

# 2. 查看生成的布局结果
ls output/placement_results/

# 3. 对特定布局结果执行布线
./routing_only -i DATA/input/asap7sc7p5t_split.sp -d DATA/input/placement_file.style -p output/placement_results/AND2x2_ASAP7_6t_R_w6_sol0.placement -o output/routing_results
```

### 批量处理示例

```bash
# 对所有布局结果执行布线
for placement_file in output/placement_results/*.placement; do
    echo "Processing $placement_file"
    ./routing_only -i DATA/input/asap7sc7p5t_split.sp -d DATA/input/placement_file.style -p "$placement_file" -o output/routing_results
done
```

## 注意事项

1. **文件依赖**：布线阶段需要对应的CDL文件来获取单元信息
2. **路径设置**：确保所有文件路径正确，特别是设计规则文件
3. **内存管理**：大型设计可能需要更多内存
4. **错误处理**：检查程序输出中的错误信息

## 技术细节

### PlacementResult类

`PlacementResult`类负责布局结果的序列化和反序列化：

- `saveToFile()`: 将PlaceGrid对象保存为.placement文件
- `loadFromFile()`: 从.placement文件加载PlaceGrid对象
- 支持解析和生成标准化的布局结果格式

### Router类扩展

Router类新增了从布局文件加载的功能：

- 新增构造函数：`Router(Cell c, const std::string& placementFilePath)`
- `loadPlacementFromFile()`: 从文件加载布局结果
- 保持与原始Router类的兼容性

### Placer类扩展

Placer类新增了布局结果保存功能：

- `savePlacementResults()`: 保存所有布局结果
- `savePlacementResult()`: 保存单个布局结果
- 自动创建布局结果目录结构

## 故障排除

### 常见问题

1. **编译错误**：确保所有依赖文件存在，检查CMake配置
2. **文件未找到**：检查文件路径和权限
3. **布局结果格式错误**：确保.placement文件格式正确
4. **内存不足**：对于大型设计，可能需要增加系统内存

### 调试技巧

1. 使用`-v`参数查看详细输出
2. 检查中间文件是否正确生成
3. 使用小规模测试用例验证功能
4. 查看程序日志和错误信息

## 未来改进

1. **并行处理**：支持多线程布局和布线
2. **增量更新**：支持布局结果的增量更新
3. **格式优化**：优化布局结果文件格式，支持压缩
4. **可视化**：添加布局结果的可视化工具
5. **性能分析**：添加详细的性能分析功能
