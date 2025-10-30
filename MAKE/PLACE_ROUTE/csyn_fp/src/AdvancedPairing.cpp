////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (c) 2024, Kyeonghyeon Baek, Sehyeon Chung, Handong Cho, 
// Hyunbae Seo, Kyu-myung Choi, and Taewhan Kim
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////

#include "../header/AdvancedPairing.h"
#include <algorithm>
#include <iostream>
#include <cmath>

void AdvancedPairer::advancedPairing() {
    std::cout << "开始高级配对算法..." << std::endl;
    
    // 1. 合并晶体管（多finger合并）
    mergeTransistors();
    
    // 2. 预匹配传输门和反相器
    preMatchSpecialStructures();
    
    // 3. 提取离散晶体管
    extractDiscreteTransistors();
    
    // 4. 对晶体管进行分组
    groupTransistors();
    
    // 5. 尝试迭代匹配
    bool iterativeSuccess = iterativeMatching();
    
    // 6. 如果迭代匹配失败，使用暴力匹配
    if (!iterativeSuccess) {
        std::cout << "迭代匹配失败，切换到暴力匹配..." << std::endl;
        bruteForceMatching();
    }
    
    // 7. 优化dummy mos插入
    optimizeDummyMos();
    
    std::cout << "高级配对算法完成！" << std::endl;
    std::cout << "匹配的配对组数量: " << matchedPairGroups.size() << std::endl;
    std::cout << "离散晶体管数量: " << discreteTransistors.size() << std::endl;
}

void AdvancedPairer::mergeTransistors() {
    std::cout << "开始晶体管合并..." << std::endl;
    
    // 使用map来存储相同属性的晶体管
    std::unordered_map<std::string, std::vector<Transistor>> mergeMap;
    
    for (const auto& trans : cell.trans) {
        // 创建合并键：source + drain + gate + type + nfin
        std::string mergeKey = trans.source + "|" + trans.drain + "|" + 
                              trans.gate + "|" + 
                              (trans.type == transtype::PMOS ? "PMOS" : "NMOS") + "|" +
                              std::to_string(trans.nfin);
        
        mergeMap[mergeKey].push_back(trans);
    }
    
    // 合并相同键的晶体管
    for (auto& pair : mergeMap) {
        auto& transList = pair.second;
        if (transList.empty()) continue;
        
        // 选择第一个晶体管作为基础
        Transistor mergedTrans = transList[0];
        
        // 计算总的finger数量
        int totalFinger = 0;
        for (const auto& trans : transList) {
            totalFinger += trans.nfin;
        }
        mergedTrans.nfin = totalFinger;
        
        // 更新名称以反映合并
        mergedTrans.name = "MERGED_" + std::to_string(mergedTransistors.size());
        
        mergedTransistors.push_back(mergedTrans);
    }
    
    std::cout << "原始晶体管数量: " << cell.trans.size() << std::endl;
    std::cout << "合并后晶体管数量: " << mergedTransistors.size() << std::endl;
}

void AdvancedPairer::preMatchSpecialStructures() {
    std::cout << "开始预匹配特殊结构..." << std::endl;
    
    std::vector<int> pmosIndices, nmosIndices;
    
    // 分离PMOS和NMOS的索引
    for (size_t i = 0; i < mergedTransistors.size(); i++) {
        if (mergedTransistors[i].type == transtype::PMOS) {
            pmosIndices.push_back(static_cast<int>(i));
        } else {
            nmosIndices.push_back(static_cast<int>(i));
        }
    }
    
    // 标记已匹配的晶体管
    isMatched.resize(mergedTransistors.size(), false);
    
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
            if (isTransmissionGate(pmos, nmos)) {
                Pair pair;
                pair.addTransistor(pmos);
                pair.addTransistor(nmos);
                pair.is_xc_pair = true;
                preMatchedPairs.push_back(pair);
                
                isMatched[pmosIdx] = true;
                isMatched[nmosIdx] = true;
                break;
            }
            // 检查是否为反相器
            else if (isInverter(pmos, nmos)) {
                Pair pair;
                pair.addTransistor(pmos);
                pair.addTransistor(nmos);
                preMatchedPairs.push_back(pair);
                
                isMatched[pmosIdx] = true;
                isMatched[nmosIdx] = true;
                break;
            }
        }
    }
    
    std::cout << "预匹配的特殊结构数量: " << preMatchedPairs.size() << std::endl;
}

