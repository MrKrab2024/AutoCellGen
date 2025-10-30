#include "../header/global.h"
#include "../header/cdlParser.h"
#include "../header/Placer.h"
#include "../header/NewPlacer.h"
#include "../header/GroupPlacer.h"
#include "../header/Pairing.h"
#include "../header/ArgumentParser.h"
#include <chrono>
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

void parsing_DR(fs::path& dr_path) {
    std::ifstream dr_file(dr_path.string());

    auto split_by_space = [](std::string line) {
	    std::vector<std::string> tokens;
	    size_t prev = line.find_first_not_of(" ");
	    size_t pos;

	    while ((pos = line.find_first_of(" ", prev)) != std::string::npos) {
		    if (pos > prev) tokens.push_back(line.substr(prev, pos - prev));
		    prev = pos + 1;
	    }
	    if (prev < line.length()) tokens.push_back(line.substr(prev, std::string::npos));
	    return tokens;        
    };

    std::string curr_line;
    while (getline(dr_file, curr_line)) {
		std::cout << curr_line << std::endl;
        std::vector<std::string> tokens = split_by_space(curr_line);
        if (tokens.size() < 1) continue;
        if (tokens[0].front() == '#') continue;
        
        if (tokens[0] == "TECH") setting.tech = tokens[1]; 
        else if (tokens[0] == "FIN_DIFF_GAP") setting.WidthDiffGap = std::stoi(tokens[1]);
        else if (tokens[0] == "NET_DIFF_GAP") setting.NetDiffGap = std::stoi(tokens[1]);
        else if (tokens[0] == "NMOS_MAX_FIN") setting.NMOSMaxAllowedFin = std::stoi(tokens[1]);
        else if (tokens[0] == "PMOS_MAX_FIN") setting.PMOSMaxAllowedFin = std::stoi(tokens[1]);
        else if (tokens[0] == "MIN_OD_JOG") setting.MinODjog = std::stoi(tokens[1]);
        else if (tokens[0] == "MAX_STEP") setting.MaxStep = std::stoi(tokens[1]);
        else if (tokens[0] == "MIN_TR_NUM") setting.MinTrNum = std::stoi(tokens[1]);
        else if (tokens[0] == "MAX_INTRA_NUM") setting.MaxIntraNetNum = std::stoi(tokens[1]);
        else if (tokens[0] == "NUM_SOL") setting.numSolutions = std::stoi(tokens[1]);
        else if (tokens[0] == "ROUTE_SOL") setting.routeSolutions = std::stoi(tokens[1]);
        else if (tokens[0] == "XC_NUM") setting.XC = std::stoi(tokens[1]);
        else if (tokens[0] == "RELAXATION") setting.relaxation = std::stoi(tokens[1]);
		else if (tokens[0] == "MERGE_GROUP"){
			if (tokens[1] == "true") setting.merge_group = true;
			else setting.merge_group = false;
		}
		else if (tokens[0] == "LOGICAL_PARTITION"){
			if (tokens[1] == "true") setting.logical_partition = true;
			else setting.logical_partition = false;
		}
        else if (tokens[0] == "AVOID_GATECUT") {
            if (tokens[1] == "true") setting.avoid_gatecut = true;
            else setting.avoid_gatecut = false;
        }
        else if (tokens[0] == "FOLDING_STYLE") {
            setting.folding_style = tokens[1];
			std::cout << setting.folding_style << std::endl;
        }
        else if (tokens[0] == "REMOVE_SYM") {
            if (tokens[1] == "true") setting.remove_sym = true;
            else setting.remove_sym = false;
        }
        else if (tokens[0] == "REMOVE_DOM") {
            if (tokens[1] == "true") setting.remove_dom = true;
            else setting.remove_dom = false;
        }
        else if (tokens[0] == "BRANCH_BOUND") {
            if (tokens[1] == "true") setting.branch_bound = true;
            else setting.branch_bound = false;
        }
        else if (tokens[0] == "REFINE_SOL") {
            if (tokens[1] == "true") setting.refine_sol = true;
            else setting.refine_sol = false;
        }
        else if (tokens[0] == "M1_DIR") setting.m1_dir = tokens[1];
        else if (tokens[0] == "M2_DIR") setting.m2_dir = tokens[1];        
        else if (tokens[0] == "MIN_M2") {
            if (tokens[1] == "true") setting.min_m2 = true;
            else setting.min_m2 = false;
        }
        else if (tokens[0] == "MIN_M1") {
            if (tokens[1] == "true") setting.min_m1 = true;
            else setting.min_m1 = false;
        }
        else if (tokens[0] == "GEN_GDS") {
            if (tokens[1] == "true") setting.gen_gds = true;
            else setting.gen_gds = false;
        }
        else if (tokens[0] == "ROUTE_ACCEPT") setting.route_accept = std::stoi(tokens[1]);
        else {
            std::cout << "Error! Unidentified design rule: " << tokens[0] << std::endl;
            std::cout << "Error! Unidentified design rule: " << tokens[0] << std::endl;
        }
    }
    std::cout << setting.m1_dir << std::endl;
}

