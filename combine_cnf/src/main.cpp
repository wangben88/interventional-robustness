#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "utils.h"
#include "parser.h"
#include "logicNode.h"

void help(){
    std::cerr << "\nUsage:\n   ./"
                 "combine_cnf -c bn.cnf -d df.cnf -m constraints.txt -o combined \n\n";
    std::cerr << "   Options:\n";
    std::cerr << "      -c <filename>: CNF for Bayesian Network (.cnf file)\n";
    std::cerr << "      -d <filename>: CNF for Decision Function (.cnf file)\n";
    std::cerr << "      -m <sinks>: Ordering Constraints (.txt file)\n";
    std::cerr << "      -o <sinks>: Output filename for combined cnf + lmap file (.txt files)\n";
    std::cerr << "      -h: Help\n";
}

int main(int argc, char **argv){
    int c;

    std::string bnCnfFile;
    std::string dfCnfFile;
    std::string constraintFile;
    std::string outFile;
    int sinks = 2; // default 2 sinks

    while ((c = getopt(argc, argv, "c:d:m:o:")) != -1){
        switch (c){
            case 'c': // provide input
            {
                bnCnfFile = optarg;
                std::string ext = get_filename_ext(bnCnfFile.c_str());
                if (ext != "cnf") {
                    std::cerr << "Unknown file extension, a '*.cnf' file is required for option -c\n";
                    return 1;
                }
            }
                break;
            case 'd': // provide input
            {
                dfCnfFile = optarg;
                std::string ext = get_filename_ext(dfCnfFile.c_str());
                if (ext != "cnf") {
                    std::cerr << "Unknown file extension, a '*.cnf' file is required for option -d\n";
                    return 1;
                }
            }
                break;
            case 'm': // provide input
            {
                constraintFile = optarg;
                std::string ext = get_filename_ext(constraintFile.c_str());
                if (ext != "txt") {
                    std::cerr << "Unknown file extension, a '*.txt' file is required for option -m\n";
                    return 1;
                }
            }
                break;
            case 'o':
            {
                outFile = optarg;
            }
                break;
            default:
                help();
                return 1;
        }
    }

    for (int index = optind; index < argc; index++)
        std::cout << "Non-option argument " << argv[index] << std::endl;

    if (bnCnfFile.empty() || dfCnfFile.empty() || constraintFile.empty()) {
        help();
        std::cerr << "Missing required argument\n";
        return 1;
    }

    // Load Cnfs

    Cnf dfCnf;
    dfCnf.read(dfCnfFile);



    std::pair<Cnf, Lmap> outputs = loadCnfSpecial(bnCnfFile, dfCnf, constraintFile, outFile);
    outputs.first.write(outFile + ".cnf");
    //outputs.second.write(outFile + ".lmap"); // For some reason, printing here instead of inside loadCnfSpecial
                                               // stops the printing halfway

    return 0;
}