bool AdvancedPairer::isTransmissionGate(const Transistor& pmos, const Transistor& nmos) {
    // 传输门特征：
    // 1. PMOS和NMOS的gate不同（互补信号）
    // 2. PMOS和NMOS的source或drain有共同连接
    // 3. 不是简单的反相器连接
    
    if (pmos.gate == nmos.gate) return false; // 不是互补信号
    
    bool hasCommonConnection = (pmos.source == nmos.source || 
                               pmos.source == nmos.drain ||
                               pmos.drain == nmos.source || 
                               pmos.drain == nmos.drain);
    
    return hasCommonConnection;
}

bool AdvancedPairer::isInverter(const Transistor& pmos, const Transistor& nmos) {
    // 反相器特征：
    // 1. PMOS和NMOS的gate相同
    // 2. PMOS的source连接VDD，NMOS的source连接VSS
    // 3. PMOS的drain和NMOS的drain连接（输出）
    
    if (pmos.gate != nmos.gate) return false; // gate必须相同
    
    // 检查输出连接
    bool outputConnected = (pmos.drain == nmos.drain);
    
    return outputConnected;
}

void AdvancedPairer::extractDiscreteTransistors() {
    std::cout << "提取离散晶体管..." << std::endl;
    
    for (size_t i = 0; i < mergedTransistors.size(); i++) {
        if (!isMatched[i]) {
            discreteTransistors.push_back(mergedTransistors[i]);
        }
    }
    
    std::cout << "离散晶体管数量: " << discreteTransistors.size() << std::endl;
}

void AdvancedPairer::groupTransistors() {
    std::cout << "开始晶体管分组..." << std::endl;
    
    // 使用并查集来分组晶体管
    std::vector<int> parent(discreteTransistors.size());
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
    for (size_t i = 0; i < discreteTransistors.size(); i++) {
        for (size_t j = i + 1; j < discreteTransistors.size(); j++) {
            const auto& t1 = discreteTransistors[i];
            const auto& t2 = discreteTransistors[j];
            
            // 检查是否有共同的source或drain连接
            bool canConnect = (t1.source == t2.source || t1.source == t2.drain ||
                              t1.drain == t2.source || t1.drain == t2.drain);
            
            if (canConnect) {
                unite(static_cast<int>(i), static_cast<int>(j));
            }
        }
    }
    
    // 构建分组
    std::unordered_map<int, std::vector<int>> groupMap;
    for (size_t i = 0; i < discreteTransistors.size(); i++) {
        int root = find(static_cast<int>(i));
        groupMap[root].push_back(static_cast<int>(i));
    }
    
    // 转换为transistorGroups
    for (const auto& pair : groupMap) {
        transistorGroups.push_back(pair.second);
    }
    
    std::cout << "晶体管分组数量: " << transistorGroups.size() << std::endl;
}

