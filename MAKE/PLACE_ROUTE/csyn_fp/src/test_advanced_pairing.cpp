////////////////////////////////////////////////////////////////////////////
// 测试高级配对算法的程序
////////////////////////////////////////////////////////////////////////////

#include "../header/AdvancedPairing.h"
#include <iostream>
#include <fstream>

// 创建测试用的Cell
Cell createTestCell() {
    Cell cell;
    cell.name = "TEST_CELL";
    
    // 添加一些测试晶体管
    // 反相器
    Transistor pmos1(transtype::PMOS);
    pmos1.name = "MP1";
    pmos1.source = "VDD";
    pmos1.gate = "A";
    pmos1.drain = "Y";
    pmos1.nfin = 2;
    cell.trans.push_back(pmos1);
    
    Transistor nmos1(transtype::NMOS);
    nmos1.name = "MN1";
    nmos1.source = "VSS";
    nmos1.gate = "A";
    nmos1.drain = "Y";
    nmos1.nfin = 1;
    cell.trans.push_back(nmos1);
    
    // 传输门
    Transistor pmos2(transtype::PMOS);
    pmos2.name = "MP2";
    pmos2.source = "IN";
    pmos2.gate = "ENB";
    pmos2.drain = "OUT";
    pmos2.nfin = 2;
    cell.trans.push_back(pmos2);
    
    Transistor nmos2(transtype::NMOS);
    nmos2.name = "MN2";
    nmos2.source = "IN";
    nmos2.gate = "EN";
    nmos2.drain = "OUT";
    nmos2.nfin = 1;
    cell.trans.push_back(nmos2);
    
    // 一些离散晶体管
    Transistor pmos3(transtype::PMOS);
    pmos3.name = "MP3";
    pmos3.source = "VDD";
    pmos3.gate = "B";
    pmos3.drain = "X";
    pmos3.nfin = 1;
    cell.trans.push_back(pmos3);
    
    Transistor nmos3(transtype::NMOS);
    nmos3.name = "MN3";
    nmos3.source = "VSS";
    nmos3.gate = "B";
    nmos3.drain = "X";
    nmos3.nfin = 1;
    cell.trans.push_back(nmos3);
    
    return cell;
}

int main() {
    std::cout << "=== 高级配对算法测试 ===" << std::endl;
    
    // 创建测试Cell
    Cell testCell = createTestCell();
    std::cout << "测试Cell: " << testCell.name << std::endl;
    std::cout << "晶体管数量: " << testCell.trans.size() << std::endl;
    
    // 创建高级配对器
    AdvancedPairer pairer(testCell);
    
    // 执行配对
    pairer.advancedPairing();
    
    // 输出结果
    std::cout << "\n=== 配对结果 ===" << std::endl;
    std::cout << "匹配的配对组数量: " << pairer.matchedPairGroups.size() << std::endl;
    std::cout << "预匹配的特殊结构数量: " << pairer.preMatchedPairs.size() << std::endl;
    std::cout << "离散晶体管数量: " << pairer.discreteTransistors.size() << std::endl;
    
    // 输出预匹配的特殊结构
    std::cout << "\n=== 预匹配的特殊结构 ===" << std::endl;
    for (size_t i = 0; i < pairer.preMatchedPairs.size(); i++) {
        const auto& pair = pairer.preMatchedPairs[i];
        std::cout << "配对 " << i << ": ";
        if (pair.is_xc_pair) {
            std::cout << "传输门 ";
        } else {
            std::cout << "反相器 ";
        }
        std::cout << "PMOS数量=" << pair.pmos.size() 
                  << " NMOS数量=" << pair.nmos.size() << std::endl;
    }
    
    // 输出匹配的配对组
    std::cout << "\n=== 匹配的配对组 ===" << std::endl;
    for (size_t i = 0; i < pairer.matchedPairGroups.size(); i++) {
        const auto& pairGroup = pairer.matchedPairGroups[i];
        std::cout << "配对组 " << i << ": 配对数量=" << pairGroup.getPairCount() << std::endl;
        for (size_t j = 0; j < pairGroup.pairList.size(); j++) {
            const auto& pair = pairGroup.pairList[j];
            std::cout << "  配对 " << j << ": PMOS数量=" << pair.pmos.size() 
                      << " NMOS数量=" << pair.nmos.size() << std::endl;
        }
    }
    
    // 输出离散晶体管
    std::cout << "\n=== 离散晶体管 ===" << std::endl;
    for (size_t i = 0; i < pairer.discreteTransistors.size(); i++) {
        const auto& trans = pairer.discreteTransistors[i];
        std::cout << "晶体管 " << i << ": " << trans.name 
                  << " 类型=" << (trans.type == transtype::PMOS ? "PMOS" : "NMOS")
                  << " Gate=" << trans.gate << std::endl;
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    
    return 0;
}