void make_IOnet(std::string outpath, Cell& cell){
    std::ofstream fout;
	fs::path path = outpath / fs::path(cell.name + "_IOnet" + ".txt");
    std::cout<<"io path: "<<path<<std::endl;
    fout.open(path);

    fout << cell.name << std::endl;
    for(int i=0; i<cell.IOnets.size(); ++i){
        fout << cell.IOnets[i] << std::endl;
    }
    
    fout.close();
}

int main(int argc, char **argv) {
    std::cout << "=== 布局算法比较程序 ===" << std::endl;
    
    ArgumentParser agparser(argc, argv);
    fs::path input_path, output_path, dr_path;

    // Find DR
    const std::string &dr_name = agparser.getCmdOption("-d");
    if (!dr_name.empty()) dr_path = fs::current_path() / fs::path(dr_name);
    else dr_path = fs::current_path() / fs::path("input/dr/ASAP7_placement.dr");
    
    parsing_DR(dr_path);

    // Find input cdl
    const std::string &input_name = agparser.getCmdOption("-i");
    if (!input_name.empty()) input_path = fs::current_path() / fs::path(input_name);
    else input_path = fs::current_path() / fs::path("input/cdl/asap7.sp");
    
    Library l;
    cdlParser parser(l, input_path);
    parser.parse();

    // Specify output path
    const std::string &output_name = agparser.getCmdOption("-o");
    if (!output_name.empty()) output_path = fs::current_path() / fs::path(output_name);
    else output_path = fs::current_path() / fs::path("input/7.5Tdataset");
	std::cout << output_path << std::endl;
	if (!fs::exists(output_path)) fs::create_directories(output_path);

    // 创建比较结果目录
    fs::path comparisonDir = output_path / "placement_comparison";
    if (!fs::exists(comparisonDir)) fs::create_directories(comparisonDir);

    int ncell = l.cells.size();
    std::cout << "单元数量: " << l.cells.size() << std::endl;

    std::ofstream out(comparisonDir.string() + "/comparison_summary.txt");

    for (int i = 0; i < ncell; i++) {
        Cell &temp = l.cells[i];
        
        std::cout << "\n============================================" << std::endl;
        std::cout << "处理单元: " << temp.name << std::endl;
        std::cout << "晶体管数量: " << temp.trans.size() << std::endl;
        std::cout << "============================================\n" << std::endl;
        
        out << "(" << i << ") ";
        out << "单元名称: " << temp.name << std::endl;
        out << "晶体管数量: " << temp.trans.size() << std::endl;

        fs::path cell_output_path = comparisonDir / fs::path(temp.name);
        if (!fs::exists(cell_output_path)) fs::create_directories(cell_output_path);
        
        if (temp.trans.size() > 10) {
            std::cout << "使用GroupPlacer进行布局..." << std::endl;
            
            // 运行原始GroupPlacer
            std::cout << "运行原始GroupPlacer算法..." << std::endl;
            GroupPlacer originalPlacer(l.cells[i]);
            originalPlacer.out_dir = cell_output_path.string();
            auto start_original = std::chrono::steady_clock::now();
            originalPlacer.run();
            auto end_original = std::chrono::steady_clock::now();
            auto duration_original = std::chrono::duration_cast<std::chrono::milliseconds>(end_original - start_original);
            
            out << "原始算法最小宽度: " << originalPlacer.min_width + 2 << std::endl;
            out << "原始算法运行时间: " << duration_original.count() << "ms" << std::endl;
            
            // 运行改进的NewPlacer
            std::cout << "运行改进的NewPlacer算法..." << std::endl;
            NewPlacer newPlacer(l.cells[i]);
            newPlacer.out_dir = cell_output_path.string();
            auto start_new = std::chrono::steady_clock::now();
            newPlacer.run();
            auto end_new = std::chrono::steady_clock::now();
            auto duration_new = std::chrono::duration_cast<std::chrono::milliseconds>(end_new - start_new);
            
            out << "改进算法最小宽度: " << newPlacer.min_width + 2 << std::endl;
            out << "改进算法运行时间: " << duration_new.count() << "ms" << std::endl;
            
            // 比较结果
            std::cout << "比较布局结果..." << std::endl;
            newPlacer.compareWithOriginal(originalPlacer, cell_output_path.string());
            
            // 计算改进效果
            double widthImprovement = 0.0;
            if (originalPlacer.min_width > 0) {
                widthImprovement = (double)(originalPlacer.min_width - newPlacer.min_width) / originalPlacer.min_width * 100.0;
            }
            
            double timeImprovement = 0.0;
            if (duration_original.count() > 0) {
                timeImprovement = (double)(duration_original.count() - duration_new.count()) / duration_original.count() * 100.0;
            }
            
            out << "宽度改进: " << std::fixed << std::setprecision(2) << widthImprovement << "%" << std::endl;
            out << "时间改进: " << std::fixed << std::setprecision(2) << timeImprovement << "%" << std::endl;
            
            std::cout << "宽度改进: " << widthImprovement << "%" << std::endl;
            std::cout << "时间改进: " << timeImprovement << "%" << std::endl;
        }
        else { 
            std::cout << "使用Placer进行布局..." << std::endl;
            
            // 运行原始Placer
            std::cout << "运行原始Placer算法..." << std::endl;
            Placer originalPlacer(l.cells[i]);
            originalPlacer.out_dir = cell_output_path.string();
            auto start_original = std::chrono::steady_clock::now();
            originalPlacer.run();
            auto end_original = std::chrono::steady_clock::now();
            auto duration_original = std::chrono::duration_cast<std::chrono::milliseconds>(end_original - start_original);
            
            out << "原始算法最小宽度: " << originalPlacer.min_width + 2 << std::endl;
            out << "原始算法运行时间: " << duration_original.count() << "ms" << std::endl;
            
            // 运行改进的NewPlacer
            std::cout << "运行改进的NewPlacer算法..." << std::endl;
            NewPlacer newPlacer(l.cells[i]);
            newPlacer.out_dir = cell_output_path.string();
            auto start_new = std::chrono::steady_clock::now();
            newPlacer.run();
            auto end_new = std::chrono::steady_clock::now();
            auto duration_new = std::chrono::duration_cast<std::chrono::milliseconds>(end_new - start_new);
            
            out << "改进算法最小宽度: " << newPlacer.min_width + 2 << std::endl;
            out << "改进算法运行时间: " << duration_new.count() << "ms" << std::endl;
            
            // 比较结果
            std::cout << "比较布局结果..." << std::endl;
            newPlacer.compareWithOriginal(originalPlacer, cell_output_path.string());
            
            // 计算改进效果
            double widthImprovement = 0.0;
            if (originalPlacer.min_width > 0) {
                widthImprovement = (double)(originalPlacer.min_width - newPlacer.min_width) / originalPlacer.min_width * 100.0;
            }
            
            double timeImprovement = 0.0;
            if (duration_original.count() > 0) {
                timeImprovement = (double)(duration_original.count() - duration_new.count()) / duration_original.count() * 100.0;
            }
            
            out << "宽度改进: " << std::fixed << std::setprecision(2) << widthImprovement << "%" << std::endl;
            out << "时间改进: " << std::fixed << std::setprecision(2) << timeImprovement << "%" << std::endl;
            
            std::cout << "宽度改进: " << widthImprovement << "%" << std::endl;
            std::cout << "时间改进: " << timeImprovement << "%" << std::endl;
        }
        
        // 生成IOnet文件
        fs::path IOpath = output_path / fs::path("../IOnet/"); 
        if(!fs::exists(IOpath)) fs::create_directories(IOpath);
        make_IOnet(IOpath.string(), temp);
        
        out << std::endl;
    }
    
    std::cout << "\n=== 布局算法比较完成 ===" << std::endl;
    std::cout << "比较结果保存在: " << comparisonDir << std::endl;
    out.close();
    
    return 0;
}
