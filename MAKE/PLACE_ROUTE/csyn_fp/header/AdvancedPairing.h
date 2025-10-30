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

#pragma once

#include "global.h"
#include "beol_data.h"
#include "setting.h"
#include <unordered_set>
#include <unordered_map>
#include <queue>

// 连接方式枚举
enum class ConnectWay {
    SERIES,  // 串联
    PARALLEL // 并联
};

// 连接组类
class ConnectGroup {
public:
    ConnectGroup() : level(0), connectWay(ConnectWay::SERIES) {}
    
    // 构造函数：从单个晶体管创建连接组
    ConnectGroup(const Transistor& trans) : level(0), connectWay(ConnectWay::SERIES) {
        connectGroupList.push_back(trans);
        source = trans.source;
        drain = trans.drain;
    }
    
    // 合并两个连接组
    void merge(const ConnectGroup& other, ConnectWay way) {
        connectGroupList.insert(connectGroupList.end(), 
                               other.connectGroupList.begin(), 
                               other.connectGroupList.end());
        level = std::max(level, other.level) + 1;
        connectWay = way;
        
        // 更新source和drain
        if (way == ConnectWay::SERIES) {
            // 串联：保留第一个的source和最后一个的drain
            // 这里简化处理，实际需要根据连接关系确定
            if (connectGroupList.size() == 2) {
                // 简单情况：两个晶体管串联
                source = connectGroupList[0].source;
                drain = connectGroupList[1].drain;
            }
        } else {
            // 并联：保留共同的source和drain
            source = connectGroupList[0].source;
            drain = connectGroupList[0].drain;
        }
    }
    
    // 检查是否可以与另一个组合并
    bool canMergeWith(const ConnectGroup& other) const {
        // 检查是否有共同的source或drain连接
        return (source == other.drain ||
                drain == other.source );
    }
    
    // 获取gate类型统计
    std::unordered_map<std::string, int> getGateStats() const {
        std::unordered_map<std::string, int> stats;
        for (const auto& trans : connectGroupList) {
            stats[trans.gate]++;
        }
        return stats;
    }
    
    // 获取晶体管数量
    int getTransistorCount() const {
        return static_cast<int>(connectGroupList.size());
    }
    
    // 检查是否包含特定晶体管
    bool containsTransistor(const Transistor& trans) const {
        for (const auto& t : connectGroupList) {
            if (t.name == trans.name) return true;
        }
        return false;
    }

public:
    std::vector<Transistor> connectGroupList;
    int level;
    std::string source, drain;
    ConnectWay connectWay;
};

// 配对类（用于匹配pmos和nmos）
class Pair {
public:
    Pair() : is_xc_pair(false) {}
    
    // 添加晶体管到配对
    void addTransistor(const Transistor& trans) {
        if (trans.type == transtype::PMOS) {
            pmos.push_back(trans);
        } else if (trans.type == transtype::NMOS) {
            nmos.push_back(trans);
        }
    }
    
    // 检查配对是否完整
    bool isComplete() const {
        return !pmos.empty() && !nmos.empty();
    }
    
    // 获取总晶体管数量
    int getTotalTransistorCount() const {
        return static_cast<int>(pmos.size() + nmos.size());
    }

public:
    bool is_xc_pair;
    std::vector<Transistor> pmos, nmos;
};

// 配对组类（用于固定pair list的顺序）
class PairGroup {
public:
    PairGroup() {}
    
    // 添加配对到组
    void addPair(const Pair& pair) {
        pairList.push_back(pair);
    }
    
    // 获取总配对数量
    int getPairCount() const {
        return static_cast<int>(pairList.size());
    }
    
    // 获取总晶体管数量
    int getTotalTransistorCount() const {
        int count = 0;
        for (const auto& pair : pairList) {
            count += pair.getTotalTransistorCount();
        }
        return count;
    }

public:
    std::vector<Pair> pairList;
};

// 高级配对器类
class AdvancedPairer {
public:
    AdvancedPairer(Cell& _cell) : cell(_cell) {}
    
    // 主要配对函数
    void advancedPairing();
    
    // 晶体管合并（多finger合并）
    void mergeTransistors();
    
    // 预匹配传输门和反相器
    void preMatchSpecialStructures();
    
    // 识别传输门
    bool isTransmissionGate(const Transistor& pmos, const Transistor& nmos);
    
    // 识别反相器
    bool isInverter(const Transistor& pmos, const Transistor& nmos);
    
    // 提取离散晶体管
    void extractDiscreteTransistors();
    
    // 对晶体管进行分组
    void groupTransistors();
    
    // 迭代匹配
    bool iterativeMatching();
    
    // 暴力匹配
    void bruteForceMatching();
    
    // 连接组合并
    void mergeConnectGroups(std::vector<ConnectGroup>& groups);
    
    // 匹配两个连接组
    bool matchConnectGroups(const ConnectGroup& pmosGroup, const ConnectGroup& nmosGroup);
    
    // 生成配对组
    PairGroup generatePairGroup(const ConnectGroup& pmosGroup, const ConnectGroup& nmosGroup);
    
    // 优化dummy mos插入
    void optimizeDummyMos();
    
    // 计算匹配评分
    double calculateMatchScore(const ConnectGroup& pmosGroup, const ConnectGroup& nmosGroup);
    
    // 检查两个晶体管是否可以串联
    bool canConnectInSeries(const Transistor& t1, const Transistor& t2);
    
    // 检查两个晶体管是否可以并联
    bool canConnectInParallel(const Transistor& t1, const Transistor& t2);
    
    // 确定两个连接组的连接方式
    ConnectWay determineConnectionWay(const ConnectGroup& group1, const ConnectGroup& group2);
    
    // 将晶体管按串联关系分组
    std::vector<std::vector<Transistor>> groupTransistorsBySeries(const std::vector<Transistor>& transistors);
    
    // 检查晶体管是否可以插入到现有配对中
    bool canInsertIntoPair(const Transistor& trans, const Pair& pair);
    
    // 计算两个晶体管的匹配评分
    double calculateTransistorMatchScore(const Transistor& pmos, const Transistor& nmos);

public:
    Cell& cell;
    
    // 合并后的晶体管列表
    std::vector<Transistor> mergedTransistors;
    
    // 已匹配的配对组
    std::vector<PairGroup> matchedPairGroups;
    
    // 离散晶体管（未匹配的）
    std::vector<Transistor> discreteTransistors;
    
    // 晶体管是否已匹配的标志
    std::vector<bool> isMatched;
    
    // 预匹配的特殊结构
    std::vector<Pair> preMatchedPairs;
    
    // 晶体管分组
    std::vector<std::vector<int>> transistorGroups;
};