bool AdvancedPairer::iterativeMatching() {
    std::cout << "开始迭代匹配..." << std::endl;
    
    // 创建未匹配晶体管的临时列表
    std::vector<Transistor> unmatchedTransistors;
    
    for (const auto& group : transistorGroups) {
        if (group.empty()) continue;
        
        // 分离PMOS和NMOS
        std::vector<Transistor> pmosGroup, nmosGroup;
        for (int idx : group) {
            const auto& trans = discreteTransistors[idx];
            if (trans.type == transtype::PMOS) {
                pmosGroup.push_back(trans);
            } else {
                nmosGroup.push_back(trans);
            }
        }
        
        if (pmosGroup.empty() || nmosGroup.empty()) {
            // 如果只有一种类型的晶体管，添加到未匹配列表
            for (const auto& trans : pmosGroup) {
                unmatchedTransistors.push_back(trans);
            }
            for (const auto& trans : nmosGroup) {
                unmatchedTransistors.push_back(trans);
            }
            continue;
        }
        
        // 创建连接组
        std::vector<ConnectGroup> pmosConnectGroups, nmosConnectGroups;
        
        // 为每个晶体管创建初始连接组
        for (const auto& trans : pmosGroup) {
            pmosConnectGroups.emplace_back(trans);
        }
        for (const auto& trans : nmosGroup) {
            nmosConnectGroups.emplace_back(trans);
        }
        
        // 迭代合并连接组
        mergeConnectGroups(pmosConnectGroups);
        mergeConnectGroups(nmosConnectGroups);
        
        // 尝试匹配连接组
        bool matched = false;
        for (auto& pmosConnectGroup : pmosConnectGroups) {
            for (auto& nmosConnectGroup : nmosConnectGroups) {
                if (matchConnectGroups(pmosConnectGroup, nmosConnectGroup)) {
                    PairGroup pairGroup = generatePairGroup(pmosConnectGroup, nmosConnectGroup);
                    matchedPairGroups.push_back(pairGroup);
                    matched = true;
                    break;
                }
            }
            if (matched) break;
        }
        
        if (!matched) {
            // 如果无法匹配，将晶体管添加到未匹配列表
            for (const auto& trans : pmosGroup) {
                unmatchedTransistors.push_back(trans);
            }
            for (const auto& trans : nmosGroup) {
                unmatchedTransistors.push_back(trans);
            }
        }
    }
    
    // 更新离散晶体管列表为未匹配的晶体管
    discreteTransistors = unmatchedTransistors;
    
    std::cout << "迭代匹配完成，未匹配晶体管数量: " << discreteTransistors.size() << std::endl;
    
    // 检查是否所有晶体管都被匹配
    int totalMatchedTransistors = 0;
    for (const auto& pairGroup : matchedPairGroups) {
        for (const auto& pair : pairGroup.pairList) {
            totalMatchedTransistors += pair.pmos.size() + pair.nmos.size();
        }
    }
    
    // 如果还有未匹配的晶体管，返回false表示需要暴力匹配
    bool allMatched = (discreteTransistors.size() == 0);
    std::cout << "匹配的晶体管数量: " << totalMatchedTransistors << std::endl;
    std::cout << "迭代匹配" << (allMatched ? "成功" : "部分成功，需要暴力匹配") << std::endl;
    
    return allMatched;
}

void AdvancedPairer::mergeConnectGroups(std::vector<ConnectGroup>& groups) {
    while (groups.size() > 1) {
        bool hasSeriesMerge = false;
        bool hasParallelMerge = false;
        
        // 第一步：筛选并合并所有串联组
        std::vector<bool> mergedInSeries(groups.size(), false);
        for (size_t i = 0; i < groups.size(); i++) {
            if (mergedInSeries[i]) continue;
            
            for (size_t j = i + 1; j < groups.size(); j++) {
                if (mergedInSeries[j]) continue;
                
                if (groups[i].canMergeWith(groups[j])) {
                    ConnectWay way = determineConnectionWay(groups[i], groups[j]);
                    if (way == ConnectWay::SERIES) {
                        // 合并串联组
                        groups[i].merge(groups[j], way);
                        groups.erase(groups.begin() + j);
                        mergedInSeries[i] = true;
                        hasSeriesMerge = true;
                        
                        // 更新mergedInSeries数组，因为索引发生了变化
                        mergedInSeries.erase(mergedInSeries.begin() + j);
                        break;
                    }
                }
            }
        }
        
        // 第二步：筛选并合并所有并联组（不包括第一步中合并的组）
        std::vector<bool> mergedInParallel(groups.size(), false);
        for (size_t i = 0; i < groups.size(); i++) {
            if (mergedInParallel[i]) continue;
            
            for (size_t j = i + 1; j < groups.size(); j++) {
                if (mergedInParallel[j]) continue;
                
                if (groups[i].canMergeWith(groups[j])) {
                    ConnectWay way = determineConnectionWay(groups[i], groups[j]);
                    if (way == ConnectWay::PARALLEL) {
                        // 合并并联组
                        groups[i].merge(groups[j], way);
                        groups.erase(groups.begin() + j);
                        mergedInParallel[i] = true;
                        hasParallelMerge = true;
                        
                        // 更新mergedInParallel数组，因为索引发生了变化
                        mergedInParallel.erase(mergedInParallel.begin() + j);
                        break;
                    }
                }
            }
        }
        
        // 如果既没有串联也没有并联合并，则强制合并所有组
        if (!hasSeriesMerge && !hasParallelMerge) {
            if (groups.size() > 1) {
                // 将所有组合并成一个组
                for (size_t i = 1; i < groups.size(); i++) {
                    // 使用串联方式合并（因为这是默认的连接方式）
                    groups[0].merge(groups[i], ConnectWay::SERIES);
                }
                groups.erase(groups.begin() + 1, groups.end());
            }
            break; // 强制合并后退出
        }
        
        // 如果只剩一个组，也退出
        if (groups.size() == 1) {
            break;
        }
    }
}

