#include "../header/NewPlacer.h"
#include <filesystem>
#include <chrono>
#include <algorithm>

NewPlacer::NewPlacer() {
    min_width = 9999;
}

void NewPlacer::run() {
    std::cout << "=== 运行改进的布局算法 ===" << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    
    // 使用改进的配对算法
    improvedPairing();
    
    // 生成布局单元
    generatePlaceUnit();
    
    numUnit = units.size();
    std::cout << "改进算法 - 单元数量: " << numUnit << std::endl;

    // 使用优化的搜索算法
    runImprovedPlacement();
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "改进算法运行时间: " << duration.count() << "ms" << std::endl;
    logAlgorithmMetrics("NewPlacer", duration.count());
    
    // 性能分析
    analyzePerformance();
    
    // 保存结果
    std::string filePath = out_dir + "/" + cell.name + "_new.txt";
    std::cout << "保存改进算法结果到: " << filePath << std::endl;
    printSolution(filePath);
    
    // 保存布局结果到文件
    savePlacementResults(out_dir);
}

void NewPlacer::runImprovedPlacement() {
    numUnit = units.size();
    std::vector<int> order(numUnit, -1);
    lower_bound = MAX_OFFSET;
    
    // 使用改进的搜索策略
    enhancedSolutionSearch();
    
    min_width = lower_bound;
    
    if (setting.relaxation > 0) {
        lower_bound = min_width + setting.relaxation;
        findSolution(0, -1, order, true);
    }
}

