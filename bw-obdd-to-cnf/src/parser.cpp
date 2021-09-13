#include "logicNode.h"
#include <string>
#include <regex>
#include <iostream>

// parser.cpp: Contains functions for loading various structures from file

// Loads Odd, in the format given by BNC_ODD (Shih/Darwiche)
Odd loadOdd(std::string infile, int numSinks) {

    std::vector<std::pair<std::string, int> > srcVarNamesNumValues;

    std::ifstream fin(infile);

    std::string line;
    std::getline(fin, line); // Header specifying names of variables in order
    std::istringstream iss(std::regex_replace(line, std::regex{R"(\{|\[|\]|\}|,)"}, " "));
    std::string srcVarName;
    while (iss >> srcVarName) {
        srcVarNamesNumValues.push_back({srcVarName, 0});
    }
    std::cout << "TIPTOP" << std::endl;
    // first pass through main file, to figure out how many different values each variable takes
    while (std::getline(fin, line)) {

        std::istringstream iss(line);
        long long id;
        std::string chString;
        int srcVarIndex;
        int numChildren = 0;

        iss >> id;
        iss >> srcVarIndex;
        while (iss >> chString) {
            numChildren++;
        }

        srcVarNamesNumValues[srcVarIndex].second = numChildren;
    }

    srcVarNamesNumValues.push_back({"Sink", numSinks});

    // Reset for rereading, ignore first line
    fin.clear();
    fin.seekg(0);
    std::getline(fin, line);

    // Now construct the diagram
    Odd diagram(srcVarNamesNumValues);

    // Add sinks
    for (int sink = 0; sink < numSinks; sink++) {
        diagram.addNode(-sink - 1, "Sink", {}, OddNode::SINK, sink);
    }

    // Each line specifies a ODD node
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        long long id, chId;
        std::string chString;
        int srcVarIndex;
        std::vector<long long> childrenId;
        OddNode::NodeType type;

        std::cout << iss.str() << std::endl;
        iss >> id;
        iss >> srcVarIndex;
        while (iss >> chString) {
            if (chString[0] == 'S') {
                type = OddNode::NodeType::SINK;
                chString.erase(0, 1);

                int sinkNum = std::stoi(chString);
                chId = -sinkNum - 1;
            }
            else {
                chId = std::stoi(chString);
            }
            childrenId.push_back(chId);
        }

        try {
            diagram.addNode(id, srcVarNamesNumValues[srcVarIndex].first, childrenId, OddNode::NORMAL);
        }
        catch (const std::logic_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }


    return diagram;

}

