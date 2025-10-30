////////////////////////////////////////////////////////////////////////////
// 模拟配对算法运行的程序
////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

// 简化的晶体管类型枚举
enum class Transtype { NMOS, PMOS };

// 简化的晶体管结构
struct Transistor {
    std::string name;
    std::string source, drain, gate, contact;
    Transtype type;
    double width, length;
    int nfin;
    
    Transistor() : name("null"), source("null"), drain("null"), gate("null"), 
                   contact("null"), type(Transtype::NMOS), width(0), length(0), nfin(0) {}
    
    Transistor(Transtype t) : name("null"), source("null"), drain("null"), gate("null"), 
                              contact("null"), type(t), width(0), length(0), nfin(0) {}
};

// 简化的配对结构
struct Pair {
    bool is_xc_pair;
    std::vector<Transistor> pmos, nmos;
    
    Pair() : is_xc_pair(false) {}
    
    void addTransistor(const Transistor& trans) {
        if (trans.type == Transtype::PMOS) {
            pmos.push_back(trans);
        } else if (trans.type == Transtype::NMOS) {
            nmos.push_back(trans);
        }
    }
};

// 创建测试用的Cell
Cell createTestCell() {
    Cell cell;
    cell.name = "TEST_CELL";
    
    // 添加输入输出引脚
    cell.IOnets = {"A", "B", "Y", "VDD", "VSS"};
    cell.nets = cell.IOnets;
    
    // 反相器
    Transistor pmos1(Transtype::PMOS);
    pmos1.name = "MP1";
    pmos1.source = "VDD";
    pmos1.gate = "A";
    pmos1.drain = "Y";
    pmos1.nfin = 2;
    cell.trans.push_back(pmos1);
    
    Transistor nmos1(Transtype::NMOS);
    nmos1.name = "MN1";
    nmos1.source = "VSS";
    nmos1.gate = "A";
    nmos1.drain = "Y";
    nmos1.nfin = 1;
    cell.trans.push_back(nmos1);
    
    // 传输门
    Transistor pmos2(Transtype::PMOS);
    pmos2.name = "MP2";
    pmos2.source = "IN";
    pmos2.gate = "ENB";
    pmos2.drain = "OUT";
    pmos2.nfin = 2;
    cell.trans.push_back(pmos2);
    
    Transistor nmos2(Transtype::NMOS);
    nmos2.name = "MN2";
    nmos2.source = "IN";
    nmos2.gate = "EN";
    nmos2.drain = "OUT";
    nmos2.nfin = 1;
    cell.trans.push_back(nmos2);
    
    // 一些离散晶体管
    Transistor pmos3(Transtype::PMOS);
    pmos3.name = "MP3";
    pmos3.source = "VDD";
    pmos3.gate = "B";
    pmos3.drain = "X";
    pmos3.nfin = 1;
    cell.trans.push_back(pmos3);
    
    Transistor nmos3(Transtype::NMOS);
    nmos3.name = "MN3";
    nmos3.source = "VSS";
    nmos3.gate = "B";
    nmos3.drain = "X";
    nmos3.nfin = 1;
    cell.trans.push_back(nmos3);
    
    return cell;
}