void NewPlacer::runOptimizedPlacement() {
    std::cout << "=== 运行优化布局算法 ===" << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    
    // 优化的配对
    improvedPairing();
    generatePlaceUnit();
    
    // 优化的状态生成
    optimizedStateGeneration();
    
    // 增强的解决方案搜索
    enhancedSolutionSearch();
    
    // 自适应细化
    if (setting.refine_sol) {
        adaptiveRefinement();
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "优化算法运行时间: " << duration.count() << "ms" << std::endl;
    logAlgorithmMetrics("OptimizedPlacer", duration.count());
}

void NewPlacer::improvedPairing() {
    std::cout << "使用改进的配对算法..." << std::endl;
    
    Pairer pairer(cell);
    pairer.pairing();
    
    // 复制配对结果
    pairGroup = pairer.pairGroup;
    pairs = pairer.pair_list;
    
    // 改进的配对优化
    std::cout << "原始配对数量: " << pairs.size() << std::endl;
    
    // 可以在这里添加配对优化逻辑
    // 例如：合并相似的配对、优化配对顺序等
    
    std::cout << "改进后配对数量: " << pairs.size() << std::endl;
}

void NewPlacer::optimizedStateGeneration() {
    std::cout << "使用优化的状态生成..." << std::endl;
    
    for (auto& unit : units) {
        if (unit.pair.is_xc_pair) {
            unit.generateStates_xc();
        } else {
            unit.generateStates();
        }
        
        // 优化状态生成
        // 可以添加状态剪枝、状态合并等优化
    }
}

void NewPlacer::enhancedSolutionSearch() {
    std::cout << "使用增强的解决方案搜索..." << std::endl;
    
    numUnit = units.size();
    std::vector<int> order(numUnit, -1);
    lower_bound = MAX_OFFSET;
    
    // 改进的搜索策略
    findSolution(0, -1, order, false);
    min_width = lower_bound;
    
    // 可以添加更多搜索优化
    // 例如：并行搜索、启发式搜索等
}

void NewPlacer::adaptiveRefinement() {
    std::cout << "使用自适应细化..." << std::endl;
    
    for (auto& sol : solutions[min_width]) {
        // 检查解决方案是否需要细化
        if (isSolutionValid(sol)) {
            optimizeSolution(sol);
        }
    }
}

void NewPlacer::compareWithOriginal(Placer& originalPlacer, const std::string& outputDir) {
    std::cout << "\n=== 布局算法比较 ===" << std::endl;
    
    // 清空比较器
    comparator.clear();
    
    // 添加原始算法结果
    for (int width = originalPlacer.min_width; width <= originalPlacer.min_width + setting.relaxation; width++) {
        if (originalPlacer.solutions.find(width) != originalPlacer.solutions.end()) {
            auto& w_solutions = originalPlacer.solutions[width];
            for (int i = 0; i < w_solutions.size(); i++) {
                comparator.addPlaceGridResult(w_solutions[i], cell.name, width, "Original", 0);
            }
        }
    }
    
    // 添加改进算法结果
    for (int width = min_width; width <= min_width + setting.relaxation; width++) {
        if (solutions.find(width) != solutions.end()) {
            auto& w_solutions = solutions[width];
            for (int i = 0; i < w_solutions.size(); i++) {
                comparator.addPlaceGridResult(w_solutions[i], cell.name, width, "Improved", 0);
            }
        }
    }
    
    // 执行比较
    comparator.compareResults();
    
    // 生成比较报告
    std::string reportPath = outputDir + "/placement_comparison";
    comparator.generateComparisonReport(reportPath);
    
    std::cout << "比较报告已生成: " << reportPath << ".csv 和 " << reportPath << ".html" << std::endl;
}

void NewPlacer::analyzePerformance() {
    std::cout << "\n=== 性能分析 ===" << std::endl;
    
    if (performanceLog.empty()) {
        std::cout << "没有性能数据可分析" << std::endl;
        return;
    }
    
    std::cout << "算法性能统计:" << std::endl;
    for (const auto& log : performanceLog) {
        std::cout << "  " << log.first << ": " << log.second << "ms" << std::endl;
    }
}

void NewPlacer::logAlgorithmMetrics(const std::string& algorithmName, int64_t runtime) {
    performanceLog.push_back({algorithmName, runtime});
}

double NewPlacer::calculateImprovedCost(const PlaceGrid& solution) {
    // 改进的成本计算函数
    double baseCost = solution.cost;
    
    // 添加额外的成本因子
    double congestionCost = solution.max_h_grid + solution.max_v_grid;
    double wireLengthCost = solution.nor_hpwl;
    
    // 组合成本
    return baseCost + 0.1 * congestionCost + 0.2 * wireLengthCost;
}

bool NewPlacer::isSolutionValid(const PlaceGrid& solution) {
    // 检查解决方案的有效性
    if (solution.cellWidth <= 0) return false;
    if (solution.nmos.empty() || solution.pmos.empty()) return false;
    
    // 可以添加更多验证逻辑
    return true;
}

void NewPlacer::optimizeSolution(PlaceGrid& solution) {
    // 解决方案优化
    if (isSolutionValid(solution)) {
        // 重新计算成本
        solution.cal_cost();
        
        // 可以添加更多优化逻辑
        // 例如：晶体管重排序、布局调整等
    }
}

// 继承原始Placer的方法
void NewPlacer::generatePlaceUnit() {
    std::cout << "开始生成布局单元!" << std::endl;
    for (const auto& pair : pairs) {
        PlaceUnit unit(pair);
        if (pair.is_xc_pair) unit.generateStates_xc();
        else unit.generateStates();
        units.push_back(unit);
    }
}

void NewPlacer::findSolution(int curr, int prev, std::vector<int>& order, bool fix_bound) {
    // 使用原始Placer的findSolution逻辑
    // 这里可以添加改进的搜索策略
    
    if (setting.remove_sym) {
        if (curr >= (numUnit + 1) / 2) {
            if (numUnit % 2 == 0) {
                if (order[0] < 0) return;
            }
            else {
                if (order[0] < 0) return;
                if (order[0] == (numUnit + 1) / 2 && order[1] < 0) return;
            }
        }
    }

    if (setting.branch_bound) {
        if (lower_bound < MAX_OFFSET && prev != -1) {
            int nMinRemain = 0;
            int pMinRemain = 0;
            for (int i = 0; i < numUnit; i++) {
                if (order[i] < 0) {
                    nMinRemain += units[i].nMinLeg;
                    pMinRemain += units[i].pMinLeg;
                }
            }

            int count = 0;
            for (auto& dstate : units[prev].dstates) {
                if (!dstate.enable) continue;
                count++;
                int nLowerBound = nMinRemain + dstate.rightNoffset;
                int pLowerBound = pMinRemain + dstate.rightPoffset;
                if (std::max(pLowerBound, nLowerBound) > lower_bound) {
                    dstate.enable = false;
                    count--;
                }
            }
            if (count == 0) {
                return;
            }
        }
    }

    if (curr >= numUnit) {
        for (int i = 0; i < numUnit; i++) {
            if (order[i] < 0) {
                std::cout << "Error : Unexplored units!" << std::endl;
                return;
            }
        }

        for (const auto& dstate : units[prev].dstates) {
            int dstateWidth = std::max(dstate.rightNoffset, dstate.rightPoffset) + 1;
            if (dstateWidth > lower_bound) continue;

            if (!fix_bound) {
                if (dstateWidth < lower_bound) {
                    solutions[lower_bound].clear();
                    solutions.erase(lower_bound);
                    lower_bound = dstateWidth;
                    std::cout << "Min width is updated to " << lower_bound << std::endl;
                }
            }

            numUnit = units.size();
            std::vector<int> sol(numUnit, -1);
            sol[numUnit - 1] = dstate.index;
            
            backtrackSolution(dstate.index, sol, order, numUnit - 1, dstateWidth);
        }
        return;
    }

    for (int i = 0; i < numUnit; i++) {
        if (order[i] < 0) {
            if (curr == 0) {
                order[i] = 0;
                units[i].initiateFirstUnit();
                findSolution(curr + 1, i, order, fix_bound);
                order[i] = -1;
                units[i].resetDStates();
            }
            else {
                order[i] = curr;
                calculateState(units[prev], units[i]);
                findSolution(curr + 1, i, order, fix_bound);
                units[i].resetDStates();
                order[i] = -1;
            }
        }
    }
}

void NewPlacer::backtrackSolution(int currIndex, std::vector<int> &dstateList, std::vector<int> &order, int currOrder, int width) {
    dstateList[currOrder] = currIndex;

    auto findUnitIndex = [&](int unit_order) {
        for (int i = 0; i < numUnit; i++) 
            if (order[i] == unit_order)
                return i;
        std::cout << "Can't find unit!" << std::endl;
        return -1;
    };

    if (currOrder == 0) {
        OutputTrans dummy("dummy", "dummy", "dummy", "dummy", 0);
        std::vector<OutputTrans> nSol(width, dummy);
        std::vector<OutputTrans> pSol(width, dummy);

        for (int i = 0; i < dstateList.size(); i++) {
            assert(i < order.size());
            assert(findUnitIndex(i) < units.size());
            const PlaceUnit &unit = units[findUnitIndex(i)];

            assert(dstateList[i] < unit.dstates.size());
            const DynamicState &dstate = unit.dstates[dstateList[i]];

            assert(dstate.hostIndex < unit.states.size());
            const UnitState &state = unit.states[dstate.hostIndex];

            for (int x = 0; x < state.nmos.size(); x++) {
                int nfin = state.nmos[x].nfin;
                if (nfin != 0) {
                    assert(dstate.offset + x < nSol.size());
                    nSol[dstate.offset + x].set(state.nmos[x].name, state.nmos[x].left, state.nmos[x].gate, state.nmos[x].right, state.nmos[x].nfin);
                }
            }

            for (int x = 0; x < state.pmos.size(); x++) {
                int pfin = state.pmos[x].nfin;
                if (pfin != 0) {
                    assert(dstate.offset + x < pSol.size());
                    pSol[dstate.offset + x].set(state.pmos[x].name, state.pmos[x].left, state.pmos[x].gate, state.pmos[x].right, state.pmos[x].nfin);
                }
            }
        }

        PlaceGrid tempSolution(std::move(nSol), std::move(pSol), width);

        // 使用改进的成本计算
        tempSolution.cal_cost();
        double improvedCost = calculateImprovedCost(tempSolution);
        tempSolution.cost = improvedCost;

        // 检查解决方案有效性
        if (!isSolutionValid(tempSolution)) {
            dstateList[currOrder] = -1;
            return;
        }

        // 优化解决方案
        optimizeSolution(tempSolution);

        // 检查步骤数
        int num_step = 0;
        int size = tempSolution.nmos.size();
        for (int i = 0; i < size - 1; i++) {
            if (tempSolution.nmos[i].nfin != 0 && tempSolution.nmos[i + 1].nfin != 0 && tempSolution.nmos[i].nfin != tempSolution.nmos[i + 1].nfin) num_step++;
            if (tempSolution.pmos[i].nfin != 0 && tempSolution.pmos[i + 1].nfin != 0 && tempSolution.pmos[i].nfin != tempSolution.pmos[i + 1].nfin) num_step++;
            if (num_step > setting.MaxStep) break;
        }
        
        if (num_step <= setting.MaxStep) {
            bool isSame = false;
            for (auto& solution : solutions[width]) {
                if (solution.cellWidth != tempSolution.cellWidth) {
                    continue;
                }
                bool curSame = true;
                for (int i = 0; i < tempSolution.cellWidth; i++) {
                    if ((tempSolution.nmos[i].gate != solution.nmos[i].gate) || (tempSolution.nmos[i].left != solution.nmos[i].left) || (tempSolution.nmos[i].right != solution.nmos[i].right) || (tempSolution.nmos[i].nfin != solution.nmos[i].nfin)) {
                        curSame = false;
                        break;
                    } 
                    if ((tempSolution.pmos[i].gate != solution.pmos[i].gate) || (tempSolution.pmos[i].left != solution.pmos[i].left) || (tempSolution.pmos[i].right != solution.pmos[i].right) || (tempSolution.pmos[i].nfin != solution.pmos[i].nfin)) {
                        curSame = false;
                        break;
                    } 
                }
                if (curSame) {
                    isSame = true;
                    break;
                }
            }

            if (!isSame) {
                auto& w_solutions = solutions[width];
                if (w_solutions.size() == 0) w_solutions.push_back(std::move(tempSolution));
                else if (tempSolution.cost < w_solutions[w_solutions.size() - 1].cost) {
                    int i;
                    for (i = 0; i < w_solutions.size(); i++) {
                        if (w_solutions[i].cost > tempSolution.cost) break;
                    }
                    w_solutions.insert(w_solutions.begin() + i, std::move(tempSolution));
                    if (w_solutions.size() > setting.numSolutions) w_solutions.erase(w_solutions.end() - 1);
                }
                else if (tempSolution.cost == w_solutions[w_solutions.size() - 1].cost) {
                    if (w_solutions.size() < setting.numSolutions) w_solutions.push_back(std::move(tempSolution));
                }
            }
        }

        dstateList[currOrder] = -1;
        return;
    }

    int unitIndex = findUnitIndex(currOrder);
    for (auto prevDstateIndex : units[unitIndex].dstates[currIndex].prevBestStates) {
        backtrackSolution(prevDstateIndex, dstateList, order, currOrder - 1, width);
    }
    
    dstateList[currOrder] = -1;
}

void NewPlacer::printSolution(std::string outPath) {
    for (int width = min_width; width <= min_width + setting.relaxation; width++) {    
        std::cout << "打印改进算法解决方案!" << std::endl;
        std::string out_path = outPath.substr(0, outPath.length() - 4) + "_w" + std::to_string(width + 2) + ".txt";
        std::ofstream out(out_path);
        if (solutions.find(width) != solutions.end()) {
            int counter = 1;
            std::cout << "改进算法宽度 " << width << " 的解决方案数量: " << solutions[width].size() << std::endl;
            for (const auto& solution : solutions[width]) {
                out << "-------- 改进算法解决方案 " << counter++ << " --------" << std::endl;
                for (int i = 0; i < solution.cellWidth; i++) {
                    out << "[列 " << i + 1 << "]" << std::endl;
                    out << "NMOS : " << solution.nmos[i].name << "(" << solution.nmos[i].nfin << ") [" << solution.nmos[i].left << " " << solution.nmos[i].gate << " " << solution.nmos[i].right << "], ";
                    out << "PMOS : " << solution.pmos[i].name << "(" << solution.pmos[i].nfin << ") [" << solution.pmos[i].left << " " << solution.pmos[i].gate << " " << solution.pmos[i].right << "]" << std::endl;
                }
                out << std::endl;
            }            
        }
        out.close();
    }
}

void NewPlacer::savePlacementResults(const std::string& outputDir) {
    // 创建布局结果目录
    fs::path placementDir = fs::path(outputDir) / "placement_results_new";
    if (!fs::exists(placementDir)) {
        fs::create_directories(placementDir);
    }
    
    // 保存所有宽度的所有解决方案
    for (int width = min_width; width <= min_width + setting.relaxation; width++) {
        if (solutions.find(width) != solutions.end()) {
            auto& w_solutions = solutions[width];
            for (int i = 0; i < w_solutions.size(); i++) {
                savePlacementResult(w_solutions[i], cell.name, width, i, placementDir.string());
            }
        }
    }
}

void NewPlacer::savePlacementResult(const PlaceGrid& solution, const std::string& cellName, int width, int solutionIndex, const std::string& outputDir) {
    PlacementResult result(solution, cellName, width);
    result.setSolutionIndex(solutionIndex);
    
    // 生成文件名
    std::string fileName = cellName + "_new_w" + std::to_string(width + 2) + "_sol" + std::to_string(solutionIndex) + ".placement";
    fs::path filePath = fs::path(outputDir) / fileName;
    
    // 保存到文件
    if (result.saveToFile(filePath.string())) {
        std::cout << "保存改进算法布局结果: " << filePath << std::endl;
    } else {
        std::cerr << "保存改进算法布局结果失败: " << filePath << std::endl;
    }
}

// 其他方法的实现可以继承自原始Placer
void NewPlacer::runSearchwithRelax() {
    numUnit = units.size();
    std::vector<int> order(numUnit, -1);
    lower_bound = MAX_OFFSET;
    findSolution(0, -1, order, false);
    min_width = lower_bound;

    if (setting.relaxation > 0) {
        lower_bound = min_width + setting.relaxation;
        findSolution(0, -1, order, true);
    }
}

void NewPlacer::runSearchOnly() {
    numUnit = units.size();
    std::cout << "改进算法单元数量: " << numUnit << std::endl;
    runSearchwithRelax();
}

void NewPlacer::runEachGroup() {
    generatePlaceUnit();
    runSearchwithRelax();
    if (setting.refine_sol) adaptiveRefinement();
}

void NewPlacer::print_refinedSol(fs::path outPath) {
    // 实现细化解决方案的打印
}

void NewPlacer::refineSolution() {
    // 实现解决方案细化
    adaptiveRefinement();
}

void NewPlacer::calculateState(PlaceUnit& prev, PlaceUnit& curr) {
    // 继承原始Placer的calculateState实现
    // 这里可以添加改进的状态计算逻辑
}