bool AdvancedPairer::matchConnectGroups(const ConnectGroup& pmosGroup, const ConnectGroup& nmosGroup) {
    // 检查level是否一致
    if (pmosGroup.level != nmosGroup.level) {
        return false;
    }
    
    // 检查晶体管数量是否匹配
    int pmosCount = pmosGroup.getTransistorCount();
    int nmosCount = nmosGroup.getTransistorCount();
    if (std::abs(pmosCount - nmosCount) > 1) {
        return false; // 数量差异太大，不匹配
    }
    
    // 检查gate相似性
    auto pmosGateStats = pmosGroup.getGateStats();
    auto nmosGateStats = nmosGroup.getGateStats();
    
    // 计算gate匹配度
    int totalGates = 0;
    int matchedGates = 0;
    
    for (const auto& pmosGate : pmosGateStats) {
        totalGates += pmosGate.second;
        auto it = nmosGateStats.find(pmosGate.first);
        if (it != nmosGateStats.end()) {
            matchedGates += std::min(pmosGate.second, it->second);
        }
    }
    
    // 如果gate匹配度低于50%，认为不匹配
    double matchRatio = (totalGates > 0) ? (double)matchedGates / totalGates : 0.0;
    if (matchRatio < 0.5) {
        return false;
    }
    
    // 检查连接方式是否兼容
    if (pmosGroup.connectWay != nmosGroup.connectWay) {
        // 如果连接方式不同，需要更严格的匹配条件
        return matchRatio >= 0.8;
    }
    
    return true;
}

PairGroup AdvancedPairer::generatePairGroup(const ConnectGroup& pmosGroup, const ConnectGroup& nmosGroup) {
    PairGroup pairGroup;
    
    // 创建配对
    Pair pair;
    for (const auto& trans : pmosGroup.connectGroupList) {
        pair.addTransistor(trans);
    }
    for (const auto& trans : nmosGroup.connectGroupList) {
        pair.addTransistor(trans);
    }
    
    pairGroup.addPair(pair);
    return pairGroup;
}

