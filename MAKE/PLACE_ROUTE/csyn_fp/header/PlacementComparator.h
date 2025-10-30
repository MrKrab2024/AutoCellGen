#pragma once

#include "global.h"
#include "PlaceGrid.h"
#include "PlacementResult.h"
#include <vector>
#include <string>
#include <fstream>
#include <chrono>

struct PlacementMetrics {
    int cellWidth;
    double cost;
    double nor_hpwl;
    double max_h_grid;
    double max_v_grid;
    double max_h_column;
    double max_v_column;
    double max_h_row;
    double max_v_row;
    int64_t runtime_ms;
    std::string algorithm_name;
    
    PlacementMetrics() : cellWidth(0), cost(0.0), nor_hpwl(0.0), 
                        max_h_grid(0.0), max_v_grid(0.0), 
                        max_h_column(0.0), max_v_column(0.0),
                        max_h_row(0.0), max_v_row(0.0), runtime_ms(0) {}
};

class PlacementComparator {
public:
    PlacementComparator() = default;
    
    // 添加布局结果用于比较
    void addPlacementResult(const PlacementResult& result, const std::string& algorithm_name, int64_t runtime_ms = 0);
    
    // 添加PlaceGrid结果用于比较
    void addPlaceGridResult(const PlaceGrid& placeGrid, const std::string& cellName, int width, 
                           const std::string& algorithm_name, int64_t runtime_ms = 0);
    
    // 比较所有算法结果
    void compareResults();
    
    // 生成比较报告
    void generateComparisonReport(const std::string& outputPath);
    
    // 获取最佳结果
    PlacementMetrics getBestResult(const std::string& metric = "cost");
    
    // 获取所有结果
    const std::vector<PlacementMetrics>& getAllResults() const { return results; }
    
    // 清空结果
    void clear() { results.clear(); }

private:
    std::vector<PlacementMetrics> results;
    
    // 从PlaceGrid提取指标
    PlacementMetrics extractMetrics(const PlaceGrid& placeGrid, const std::string& cellName, 
                                   int width, const std::string& algorithm_name, int64_t runtime_ms);
    
    // 生成CSV格式的比较报告
    void generateCSVReport(const std::string& outputPath);
    
    // 生成HTML格式的比较报告
    void generateHTMLReport(const std::string& outputPath);
};
