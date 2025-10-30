////////////////////////////////////////////////////////////////////////////
// 分析配对算法的程序（不依赖编译器，直接分析代码逻辑）
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

// 简化的Cell结构
struct Cell {
    std::string name;
    std::vector<std::string> IOnets;
    std::vector<std::string> nets;
    std::vector<Transistor> trans;
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

// 分析晶体管合并
void analyzeTransistorMerging(const Cell& cell) {
    std::cout << "\n=== 晶体管合并分析 ===" << std::endl;
    
    std::unordered_map<std::string, std::vector<Transistor>> mergeMap;
    
    for (const auto& trans : cell.trans) {
        // 创建合并键：source + drain + gate + type + nfin
        std::string mergeKey = trans.source + "|" + trans.drain + "|" + 
                              trans.gate + "|" + 
                              (trans.type == Transtype::PMOS ? "PMOS" : "NMOS") + "|" +
                              std::to_string(trans.nfin);
        
        mergeMap[mergeKey].push_back(trans);
    }
    
    std::cout << "原始晶体管数量: " << cell.trans.size() << std::endl;
    std::cout << "合并键数量: " << mergeMap.size() << std::endl;
    
    for (const auto& pair : mergeMap) {
        const auto& transList = pair.second;
        std::cout << "合并键: " << pair.first << " -> 晶体管数量: " << transList.size() << std::endl;
        for (const auto& trans : transList) {
            std::cout << "  " << trans.name << " (nfin=" << trans.nfin << ")" << std::endl;
        }
    }
}

// 分析特殊结构识别
void analyzeSpecialStructures(const Cell& cell) {
    std::cout << "\n=== 特殊结构识别分析 ===" << std::endl;
    
    std::vector<Transistor> pmosList, nmosList;
    
    // 分离PMOS和NMOS
    for (const auto& trans : cell.trans) {
        if (trans.type == Transtype::PMOS) {
            pmosList.push_back(trans);
        } else {
            nmosList.push_back(trans);
        }
    }
    
    std::cout << "PMOS数量: " << pmosList.size() << std::endl;
    std::cout << "NMOS数量: " << nmosList.size() << std::endl;
    
    // 寻找传输门
    std::cout << "\n传输门识别:" << std::endl;
    for (size_t i = 0; i < pmosList.size(); i++) {
        for (size_t j = 0; j < nmosList.size(); j++) {
            const auto& pmos = pmosList[i];
            const auto& nmos = nmosList[j];
            
            // 传输门特征：gate不同且共享连接
            if (pmos.gate != nmos.gate) {
                bool hasCommonConnection = (pmos.source == nmos.source || 
                                           pmos.source == nmos.drain ||
                                           pmos.drain == nmos.source || 
                                           pmos.drain == nmos.drain);
                
                if (hasCommonConnection) {
                    std::cout << "  传输门: " << pmos.name << " + " << nmos.name 
                              << " (gates: " << pmos.gate << " vs " << nmos.gate << ")" << std::endl;
                }
            }
        }
    }
    
    // 寻找反相器
    std::cout << "\n反相器识别:" << std::endl;
    for (size_t i = 0; i < pmosList.size(); i++) {
        for (size_t j = 0; j < nmosList.size(); j++) {
            const auto& pmos = pmosList[i];
            const auto& nmos = nmosList[j];
            
            // 反相器特征：gate相同且输出连接
            if (pmos.gate == nmos.gate) {
                bool outputConnected = (pmos.drain == nmos.drain);
                
                if (outputConnected) {
                    std::cout << "  反相器: " << pmos.name << " + " << nmos.name 
                              << " (gate: " << pmos.gate << ")" << std::endl;
                }
            }
        }
    }
}

// 分析晶体管分组
void analyzeTransistorGrouping(const Cell& cell) {
    std::cout << "\n=== 晶体管分组分析 ===" << std::endl;
    
    // 使用并查集来分组晶体管
    std::vector<int> parent(cell.trans.size());
    for (size_t i = 0; i < parent.size(); i++) {
        parent[i] = static_cast<int>(i);
    }
    
    auto find = [&](int x) -> int {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    };
    
    auto unite = [&](int x, int y) {
        int px = find(x), py = find(y);
        if (px != py) {
            parent[px] = py;
        }
    };
    
    // 根据source和drain连接关系分组
    for (size_t i = 0; i < cell.trans.size(); i++) {
        for (size_t j = i + 1; j < cell.trans.size(); j++) {
            const auto& t1 = cell.trans[i];
            const auto& t2 = cell.trans[j];
            
            // 检查是否有共同的source或drain连接
            bool canConnect = (t1.source == t2.source || t1.source == t2.drain ||
                              t1.drain == t2.source || t1.drain == t2.drain);
            
            if (canConnect) {
                unite(static_cast<int>(i), static_cast<int>(j));
                std::cout << "连接: " << t1.name << " <-> " << t2.name 
                          << " (通过 " << t1.source << "/" << t1.drain 
                          << " <-> " << t2.source << "/" << t2.drain << ")" << std::endl;
            }
        }
    }
    
    // 构建分组
    std::unordered_map<int, std::vector<int>> groupMap;
    for (size_t i = 0; i < cell.trans.size(); i++) {
        int root = find(static_cast<int>(i));
        groupMap[root].push_back(static_cast<int>(i));
    }
    
    std::cout << "\n分组结果:" << std::endl;
    for (const auto& pair : groupMap) {
        std::cout << "组 " << pair.first << ": ";
        for (int idx : pair.second) {
            std::cout << cell.trans[idx].name << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::cout << "=== 配对算法逻辑分析 ===" << std::endl;
    
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
    
    // 分析各个步骤
    analyzeTransistorMerging(testCell);
    analyzeSpecialStructures(testCell);
    analyzeTransistorGrouping(testCell);
    
    std::cout << "\n=== 分析完成 ===" << std::endl;
    
    return 0;
}