// 模拟晶体管合并
std::vector<Transistor> simulateTransistorMerging(const std::vector<Transistor>& trans) {
    std::cout << "\n=== 步骤1: 晶体管合并 ===" << std::endl;
    
    std::unordered_map<std::string, std::vector<Transistor>> mergeMap;
    
    for (const auto& t : trans) {
        // 创建合并键：source + drain + gate + type + nfin
        std::string mergeKey = t.source + "|" + t.drain + "|" + 
                              t.gate + "|" + 
                              (t.type == Transtype::PMOS ? "PMOS" : "NMOS") + "|" +
                              std::to_string(t.nfin);
        
        mergeMap[mergeKey].push_back(t);
    }
    
    std::vector<Transistor> mergedTransistors;
    
    for (const auto& pair : mergeMap) {
        const auto& transList = pair.second;
        if (transList.empty()) continue;
        
        // 选择第一个晶体管作为基础
        Transistor mergedTrans = transList[0];
        
        // 计算总的finger数量
        int totalFinger = 0;
        for (const auto& t : transList) {
            totalFinger += t.nfin;
        }
        mergedTrans.nfin = totalFinger;
        
        // 更新名称以反映合并
        mergedTrans.name = "MERGED_" + std::to_string(mergedTransistors.size());
        
        mergedTransistors.push_back(mergedTrans);
        
        std::cout << "合并: " << pair.first << " -> " << mergedTrans.name 
                  << " (nfin=" << mergedTrans.nfin << ")" << std::endl;
    }
    
    std::cout << "原始晶体管数量: " << trans.size() << std::endl;
    std::cout << "合并后晶体管数量: " << mergedTransistors.size() << std::endl;
    
    return mergedTransistors;
}

// 模拟特殊结构识别
std::vector<Pair> simulateSpecialStructureMatching(const std::vector<Transistor>& mergedTransistors) {
    std::cout << "\n=== 步骤2: 特殊结构预匹配 ===" << std::endl;
    
    std::vector<Pair> preMatchedPairs;
    std::vector<bool> isMatched(mergedTransistors.size(), false);
    
    std::vector<int> pmosIndices, nmosIndices;
    
    // 分离PMOS和NMOS的索引
    for (size_t i = 0; i < mergedTransistors.size(); i++) {
        if (mergedTransistors[i].type == Transtype::PMOS) {
            pmosIndices.push_back(static_cast<int>(i));
        } else {
            nmosIndices.push_back(static_cast<int>(i));
        }
    }
    
    // 寻找传输门和反相器
    for (size_t i = 0; i < pmosIndices.size(); i++) {
        int pmosIdx = pmosIndices[i];
        if (isMatched[pmosIdx]) continue;
        
        for (size_t j = 0; j < nmosIndices.size(); j++) {
            int nmosIdx = nmosIndices[j];
            if (isMatched[nmosIdx]) continue;
            
            const auto& pmos = mergedTransistors[pmosIdx];
            const auto& nmos = mergedTransistors[nmosIdx];
            
            // 检查是否为传输门
            if (pmos.gate != nmos.gate) {
                bool hasCommonConnection = (pmos.source == nmos.source || 
                                           pmos.source == nmos.drain ||
                                           pmos.drain == nmos.source || 
                                           pmos.drain == nmos.drain);
                
                if (hasCommonConnection) {
                    Pair pair;
                    pair.addTransistor(pmos);
                    pair.addTransistor(nmos);
                    pair.is_xc_pair = true;
                    preMatchedPairs.push_back(pair);
                    
                    isMatched[pmosIdx] = true;
                    isMatched[nmosIdx] = true;
                    
                    std::cout << "传输门匹配: " << pmos.name << " + " << nmos.name 
                              << " (gates: " << pmos.gate << " vs " << nmos.gate << ")" << std::endl;
                    break;
                }
            }
            // 检查是否为反相器
            else if (pmos.gate == nmos.gate && pmos.drain == nmos.drain) {
                Pair pair;
                pair.addTransistor(pmos);
                pair.addTransistor(nmos);
                preMatchedPairs.push_back(pair);
                
                isMatched[pmosIdx] = true;
                isMatched[nmosIdx] = true;
                
                std::cout << "反相器匹配: " << pmos.name << " + " << nmos.name 
                          << " (gate: " << pmos.gate << ")" << std::endl;
                break;
            }
        }
    }
    
    std::cout << "预匹配的特殊结构数量: " << preMatchedPairs.size() << std::endl;
    
    return preMatchedPairs;
}

