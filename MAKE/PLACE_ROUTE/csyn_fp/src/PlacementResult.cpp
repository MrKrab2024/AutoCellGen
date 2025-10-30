#include "../header/PlacementResult.h"

PlacementResult::PlacementResult(const PlaceGrid& placeGrid, const std::string& cellName, int width) 
    : placeGrid(placeGrid), cellName(cellName), width(width), solutionIndex(0) {
}

bool PlacementResult::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open placement file: " << filePath << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    return parsePlacementFile(content);
}

bool PlacementResult::saveToFile(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create placement file: " << filePath << std::endl;
        return false;
    }
    
    // 写入文件头
    file << "# Placement Result File" << std::endl;
    file << "# Cell: " << cellName << std::endl;
    file << "# Width: " << width << std::endl;
    file << "# Solution Index: " << solutionIndex << std::endl;
    file << "# Format: [Column] NMOS(name,left,gate,right,nfin) PMOS(name,left,gate,right,nfin)" << std::endl;
    file << "BEGIN_PLACEMENT" << std::endl;
    
    // 写入布局数据
    for (int i = 0; i < placeGrid.cellWidth; i++) {
        file << "[Column " << (i + 1) << "]" << std::endl;
        file << "NMOS: " << placeGrid.nmos[i].name 
             << "(" << placeGrid.nmos[i].nfin << ") [" 
             << placeGrid.nmos[i].left << " " 
             << placeGrid.nmos[i].gate << " " 
             << placeGrid.nmos[i].right << "], ";
        file << "PMOS: " << placeGrid.pmos[i].name 
             << "(" << placeGrid.pmos[i].nfin << ") [" 
             << placeGrid.pmos[i].left << " " 
             << placeGrid.pmos[i].gate << " " 
             << placeGrid.pmos[i].right << "]" << std::endl;
    }
    
    file << "END_PLACEMENT" << std::endl;
    file.close();
    
    return true;
}

bool PlacementResult::parsePlacementFile(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    
    // 解析文件头
    while (std::getline(stream, line)) {
        if (line.find("# Cell:") != std::string::npos) {
            cellName = line.substr(line.find(":") + 2);
        } else if (line.find("# Width:") != std::string::npos) {
            width = std::stoi(line.substr(line.find(":") + 2));
        } else if (line.find("# Solution Index:") != std::string::npos) {
            solutionIndex = std::stoi(line.substr(line.find(":") + 2));
        } else if (line == "BEGIN_PLACEMENT") {
            break;
        }
    }
    
    // 解析布局数据
    std::vector<OutputTrans> nmos, pmos;
    int cellWidth = 0;
    
    while (std::getline(stream, line)) {
        if (line == "END_PLACEMENT") {
            break;
        }
        
        if (line.find("[Column") != std::string::npos) {
            // 解析列数据
            std::getline(stream, line); // 读取NMOS和PMOS数据行
            
            // 解析NMOS数据
            size_t nmosStart = line.find("NMOS: ");
            size_t pmosStart = line.find("PMOS: ");
            
            if (nmosStart != std::string::npos && pmosStart != std::string::npos) {
                std::string nmosData = line.substr(nmosStart + 6, pmosStart - nmosStart - 6);
                std::string pmosData = line.substr(pmosStart + 6);
                
                // 解析NMOS
                std::vector<std::string> nmosTokens = splitString(nmosData, ' ');
                if (nmosTokens.size() >= 4) {
                    std::string name = nmosTokens[0];
                    int nfin = 0;
                    std::string left, gate, right;
                    
                    // 解析括号中的nfin
                    size_t parenStart = name.find('(');
                    if (parenStart != std::string::npos) {
                        size_t parenEnd = name.find(')', parenStart);
                        if (parenEnd != std::string::npos) {
                            nfin = std::stoi(name.substr(parenStart + 1, parenEnd - parenStart - 1));
                            name = name.substr(0, parenStart);
                        }
                    }
                    
                    // 解析方括号中的left, gate, right
                    size_t bracketStart = line.find('[');
                    size_t bracketEnd = line.find(']', bracketStart);
                    if (bracketStart != std::string::npos && bracketEnd != std::string::npos) {
                        std::string bracketData = line.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
                        std::vector<std::string> bracketTokens = splitString(bracketData, ' ');
                        if (bracketTokens.size() >= 3) {
                            left = bracketTokens[0];
                            gate = bracketTokens[1];
                            right = bracketTokens[2];
                        }
                    }
                    
                    nmos.emplace_back(name, left, gate, right, nfin);
                }
                
                // 解析PMOS
                std::vector<std::string> pmosTokens = splitString(pmosData, ' ');
                if (pmosTokens.size() >= 4) {
                    std::string name = pmosTokens[0];
                    int nfin = 0;
                    std::string left, gate, right;
                    
                    // 解析括号中的nfin
                    size_t parenStart = name.find('(');
                    if (parenStart != std::string::npos) {
                        size_t parenEnd = name.find(')', parenStart);
                        if (parenEnd != std::string::npos) {
                            nfin = std::stoi(name.substr(parenStart + 1, parenEnd - parenStart - 1));
                            name = name.substr(0, parenStart);
                        }
                    }
                    
                    // 解析方括号中的left, gate, right
                    size_t bracketStart = pmosData.find('[');
                    size_t bracketEnd = pmosData.find(']', bracketStart);
                    if (bracketStart != std::string::npos && bracketEnd != std::string::npos) {
                        std::string bracketData = pmosData.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
                        std::vector<std::string> bracketTokens = splitString(bracketData, ' ');
                        if (bracketTokens.size() >= 3) {
                            left = bracketTokens[0];
                            gate = bracketTokens[1];
                            right = bracketTokens[2];
                        }
                    }
                    
                    pmos.emplace_back(name, left, gate, right, nfin);
                }
                
                cellWidth++;
            }
        }
    }
    
    // 创建PlaceGrid对象
    placeGrid = PlaceGrid(nmos, pmos, cellWidth);
    
    return true;
}

std::vector<std::string> PlacementResult::splitString(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}
