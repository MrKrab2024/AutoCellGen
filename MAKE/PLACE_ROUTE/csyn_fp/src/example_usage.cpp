////////////////////////////////////////////////////////////////////////////
// 高级配对算法使用示例
////////////////////////////////////////////////////////////////////////////

#include "../header/Pairing.h"
#include <iostream>
#include <vector>

// 创建示例Cell
Cell createExampleCell() {
    Cell cell;
    cell.name = "EXAMPLE_CELL";
    
    // 添加输入输出引脚
    cell.IOnets = {"A", "B", "Y", "VDD", "VSS"};
    cell.nets = cell.IOnets;
    
    // 创建NAND门电路
    // PMOS晶体管（上拉网络）
    Transistor pmos1(transtype::PMOS);
    pmos1.name = "MP1";
    pmos1.source = "VDD";
    pmos1.gate = "A";
    pmos1.drain = "Y";
    pmos1.nfin = 2;
    cell.trans.push_back(pmos1);
    
    Transistor pmos2(transtype::PMOS);
    pmos2.name = "MP2";
    pmos2.source = "VDD";
    pmos2.gate = "B";
    pmos2.drain = "Y";
    pmos2.nfin = 2;
    cell.trans.push_back(pmos2);
    
    // NMOS晶体管（下拉网络）
    Transistor nmos1(transtype::NMOS);
    nmos1.name = "MN1";
    nmos1.source = "VSS";
    nmos1.gate = "A";
    nmos1.drain = "X";
    nmos1.nfin = 1;
    cell.trans.push_back(nmos1);
    
    Transistor nmos2(transtype::NMOS);
    nmos2.name = "MN2";
    nmos2.source = "X";
    nmos2.gate = "B";
    nmos2.drain = "Y";
    nmos2.nfin = 1;
    cell.trans.push_back(nmos2);
    
    return cell;
}

void printPairingResults(const Pairer& pairer) {
    std::cout << "\n=== 配对结果分析 ===" << std::endl;
    
    // 获取配对列表
    auto pairList = pairer.getPairList();
    std::cout << "总配对数量: " << pairList.size() << std::endl;
    
    // 分析配对类型
    int inverterCount = 0;
    int transmissionGateCount = 0;
    int regularPairCount = 0;
    int singleTransistorCount = 0;
    
    for (const auto& pair : pairList) {
        if (pair.is_xc_pair) {
            transmissionGateCount++;
        } else if (pair.pmos.size() == 1 && pair.nmos.size() == 1) {
            // 检查是否为反相器
            if (pair.pmos[0].gate == pair.nmos[0].gate) {
                inverterCount++;
            } else {
                regularPairCount++;
            }
        } else if (pair.pmos.empty() || pair.nmos.empty()) {
            singleTransistorCount++;
        } else {
            regularPairCount++;
        }
    }
    
    std::cout << "反相器配对: " << inverterCount << std::endl;
    std::cout << "传输门配对: " << transmissionGateCount << std::endl;
    std::cout << "常规配对: " << regularPairCount << std::endl;
    std::cout << "单晶体管: " << singleTransistorCount << std::endl;
    
    // 获取高级配对结果
    auto matchedGroups = pairer.getMatchedPairGroups();
    auto discreteTransistors = pairer.getDiscreteTransistors();
    
    std::cout << "\n匹配的配对组数量: " << matchedGroups.size() << std::endl;
    std::cout << "离散晶体管数量: " << discreteTransistors.size() << std::endl;
    
    // 输出详细配对信息
    std::cout << "\n=== 详细配对信息 ===" << std::endl;
    for (size_t i = 0; i < pairList.size(); i++) {
        const auto& pair = pairList[i];
        std::cout << "配对 " << i << ": ";
        
        if (pair.is_xc_pair) {
            std::cout << "[传输门] ";
        } else if (pair.pmos.size() == 1 && pair.nmos.size() == 1 && 
                   pair.pmos[0].gate == pair.nmos[0].gate) {
            std::cout << "[反相器] ";
        } else {
            std::cout << "[常规] ";
        }
        
        std::cout << "PMOS=" << pair.pmos.size() 
                  << " NMOS=" << pair.nmos.size();
        
        if (!pair.pmos.empty()) {
            std::cout << " (Gate: " << pair.pmos[0].gate << ")";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::cout << "=== 高级配对算法使用示例 ===" << std::endl;
    
    // 创建示例Cell
    Cell cell = createExampleCell();
    std::cout << "Cell名称: " << cell.name << std::endl;
    std::cout << "晶体管数量: " << cell.trans.size() << std::endl;
    
    // 显示原始晶体管信息
    std::cout << "\n=== 原始晶体管信息 ===" << std::endl;
    for (size_t i = 0; i < cell.trans.size(); i++) {
        const auto& trans = cell.trans[i];
        std::cout << "晶体管 " << i << ": " << trans.name 
                  << " 类型=" << (trans.type == transtype::PMOS ? "PMOS" : "NMOS")
                  << " Gate=" << trans.gate 
                  << " Source=" << trans.source 
                  << " Drain=" << trans.drain
                  << " nfin=" << trans.nfin << std::endl;
    }
    
    // 创建配对器
    Pairer pairer(cell);
    
    // 启用高级配对算法
    pairer.useAdvancedPairing = true;
    
    // 执行配对
    std::cout << "\n=== 执行配对 ===" << std::endl;
    pairer.pairing();
    
    // 输出结果
    printPairingResults(pairer);
    
    // 演示切换到传统算法
    std::cout << "\n=== 切换到传统配对算法 ===" << std::endl;
    pairer.useAdvancedPairing = false;
    pairer.pairing();
    printPairingResults(pairer);
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    
    return 0;
}
