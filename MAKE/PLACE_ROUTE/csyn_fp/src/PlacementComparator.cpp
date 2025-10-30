#include "../header/PlacementComparator.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

void PlacementComparator::addPlacementResult(const PlacementResult& result, const std::string& algorithm_name, int64_t runtime_ms) {
    PlacementMetrics metrics;
    metrics.cellWidth = result.getPlaceGrid().cellWidth;
    metrics.cost = result.getPlaceGrid().cost;
    metrics.nor_hpwl = result.getPlaceGrid().nor_hpwl;
    metrics.max_h_grid = result.getPlaceGrid().max_h_grid;
    metrics.max_v_grid = result.getPlaceGrid().max_v_grid;
    metrics.max_h_column = result.getPlaceGrid().max_h_column;
    metrics.max_v_column = result.getPlaceGrid().max_v_column;
    metrics.max_h_row = result.getPlaceGrid().max_h_row;
    metrics.max_v_row = result.getPlaceGrid().max_v_row;
    metrics.runtime_ms = runtime_ms;
    metrics.algorithm_name = algorithm_name;
    
    results.push_back(metrics);
}

void PlacementComparator::addPlaceGridResult(const PlaceGrid& placeGrid, const std::string& cellName, int width, 
                                           const std::string& algorithm_name, int64_t runtime_ms) {
    PlacementMetrics metrics = extractMetrics(placeGrid, cellName, width, algorithm_name, runtime_ms);
    results.push_back(metrics);
}

PlacementMetrics PlacementComparator::extractMetrics(const PlaceGrid& placeGrid, const std::string& cellName, 
                                                    int width, const std::string& algorithm_name, int64_t runtime_ms) {
    PlacementMetrics metrics;
    metrics.cellWidth = placeGrid.cellWidth;
    metrics.cost = placeGrid.cost;
    metrics.nor_hpwl = placeGrid.nor_hpwl;
    metrics.max_h_grid = placeGrid.max_h_grid;
    metrics.max_v_grid = placeGrid.max_v_grid;
    metrics.max_h_column = placeGrid.max_h_column;
    metrics.max_v_column = placeGrid.max_v_column;
    metrics.max_h_row = placeGrid.max_h_row;
    metrics.max_v_row = placeGrid.max_v_row;
    metrics.runtime_ms = runtime_ms;
    metrics.algorithm_name = algorithm_name;
    
    return metrics;
}

void PlacementComparator::compareResults() {
    if (results.empty()) {
        std::cout << "No results to compare." << std::endl;
        return;
    }
    
    std::cout << "\n=== 布局算法比较结果 ===" << std::endl;
    std::cout << "算法数量: " << results.size() << std::endl;
    
    // 按成本排序
    std::sort(results.begin(), results.end(), 
              [](const PlacementMetrics& a, const PlacementMetrics& b) {
                  return a.cost < b.cost;
              });
    
    std::cout << "\n按成本排序的结果:" << std::endl;
    std::cout << std::setw(15) << "算法名称" 
              << std::setw(10) << "宽度" 
              << std::setw(12) << "成本" 
              << std::setw(12) << "HPWL" 
              << std::setw(10) << "运行时间(ms)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (const auto& result : results) {
        std::cout << std::setw(15) << result.algorithm_name
                  << std::setw(10) << result.cellWidth
                  << std::setw(12) << std::fixed << std::setprecision(2) << result.cost
                  << std::setw(12) << std::fixed << std::setprecision(2) << result.nor_hpwl
                  << std::setw(10) << result.runtime_ms << std::endl;
    }
    
    // 找出最佳结果
    auto bestCost = std::min_element(results.begin(), results.end(),
                                   [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                       return a.cost < b.cost;
                                   });
    
    auto bestRuntime = std::min_element(results.begin(), results.end(),
                                      [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                          return a.runtime_ms < b.runtime_ms;
                                      });
    
    std::cout << "\n最佳结果:" << std::endl;
    std::cout << "最低成本: " << bestCost->algorithm_name << " (成本: " << bestCost->cost << ")" << std::endl;
    std::cout << "最快速度: " << bestRuntime->algorithm_name << " (时间: " << bestRuntime->runtime_ms << "ms)" << std::endl;
}

void PlacementComparator::generateComparisonReport(const std::string& outputPath) {
    generateCSVReport(outputPath + ".csv");
    generateHTMLReport(outputPath + ".html");
}