// 模拟离散晶体管提取
std::vector<Transistor> simulateDiscreteTransistorExtraction(const std::vector<Transistor>& mergedTransistors, 
                                                           const std::vector<bool>& isMatched) {
    std::cout << "\n=== 步骤3: 离散晶体管提取 ===" << std::endl;
    
    std::vector<Transistor> discreteTransistors;
    
    for (size_t i = 0; i < mergedTransistors.size(); i++) {
        if (!isMatched[i]) {
            discreteTransistors.push_back(mergedTransistors[i]);
        }
    }
    
    std::cout << "离散晶体管数量: " << discreteTransistors.size() << std::endl;
    for (const auto& trans : discreteTransistors) {
        std::cout << "  离散: " << trans.name << " 类型=" 
                  << (trans.type == Transtype::PMOS ? "PMOS" : "NMOS")
                  << " Gate=" << trans.gate << std::endl;
    }
    
    return discreteTransistors;
}

int main() {
    std::cout << "=== 配对算法模拟运行 ===" << std::endl;
    
    // 创建测试Cell
    Cell testCell = createTestCell();
    std::cout << "测试Cell: " << testCell.name << std::endl;
    std::cout << "晶体管数量: " << testCell.trans.size() << std::endl;
    
    // 显示原始晶体管信息
    std::cout << "\n=== 原始晶体管信息 ===" << std::endl;
    for (size_t i = 0; i < testCell.trans.size(); i++) {
        const auto& trans = testCell.trans[i];
        std::cout << "晶体管 " << i << ": " << trans.name 
                  << " 类型=" << (trans.type == Transtype::PMOS ? "PMOS" : "NMOS")
                  << " Gate=" << trans.gate 
                  << " Source=" << trans.source 
                  << " Drain=" << trans.drain
                  << " nfin=" << trans.nfin << std::endl;
    }
    
    // 模拟算法步骤
    auto mergedTransistors = simulateTransistorMerging(testCell.trans);
    auto preMatchedPairs = simulateSpecialStructureMatching(mergedTransistors);
    
    // 创建isMatched数组（简化版本）
    std::vector<bool> isMatched(mergedTransistors.size(), false);
    for (const auto& pair : preMatchedPairs) {
        // 简化：假设所有预匹配的晶体管都被标记
        for (size_t i = 0; i < mergedTransistors.size(); i++) {
            for (const auto& pmos : pair.pmos) {
                if (mergedTransistors[i].name == pmos.name) {
                    isMatched[i] = true;
                }
            }
            for (const auto& nmos : pair.nmos) {
                if (mergedTransistors[i].name == nmos.name) {
                    isMatched[i] = true;
                }
            }
        }
    }
    
    auto discreteTransistors = simulateDiscreteTransistorExtraction(mergedTransistors, isMatched);
    
    // 输出最终结果
    std::cout << "\n=== 最终配对结果 ===" << std::endl;
    std::cout << "预匹配的特殊结构数量: " << preMatchedPairs.size() << std::endl;
    std::cout << "离散晶体管数量: " << discreteTransistors.size() << std::endl;
    
    std::cout << "\n=== 预匹配的特殊结构详情 ===" << std::endl;
    for (size_t i = 0; i < preMatchedPairs.size(); i++) {
        const auto& pair = preMatchedPairs[i];
        std::cout << "配对 " << i << ": ";
        if (pair.is_xc_pair) {
            std::cout << "[传输门] ";
        } else {
            std::cout << "[反相器] ";
        }
        std::cout << "PMOS数量=" << pair.pmos.size() 
                  << " NMOS数量=" << pair.nmos.size() << std::endl;
        
        for (const auto& pmos : pair.pmos) {
            std::cout << "  PMOS: " << pmos.name << " (gate=" << pmos.gate << ")" << std::endl;
        }
        for (const auto& nmos : pair.nmos) {
            std::cout << "  NMOS: " << nmos.name << " (gate=" << nmos.gate << ")" << std::endl;
        }
    }
    
    std::cout << "\n=== 模拟完成 ===" << std::endl;
    
    return 0;
}