void AdvancedPairer::bruteForceMatching() {
    std::cout << "开始暴力匹配..." << std::endl;
    
    // 分离PMOS和NMOS
    std::vector<Transistor> pmosList, nmosList;
    for (const auto& trans : discreteTransistors) {
        if (trans.type == transtype::PMOS) {
            pmosList.push_back(trans);
        } else {
            nmosList.push_back(trans);
        }
    }
    
    // 寻找串联部分并分组
    std::vector<std::vector<Transistor>> pmosGroups, nmosGroups;
    
    // 对PMOS进行串联分组
    pmosGroups = groupTransistorsBySeries(pmosList);
    
    // 对NMOS进行串联分组
    nmosGroups = groupTransistorsBySeries(nmosList);
    
    // 评分匹配
    std::vector<bool> pmosMatched(pmosGroups.size(), false);
    std::vector<bool> nmosMatched(nmosGroups.size(), false);
    
    while (true) {
        double bestScore = -1;
        int bestPmosIdx = -1, bestNmosIdx = -1;
        
        // 寻找最佳匹配
        for (size_t i = 0; i < pmosGroups.size(); i++) {
            if (pmosMatched[i]) continue;
            
            for (size_t j = 0; j < nmosGroups.size(); j++) {
                if (nmosMatched[j]) continue;
                
                // 创建临时连接组进行评分
                ConnectGroup pmosGroup, nmosGroup;
                for (const auto& trans : pmosGroups[i]) {
                    pmosGroup.connectGroupList.push_back(trans);
                }
                for (const auto& trans : nmosGroups[j]) {
                    nmosGroup.connectGroupList.push_back(trans);
                }
                
                double score = calculateMatchScore(pmosGroup, nmosGroup);
                if (score > bestScore) {
                    bestScore = score;
                    bestPmosIdx = static_cast<int>(i);
                    bestNmosIdx = static_cast<int>(j);
                }
            }
        }
        
        if (bestPmosIdx == -1 || bestNmosIdx == -1) break;
        
        // 创建配对
        Pair pair;
        for (const auto& trans : pmosGroups[bestPmosIdx]) {
            pair.addTransistor(trans);
        }
        for (const auto& trans : nmosGroups[bestNmosIdx]) {
            pair.addTransistor(trans);
        }
        
        PairGroup pairGroup;
        pairGroup.addPair(pair);
        matchedPairGroups.push_back(pairGroup);
        
        pmosMatched[bestPmosIdx] = true;
        nmosMatched[bestNmosIdx] = true;
    }
    
    // 将未匹配的晶体管添加到离散列表
    // 注意：这些晶体管已经在discreteTransistors中了，不需要重复添加
    // 这里只是标记哪些晶体管没有被匹配
    std::cout << "暴力匹配完成，未匹配的晶体管数量: ";
    int unmatchedCount = 0;
    for (size_t i = 0; i < pmosGroups.size(); i++) {
        if (!pmosMatched[i]) {
            unmatchedCount += pmosGroups[i].size();
        }
    }
    for (size_t i = 0; i < nmosGroups.size(); i++) {
        if (!nmosMatched[i]) {
            unmatchedCount += nmosGroups[i].size();
        }
    }
    std::cout << unmatchedCount << std::endl;
}

double AdvancedPairer::calculateMatchScore(const ConnectGroup& pmosGroup, const ConnectGroup& nmosGroup) {
    double score = 0.0;
    
    // Gate相似性评分
    auto pmosGateStats = pmosGroup.getGateStats();
    auto nmosGateStats = nmosGroup.getGateStats();
    
    for (const auto& pmosGate : pmosGateStats) {
        auto it = nmosGateStats.find(pmosGate.first);
        if (it != nmosGateStats.end()) {
            score += std::min(pmosGate.second, it->second);
        }
    }
    
    // 晶体管数量匹配评分
    int sizeDiff = std::abs(pmosGroup.getTransistorCount() - nmosGroup.getTransistorCount());
    score -= sizeDiff * 0.1;
    
    return score;
}