void PlacementComparator::generateCSVReport(const std::string& outputPath) {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "无法创建CSV报告文件: " << outputPath << std::endl;
        return;
    }
    
    // 写入CSV头部
    file << "算法名称,单元宽度,成本,HPWL,最大H网格,最大V网格,最大H列,最大V列,最大H行,最大V行,运行时间(ms)" << std::endl;
    
    // 写入数据
    for (const auto& result : results) {
        file << result.algorithm_name << ","
             << result.cellWidth << ","
             << std::fixed << std::setprecision(6) << result.cost << ","
             << std::fixed << std::setprecision(6) << result.nor_hpwl << ","
             << std::fixed << std::setprecision(6) << result.max_h_grid << ","
             << std::fixed << std::setprecision(6) << result.max_v_grid << ","
             << std::fixed << std::setprecision(6) << result.max_h_column << ","
             << std::fixed << std::setprecision(6) << result.max_v_column << ","
             << std::fixed << std::setprecision(6) << result.max_h_row << ","
             << std::fixed << std::setprecision(6) << result.max_v_row << ","
             << result.runtime_ms << std::endl;
    }
    
    file.close();
    std::cout << "CSV报告已生成: " << outputPath << std::endl;
}

void PlacementComparator::generateHTMLReport(const std::string& outputPath) {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "无法创建HTML报告文件: " << outputPath << std::endl;
        return;
    }
    
    file << "<!DOCTYPE html>" << std::endl;
    file << "<html><head><title>布局算法比较报告</title>" << std::endl;
    file << "<style>" << std::endl;
    file << "table { border-collapse: collapse; width: 100%; }" << std::endl;
    file << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }" << std::endl;
    file << "th { background-color: #f2f2f2; }" << std::endl;
    file << ".best { background-color: #d4edda; }" << std::endl;
    file << "</style></head><body>" << std::endl;
    file << "<h1>布局算法比较报告</h1>" << std::endl;
    file << "<p>生成时间: " << std::chrono::system_clock::now().time_since_epoch().count() << "</p>" << std::endl;
    
    // 找出最佳结果
    auto bestCost = std::min_element(results.begin(), results.end(),
                                   [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                       return a.cost < b.cost;
                                   });
    
    auto bestRuntime = std::min_element(results.begin(), results.end(),
                                      [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                          return a.runtime_ms < b.runtime_ms;
                                      });
    
    file << "<h2>最佳结果</h2>" << std::endl;
    file << "<ul>" << std::endl;
    file << "<li>最低成本: " << bestCost->algorithm_name << " (成本: " << bestCost->cost << ")</li>" << std::endl;
    file << "<li>最快速度: " << bestRuntime->algorithm_name << " (时间: " << bestRuntime->runtime_ms << "ms)</li>" << std::endl;
    file << "</ul>" << std::endl;
    
    file << "<h2>详细比较</h2>" << std::endl;
    file << "<table>" << std::endl;
    file << "<tr><th>算法名称</th><th>单元宽度</th><th>成本</th><th>HPWL</th><th>最大H网格</th><th>最大V网格</th><th>最大H列</th><th>最大V列</th><th>最大H行</th><th>最大V行</th><th>运行时间(ms)</th></tr>" << std::endl;
    
    for (const auto& result : results) {
        std::string rowClass = "";
        if (&result == &(*bestCost)) rowClass = " class=\"best\"";
        
        file << "<tr" << rowClass << ">" << std::endl;
        file << "<td>" << result.algorithm_name << "</td>" << std::endl;
        file << "<td>" << result.cellWidth << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.cost << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.nor_hpwl << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.max_h_grid << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.max_v_grid << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.max_h_column << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.max_v_column << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.max_h_row << "</td>" << std::endl;
        file << "<td>" << std::fixed << std::setprecision(6) << result.max_v_row << "</td>" << std::endl;
        file << "<td>" << result.runtime_ms << "</td>" << std::endl;
        file << "</tr>" << std::endl;
    }
    
    file << "</table>" << std::endl;
    file << "</body></html>" << std::endl;
    
    file.close();
    std::cout << "HTML报告已生成: " << outputPath << std::endl;
}

PlacementMetrics PlacementComparator::getBestResult(const std::string& metric) {
    if (results.empty()) {
        return PlacementMetrics();
    }
    
    if (metric == "cost") {
        return *std::min_element(results.begin(), results.end(),
                               [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                   return a.cost < b.cost;
                               });
    } else if (metric == "runtime") {
        return *std::min_element(results.begin(), results.end(),
                               [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                   return a.runtime_ms < b.runtime_ms;
                               });
    } else if (metric == "hpwl") {
        return *std::min_element(results.begin(), results.end(),
                               [](const PlacementMetrics& a, const PlacementMetrics& b) {
                                   return a.nor_hpwl < b.nor_hpwl;
                               });
    }
    
    return results[0]; // 默认返回第一个结果
}
