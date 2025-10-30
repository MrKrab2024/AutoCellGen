# 布局算法比较系统

本文档描述了如何使用新的布局算法比较系统，用于对比原始布局算法和改进的布局算法。

## 概述

布局算法比较系统提供了以下功能：

1. **NewPlacer类** - 实现改进的布局算法
2. **PlacementComparator类** - 用于比较不同布局算法的结果
3. **比较程序** - 自动运行并比较新旧算法
4. **详细报告** - 生成CSV和HTML格式的比较报告

## 新增文件

### 核心类文件
- `header/NewPlacer.h` - 改进的布局算法类定义
- `src/NewPlacer.cpp` - 改进的布局算法实现
- `header/PlacementComparator.h` - 布局结果比较器定义
- `src/PlacementComparator.cpp` - 布局结果比较器实现

### 程序文件
- `src/main_placement_comparison.cpp` - 布局算法比较主程序
- `run_placement_comparison.sh` - 比较演示脚本

## 改进的布局算法特性

### NewPlacer类的主要改进

1. **改进的配对算法** (`improvedPairing()`)
   - 优化晶体管配对策略
   - 减少配对数量，提高效率

2. **优化的状态生成** (`optimizedStateGeneration()`)
   - 改进状态生成过程
   - 添加状态剪枝和合并优化

3. **增强的解决方案搜索** (`enhancedSolutionSearch()`)
   - 改进搜索策略
   - 支持启发式搜索

4. **自适应细化** (`adaptiveRefinement()`)
   - 智能解决方案细化
   - 根据结果质量自动调整

5. **改进的成本计算** (`calculateImprovedCost()`)
   - 考虑拥塞成本
   - 考虑线长成本
   - 更全面的成本评估

### 性能分析功能

- **性能日志** - 记录算法运行时间
- **指标分析** - 分析各种布局指标
- **结果验证** - 验证解决方案有效性

## 使用方法

### 1. 编译项目

```bash
cd MAKE/PLACE_ROUTE/csyn_fp
mkdir build
cd build
cmake ..
make -j4
```

编译后会生成 `placement_comparison` 可执行文件。

### 2. 运行比较程序

```bash
./placement_comparison -i <input_cdl> -d <design_rules> -o <output_dir>
```

参数说明：
- `-i`: 输入CDL文件路径
- `-d`: 设计规则文件路径
- `-o`: 输出目录路径

### 3. 使用演示脚本

```bash
./run_placement_comparison.sh
```

## 输出结果

### 目录结构

```
output_comparison/
├── placement_comparison/
│   ├── comparison_summary.txt          # 总体比较摘要
│   ├── Cell1/                          # 每个单元的比较结果
│   │   ├── placement_comparison.csv    # CSV格式比较报告
│   │   ├── placement_comparison.html   # HTML格式比较报告
│   │   ├── Cell1.txt                   # 原始算法结果
│   │   └── Cell1_new.txt               # 改进算法结果
│   └── Cell2/
│       └── ...
```

### 比较指标

1. **布局质量指标**
   - 单元宽度 (Cell Width)
   - 成本 (Cost)
   - 半周长线长 (HPWL)
   - 网格使用率 (Grid Usage)

2. **性能指标**
   - 运行时间 (Runtime)
   - 内存使用
   - 算法效率

3. **改进效果**
   - 宽度改进百分比
   - 时间改进百分比
   - 质量提升程度

## 比较报告格式

### CSV报告

包含所有布局结果的详细数据，便于数据分析：

```csv
算法名称,单元宽度,成本,HPWL,最大H网格,最大V网格,最大H列,最大V列,最大H行,最大V行,运行时间(ms)
Original,6,1.234,0.567,0.8,0.9,0.7,0.8,0.6,0.7,150
Improved,5,1.123,0.456,0.7,0.8,0.6,0.7,0.5,0.6,120
```

### HTML报告

提供可视化的比较结果，包括：

- 最佳结果高亮显示
- 详细的指标对比表格
- 性能统计图表
- 改进效果分析

## 算法改进策略

### 1. 配对优化

```cpp
void NewPlacer::improvedPairing() {
    // 使用改进的配对策略
    // 合并相似的配对
    // 优化配对顺序
}
```

### 2. 状态生成优化

```cpp
void NewPlacer::optimizedStateGeneration() {
    // 状态剪枝
    // 状态合并
    // 减少冗余状态
}
```

### 3. 搜索策略改进

```cpp
void NewPlacer::enhancedSolutionSearch() {
    // 启发式搜索
    // 并行搜索
    // 智能剪枝
}
```

### 4. 成本函数改进

```cpp
double NewPlacer::calculateImprovedCost(const PlaceGrid& solution) {
    double baseCost = solution.cost;
    double congestionCost = solution.max_h_grid + solution.max_v_grid;
    double wireLengthCost = solution.nor_hpwl;
    
    return baseCost + 0.1 * congestionCost + 0.2 * wireLengthCost;
}
```

## 扩展功能

### 1. 添加新的改进算法

```cpp
class NewPlacer {
    void runCustomAlgorithm() {
        // 实现自定义算法
    }
};
```

### 2. 自定义比较指标

```cpp
struct CustomMetrics {
    double customMetric1;
    double customMetric2;
    // 添加自定义指标
};
```

### 3. 批量比较

```cpp
void batchComparison(const std::vector<std::string>& cellNames) {
    for (const auto& cellName : cellNames) {
        // 对每个单元进行比较
    }
}
```

## 性能优化建议

1. **并行处理** - 使用多线程并行运行不同算法
2. **内存管理** - 优化内存使用，避免内存泄漏
3. **算法调优** - 根据实际数据调整算法参数
4. **缓存优化** - 缓存中间结果，减少重复计算

## 故障排除

### 常见问题

1. **编译错误** - 检查依赖文件和编译器版本
2. **运行时错误** - 检查输入文件格式和路径
3. **内存不足** - 增加系统内存或优化算法
4. **结果异常** - 检查算法实现和参数设置

### 调试技巧

1. 使用详细日志输出
2. 检查中间结果文件
3. 对比单个单元的结果
4. 分析性能瓶颈

## 未来改进方向

1. **机器学习集成** - 使用ML优化布局参数
2. **多目标优化** - 同时优化多个目标函数
3. **实时比较** - 支持实时算法比较
4. **可视化工具** - 添加图形化比较界面
5. **自动化测试** - 集成到CI/CD流程

## 总结

布局算法比较系统提供了一个完整的框架来开发和测试改进的布局算法。通过详细的比较分析，可以量化算法改进的效果，为进一步的优化提供指导。
