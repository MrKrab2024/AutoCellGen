#pragma once

#include "global.h"
#include "PlaceGrid.h"
#include <fstream>
#include <sstream>

class PlacementResult {
public:
    PlacementResult() = default;
    
    // 构造函数：从PlaceGrid创建布局结果
    PlacementResult(const PlaceGrid& placeGrid, const std::string& cellName, int width);
    
    // 从文件加载布局结果
    bool loadFromFile(const std::string& filePath);
    
    // 保存布局结果到文件
    bool saveToFile(const std::string& filePath) const;
    
    // 获取PlaceGrid对象
    PlaceGrid getPlaceGrid() const { return placeGrid; }
    
    // 获取单元名称
    std::string getCellName() const { return cellName; }
    
    // 获取宽度
    int getWidth() const { return width; }
    
    // 获取解决方案索引
    int getSolutionIndex() const { return solutionIndex; }
    
    // 设置解决方案索引
    void setSolutionIndex(int index) { solutionIndex = index; }

private:
    PlaceGrid placeGrid;
    std::string cellName;
    int width;
    int solutionIndex;
    
    // 解析布局结果文件的辅助函数
    bool parsePlacementFile(const std::string& content);
    std::vector<std::string> splitString(const std::string& str, char delimiter) const;
};
