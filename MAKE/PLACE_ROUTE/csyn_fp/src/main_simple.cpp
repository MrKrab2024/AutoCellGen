#include "../header/global.h"
#include "../header/Placer.h"
#include "../header/GroupPlacer.h"
#include "../header/Pairing.h"
#include "../header/cdlParser.h"
#include "../header/beol_data.h"
#include "../header/setting.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <netlist_file> [output_path]" << std::endl;
        return 1;
    }

    std::string netlist_file = argv[1];
    std::string output_path = (argc >= 3) ? argv[2] : "./output";

    // Create output directory
    fs::create_directories(output_path);

    // Parse netlist
    Library l;
    cdlParser parser(l, netlist_file);
    parser.parse();

    std::cout << "Parsed " << l.cells.size() << " cells from " << netlist_file << std::endl;

    // Process each cell
    for (int i = 0; i < l.cells.size(); i++) {
        Cell &temp = l.cells[i];
        std::cout << "Processing cell: " << temp.name << " with " << temp.trans.size() << " transistors" << std::endl;

        if (temp.trans.size() > 10) {
            std::cout << "Using GroupPlacer for large cell" << std::endl;
            GroupPlacer placer(l.cells[i]);
            placer.out_dir = output_path;
            
            auto start = std::chrono::steady_clock::now();
            placer.run();
            auto end = std::chrono::steady_clock::now();
            
            std::cout << "Min #CPP = " << placer.min_width + 2 << std::endl;
            std::cout << "Placement time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
        } else {
            std::cout << "Using Placer for small cell" << std::endl;
            Placer placer(l.cells[i]);
            placer.out_dir = output_path;
            
            auto start = std::chrono::steady_clock::now();
            placer.run();
            auto end = std::chrono::steady_clock::now();
            
            std::cout << "Min #CPP = " << placer.min_width + 2 << std::endl;
            std::cout << "Placement time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
        }
    }

    std::cout << "Placement completed!" << std::endl;
    return 0;
}
