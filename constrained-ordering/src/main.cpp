#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>
#include <map>
#include <vector>
#include "utils.h"
#include "graphModel.h"
#include "reader.h"

void help(){
    std::cerr << "\nUsage:\n   ./bw_obdd_to_cnf -i <filename> \n\n";
    std::cerr << "   Options:\n";
    std::cerr << "      -i <filename>: Input (.net file)\n";
    std::cerr << "      -c <filename>: Constraint file (.txt)\n";
    std::cerr << "      -o <filename>: Output filename for ordering (.txt)\n";
    std::cerr << "      -m <filename>: Output filename for modified constraints (.txt)\n";
    std::cerr << "      -h: Help\n";
}



int main(int argc, char **argv) {
    int c;
    std::string netFile;
    std::string constraintFile;
    std::string outFile;
    std::string outConstraintFile;
    int sinks = 2; // default 2 sinks

    while ((c = getopt(argc, argv, "i:c:o:m:")) != -1) {
        switch (c) {
            case 'i': // provide input
            {
                netFile = optarg;
                std::string ext = get_filename_ext(netFile.c_str());
                if (ext != "net") {
                    std::cerr << "Unknown file extension, a '*.net' file is required\n";
                    return 1;
                }
            }
                break;
            case 'c':
            {
                constraintFile = optarg;
                std::string ext = get_filename_ext(constraintFile.c_str());
                if (ext != "txt") {
                    std::cerr << "Unknown file extension, a '*.txt' file is required\n";
                    return 1;
                }
            }
                break;
            case 'o':
            {
                outFile = optarg;
                std::string ext = get_filename_ext(outFile.c_str());
                if (ext != "txt") {
                    std::cerr << "Unknown file extension, a '*.txt' file is required\n";
                    return 1;
                }
            }
                break;
            case 'm':
            {
                outConstraintFile = optarg;
                std::string ext = get_filename_ext(outConstraintFile.c_str());
                if (ext != "txt") {
                    std::cerr << "Unknown file extension, a '*.txt' file is required\n";
                    return 1;
                }
            }
                break;
            default:
                help();
                return 1;
        }
    }

    for (int index = optind; index < argc; index++)
        std::cout << "Non-option argument " << argv[index] << std::endl;

    if (netFile.empty()) {
        help();
        std::cerr << "Specify input file with -i <filename>\n";
        return 1;
    }

    std::map<std::string, std::vector<std::string> > constraints = readConstraints(constraintFile);

    GraphModel bn;
    bn.readNET(netFile);

    constraints = bn.addTopologicalConstraints(constraints);

    writeConstraints(outConstraintFile, constraints);

    bn = bn.moralize();

    std::vector<std::string> ordering = bn.getOrdering(GraphModel::Heuristic::MIN_FILL, GraphModel::Constraint::PARTIAL_ORDER, constraints);

    std::ofstream fout(outFile);

    // we are reversing in order to then use dt_method 3 (so that classifier nodes are also in the right place)
    std::reverse(ordering.begin(), ordering.end());
    for (auto str: ordering) {
        fout << str << std::endl;
    }
}