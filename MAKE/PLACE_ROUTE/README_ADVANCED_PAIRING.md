# 高级晶体管配对算法

## 概述

本文档描述了重构后的晶体管配对算法，该算法基于您提供的详细算法描述实现。新算法提供了更智能的晶体管匹配和布局优化。

## 算法特点

### 1. 多Finger晶体管合并
- 自动识别具有相同source、drain、gate和nfin的晶体管
- 将相同属性的晶体管合并为多finger晶体管
- 减少晶体管数量，提高匹配效率

### 2. 特殊结构预匹配
- **传输门识别**：识别PMOS和NMOS具有互补gate信号且共享source/drain的结构
- **反相器识别**：识别具有相同gate且输出连接的PMOS/NMOS对
- 提前匹配这些特殊结构，避免后续复杂匹配

### 3. 迭代匹配算法
- 将晶体管按连接关系分组
- 创建连接组（ConnectGroup）并迭代合并
- 支持串联和并联连接识别
- 基于level和gate相似性进行匹配

### 4. 暴力匹配算法
- 当迭代匹配失败时自动切换到暴力匹配
- 基于评分系统选择最佳匹配组合
- 确保所有晶体管都能得到合理匹配

### 5. Dummy MOS优化
- 通过插入dummy MOS减少离散晶体管数量
- 优化布局密度和布线长度

## 数据结构

### ConnectGroup（连接组）
```cpp
class ConnectGroup {
    std::vector<Transistor> connectGroupList;  // 包含的晶体管列表
    int level;                                 // 合并层级
    std::string source, drain;                 // 连接端点
    ConnectWay connectWay;                     // 连接方式（串联/并联）
};
```

### PairGroup（配对组）
```cpp
class PairGroup {
    std::vector<Pair> pairList;  // 配对列表
};
```

### Pair（配对）
```cpp
class Pair {
    bool is_xc_pair;                    // 是否为传输门配对
    std::vector<Transistor> pmos, nmos; // PMOS和NMOS晶体管
};
```

## 算法流程

1. **晶体管合并**：将相同属性的晶体管合并为多finger晶体管
2. **特殊结构预匹配**：识别并匹配传输门和反相器
3. **离散晶体管提取**：提取未匹配的晶体管
4. **晶体管分组**：根据连接关系对晶体管进行分组
5. **迭代匹配**：尝试使用迭代算法匹配连接组
6. **暴力匹配**：如果迭代匹配失败，使用暴力匹配
7. **Dummy MOS优化**：优化离散晶体管的布局

## 使用方法

### 基本使用
```cpp
#include "AdvancedPairing.h"

// 创建Cell对象
Cell cell;
// ... 填充晶体管数据 ...

// 创建高级配对器
AdvancedPairer pairer(cell);

// 执行配对
pairer.advancedPairing();

// 获取结果
auto matchedGroups = pairer.getMatchedPairGroups();
auto discreteTransistors = pairer.getDiscreteTransistors();
```

### 集成到现有Pairer类
```cpp
#include "Pairing.h"

// 创建Pairer对象
Pairer pairer(cell);

// 启用高级配对算法（默认启用）
pairer.useAdvancedPairing = true;

// 执行配对
pairer.pairing();

// 获取结果
auto pairList = pairer.getPairList();
auto matchedGroups = pairer.getMatchedPairGroups();
```

## 编译和测试

### 编译测试程序
```bash
cd MAKE/PLACE_ROUTE/csyn_fp
make -f Makefile_test
```

### 运行测试
```bash
make -f Makefile_test test
```

## 算法优势

1. **更智能的匹配**：基于连接关系和电路结构的智能匹配
2. **更好的布局优化**：考虑串联/并联连接，优化布局密度
3. **更高的匹配率**：通过多种匹配策略确保高匹配率
4. **更好的扩展性**：模块化设计，易于扩展和修改
5. **向后兼容**：保持与现有代码的兼容性

## 配置选项

- `useAdvancedPairing`：控制是否使用高级配对算法（默认：true）
- 可以通过修改`AdvancedPairer`类的参数来调整算法行为

## 注意事项

1. 算法假设晶体管具有正确的连接关系
2. 特殊结构识别基于简单的连接模式，可能需要根据实际电路调整
3. 迭代匹配的复杂度较高，对于大型电路可能需要优化
4. Dummy MOS插入逻辑需要根据具体布局要求实现

## 未来改进

1. 更复杂的特殊结构识别
2. 更智能的评分系统
3. 并行化处理支持
4. 更精细的布局优化
5. 支持更多连接模式
