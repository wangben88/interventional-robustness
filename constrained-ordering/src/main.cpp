#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <map>
#include <vector>
#include "utils.h"
#include "graphModel.h"
#include "reader.h"
#include <algorithm>

void help(){
    std::cerr << "\nUsage:\n   ./constrained_ordering -i bn.net -c constraints.txt -o ordering.txt -m modconstraints.txt \n\n";
    std::cerr << "   Options:\n";
    std::cerr << "      -i <filename>: Input (.net file)\n";
    std::cerr << "      -c <filename>: Constraint file (.txt) - optional\n";
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
        return 1;
    }


    std::map<std::string, std::vector<std::string> > constraints;
    if (!constraintFile.empty()) {
        constraints = readConstraints(constraintFile);
    }

    GraphModel bn;
    bn.readNET(netFile);

    // Add all topological constraints (i.e. involving node and its parents in the Bayesian network). While this is
    // not strictly necessary, it is usually a good idea to enforce, unless the compilation is too slow.
    constraints = bn.addTopologicalConstraints(constraints);

    writeConstraints(outConstraintFile, constraints);

    bn = bn.moralize();

    std::vector<std::string> ordering = bn.getOrdering(GraphModel::Heuristic::MIN_FILL, GraphModel::Constraint::PARTIAL_ORDER, constraints);

    std::ofstream fout(outFile);

    // getOrdering enforces constraints that ensure that nodes come before their parents in the ordering. We reverse
    // to get an ordering where parents come before nodes.
    // (technical note: this is not equivalent to just writing getOrdering/the constraints so that parents come before
    // nodes, without reversing, because the heuristic would be different. we do it in this way because it uses the
    // heuristic in the correct way, for the compilation downstream).
    std::reverse(ordering.begin(), ordering.end());
    fout << ordering.size() << std::endl;
    for (auto str: ordering) {
        fout << str << std::endl;
    }
}