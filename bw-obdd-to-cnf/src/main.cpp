#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "utils.h"
#include "parser.h"
#include "logicNode.h"

void help(){
    std::cerr << "\nUsage:\n   ./bw_obdd_to_cnf -i <filename> \n\n";
    std::cerr << "   Options:\n";
    std::cerr << "      -i <filename>: Input (.odd file)\n";
    std::cerr << "      -o <filename>: Output filename for CNF representation (.txt file)\n";
    std::cerr << "      -s <sinks>: Number of sinks (i.e. number of classifier outcomes), default 2\n";
    std::cerr << "      -h: Help\n";
}

int main(int argc, char **argv){
    int c;
    std::string infile;
    std::string bnCnfFile;
    std::string ordFile;
    std::string outfile;
    std::string constraintFile;
    int sinks = 2; // default 2 sinks

    while ((c = getopt(argc, argv, "i:o:")) != -1){
        switch (c){
            case 'i': // provide input
            {
                infile = optarg;
                std::string ext = get_filename_ext(infile.c_str());
                if (ext != "odd") {
                    std::cerr << "Unknown file extension, a '*.odd' file is required\n";
                    return 1;
                }
            }
                break;
            case 'o':
            {
                outfile = optarg;
                std::string ext = get_filename_ext(outfile.c_str());
            }
                break;
            case 's':
                sinks = std::stoi(optarg);
                break;
            default:
                help();
                return 1;
        }
    }

    for (int index = optind; index < argc; index++)
        std::cout << "Non-option argument " << argv[index] << std::endl;

    if (infile.empty()) {
        help();
        std::cerr << "Specify input file with -i <filename>\n";
        return 1;
    }
    // Load Odd

    Odd diagram = loadOdd(infile, sinks);
    std::cout << "READIN" << std::endl;
    auto srcVarNamesNumValues = diagram.getSrcVariableDetails();
    Nnf nnfdiag(diagram.getSrcVariableDetails());
    nnfdiag.loadFromOdd(diagram);
    nnfdiag.dump();
    Cnf form(nnfdiag.getSize());
    form.encodeNNF(nnfdiag);
    std::cout << "Classifier CNF numvars: " << form.getNumCnfVars() << std::endl;
    form.write(outfile);


    return 0;
}