void AdvancedPairer::optimizeDummyMos() {
    std::cout << "开始dummy mos优化..." << std::endl;
    
    if (discreteTransistors.empty()) {
        std::cout << "没有离散晶体管需要优化" << std::endl;
        return;
    }
    
    // 统计离散晶体管的类型
    int pmosCount = 0, nmosCount = 0;
    for (const auto& trans : discreteTransistors) {
        if (trans.type == transtype::PMOS) {
            pmosCount++;
        } else {
            nmosCount++;
        }
    }
    
    std::cout << "离散晶体管统计: PMOS=" << pmosCount << " NMOS=" << nmosCount << std::endl;
    
    // 尝试将离散晶体管插入到现有的配对组中
    std::vector<Transistor> remainingDiscrete;
    
    for (const auto& trans : discreteTransistors) {
        bool inserted = false;
        
        // 尝试插入到现有的配对组中
        for (auto& pairGroup : matchedPairGroups) {
            for (auto& pair : pairGroup.pairList) {
                // 检查是否可以插入到现有配对中
                if (canInsertIntoPair(trans, pair)) {
                    pair.addTransistor(trans);
                    inserted = true;
                    std::cout << "将晶体管 " << trans.name << " 插入到现有配对中" << std::endl;
                    break;
                }
            }
            if (inserted) break;
        }
        
        if (!inserted) {
            remainingDiscrete.push_back(trans);
        }
    }
    
    // 尝试将剩余的离散晶体管配对
    std::vector<Transistor> pmosRemaining, nmosRemaining;
    for (const auto& trans : remainingDiscrete) {
        if (trans.type == transtype::PMOS) {
            pmosRemaining.push_back(trans);
        } else {
            nmosRemaining.push_back(trans);
        }
    }
    
    // 尝试配对剩余的晶体管
    while (!pmosRemaining.empty() && !nmosRemaining.empty()) {
        // 寻找最佳匹配
        double bestScore = -1;
        int bestPmosIdx = -1, bestNmosIdx = -1;
        
        for (size_t i = 0; i < pmosRemaining.size(); i++) {
            for (size_t j = 0; j < nmosRemaining.size(); j++) {
                double score = calculateTransistorMatchScore(pmosRemaining[i], nmosRemaining[j]);
                if (score > bestScore) {
                    bestScore = score;
                    bestPmosIdx = static_cast<int>(i);
                    bestNmosIdx = static_cast<int>(j);
                }
            }
        }
        
        if (bestPmosIdx >= 0 && bestNmosIdx >= 0 && bestScore > 0.3) {
            // 创建新的配对
            Pair pair;
            pair.addTransistor(pmosRemaining[bestPmosIdx]);
            pair.addTransistor(nmosRemaining[bestNmosIdx]);
            
            PairGroup pairGroup;
            pairGroup.addPair(pair);
            matchedPairGroups.push_back(pairGroup);
            
            std::cout << "创建新配对: " << pmosRemaining[bestPmosIdx].name 
                      << " + " << nmosRemaining[bestNmosIdx].name 
                      << " (评分: " << bestScore << ")" << std::endl;
            
            // 移除已配对的晶体管
            pmosRemaining.erase(pmosRemaining.begin() + bestPmosIdx);
            nmosRemaining.erase(nmosRemaining.begin() + bestNmosIdx);
        } else {
            break; // 没有合适的匹配
        }
    }
    
    // 更新离散晶体管列表
    discreteTransistors.clear();
    for (const auto& trans : pmosRemaining) {
        discreteTransistors.push_back(trans);
    }
    for (const auto& trans : nmosRemaining) {
        discreteTransistors.push_back(trans);
    }
    
    std::cout << "dummy mos优化完成，剩余离散晶体管: " << discreteTransistors.size() << std::endl;
}

bool AdvancedPairer::canConnectInSeries(const Transistor& t1, const Transistor& t2) {
    // 检查两个晶体管是否可以串联连接
    return (t1.drain == t2.source || t1.source == t2.drain ||
            t1.drain == t2.drain || t1.source == t2.source);
}

bool AdvancedPairer::canConnectInParallel(const Transistor& t1, const Transistor& t2) {
    // 检查两个晶体管是否可以并联连接
    return (t1.source == t2.source && t1.drain == t2.drain) ||
           (t1.source == t2.drain && t1.drain == t2.source);
}

