#pragma once

#include "global.h"
#include "beol_data.h"
#include "setting.h"
#include "PlaceUnit.h"
#include "PlaceGrid.h"
#include "PlaceGroupUnit.h"
#include "PlacementResult.h"
#include "PlacementComparator.h"

#define MAX_OFFSET 9999

class NewPlacer {
public:
    NewPlacer();
    NewPlacer(Cell& c, int mw=MAX_OFFSET) : cell(c), min_width(mw){}

    void run();
    void runSearchwithRelax();
    void runSearchOnly();
    void runEachGroup();

    // 新增：改进的布局算法
    void runImprovedPlacement();
    void runOptimizedPlacement();
    
    // 新增：布局结果比较
    void compareWithOriginal(Placer& originalPlacer, const std::string& outputDir);

    // input variables
    Cell cell;
    int min_width; 
    int lower_bound;

    std::string out_dir;

    // after pairing
    std::unordered_map<int, int> pairGroup;
    int numGroup;
    std::vector<Pair> pairs;
    
    // after generatePlaceUnit
    std::vector<PlaceUnit> units;
    int numUnit;

    // after findSolution
    std::unordered_map<int, std::vector<PlaceGrid>> solutions;

    std::ofstream dbg;

    void generatePlaceUnit();
    void findSolution(int curr, int prev, std::vector<int>& order, bool fix_bound);
    void backtrackSolution (int currIndex, std::vector<int> &dstates, std::vector<int> &order, int currOrder, int width);
    void printSolution(std::string outPath);
    void print_refinedSol(fs::path outPath);
    void refineSolution();

    // 新增：保存布局结果到文件
    void savePlacementResults(const std::string& outputDir);
    void savePlacementResult(const PlaceGrid& solution, const std::string& cellName, int width, int solutionIndex, const std::string& outputDir);

    void calculateState (PlaceUnit& prev, PlaceUnit& curr);
    
    // 新增：改进的算法方法
    void improvedPairing();
    void optimizedStateGeneration();
    void enhancedSolutionSearch();
    void adaptiveRefinement();
    
    // 新增：性能分析
    void analyzePerformance();
    void logAlgorithmMetrics(const std::string& algorithmName, int64_t runtime);
    
private:
    PlacementComparator comparator;
    std::vector<std::pair<std::string, int64_t>> performanceLog;
    
    // 改进算法的辅助方法
    double calculateImprovedCost(const PlaceGrid& solution);
    bool isSolutionValid(const PlaceGrid& solution);
    void optimizeSolution(PlaceGrid& solution);
};