ConnectWay AdvancedPairer::determineConnectionWay(const ConnectGroup& group1, const ConnectGroup& group2) {
    // 检查是否为并联连接
    // 并联：两个组的source和drain完全相同
    if (group1.source == group2.source && group1.drain == group2.drain) {
        return ConnectWay::PARALLEL;
    }
    if (group1.source == group2.drain && group1.drain == group2.source) {
        return ConnectWay::PARALLEL;
    }
    
    // 检查是否为串联连接
    // 串联：一个组的drain连接到另一个组的source
    if (group1.drain == group2.source || group1.source == group2.drain) {
        return ConnectWay::SERIES;
    }
    
    // 默认情况：根据连接点数量判断
    // 如果只有一个连接点，通常是串联
    // 如果有两个连接点，通常是并联
    int connectionPoints = 0;
    if (group1.source == group2.source || group1.source == group2.drain) connectionPoints++;
    if (group1.drain == group2.source || group1.drain == group2.drain) connectionPoints++;
    
    return (connectionPoints == 1) ? ConnectWay::SERIES : ConnectWay::PARALLEL;
}

std::vector<std::vector<Transistor>> AdvancedPairer::groupTransistorsBySeries(const std::vector<Transistor>& transistors) {
    std::vector<std::vector<Transistor>> groups;
    
    if (transistors.empty()) {
        return groups;
    }
    
    // 使用并查集来分组串联的晶体管
    std::vector<int> parent(transistors.size());
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
    
    // 根据串联关系分组
    for (size_t i = 0; i < transistors.size(); i++) {
        for (size_t j = i + 1; j < transistors.size(); j++) {
            if (canConnectInSeries(transistors[i], transistors[j])) {
                unite(static_cast<int>(i), static_cast<int>(j));
            }
        }
    }
    
    // 构建分组
    std::unordered_map<int, std::vector<Transistor>> groupMap;
    for (size_t i = 0; i < transistors.size(); i++) {
        int root = find(static_cast<int>(i));
        groupMap[root].push_back(transistors[i]);
    }
    
    // 转换为结果格式
    for (const auto& pair : groupMap) {
        groups.push_back(pair.second);
    }
    
    return groups;
}

bool AdvancedPairer::canInsertIntoPair(const Transistor& trans, const Pair& pair) {
    // 检查晶体管是否可以插入到现有配对中
    // 基本条件：类型匹配且gate相似
    
    if (trans.type == transtype::PMOS) {
        // 检查是否与PMOS晶体管有相似的gate
        for (const auto& pmos : pair.pmos) {
            if (trans.gate == pmos.gate) {
                return true;
            }
        }
    } else if (trans.type == transtype::NMOS) {
        // 检查是否与NMOS晶体管有相似的gate
        for (const auto& nmos : pair.nmos) {
            if (trans.gate == nmos.gate) {
                return true;
            }
        }
    }
    
    return false;
}

double AdvancedPairer::calculateTransistorMatchScore(const Transistor& pmos, const Transistor& nmos) {
    double score = 0.0;
    
    // Gate匹配评分
    if (pmos.gate == nmos.gate) {
        score += 1.0; // 完全匹配
    } else {
        // 部分匹配：检查gate名称的相似性
        if (pmos.gate.length() > 0 && nmos.gate.length() > 0) {
            // 简单的字符串相似性检查
            int commonChars = 0;
            int minLength = std::min(pmos.gate.length(), nmos.gate.length());
            for (int i = 0; i < minLength; i++) {
                if (pmos.gate[i] == nmos.gate[i]) {
                    commonChars++;
                }
            }
            score += (double)commonChars / std::max(pmos.gate.length(), nmos.gate.length());
        }
    }
    
    // 连接匹配评分
    bool hasCommonConnection = (pmos.source == nmos.source || 
                               pmos.source == nmos.drain ||
                               pmos.drain == nmos.source || 
                               pmos.drain == nmos.drain);
    if (hasCommonConnection) {
        score += 0.5;
    }
    
    // 尺寸匹配评分
    double sizeRatio = (double)std::min(pmos.nfin, nmos.nfin) / std::max(pmos.nfin, nmos.nfin);
    score += sizeRatio * 0.3;
    
    return score;
}
