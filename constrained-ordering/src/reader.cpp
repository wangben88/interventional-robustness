//
// Created by Benjie Wang on 18/12/2020.
//
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>


std::map <std::string, std::vector<std::string> > readConstraints(std::string infile) {
    std::map<std::string, std::vector<std::string> > constraints;
    std::ifstream fin(infile);

    std::string line;

    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        std::string nodeName; iss >> nodeName;
        std::string contextNodeName;
        while (iss >> contextNodeName) {
            constraints[contextNodeName].push_back(nodeName);
        }
    }

    return constraints;
}

void writeConstraints(std::string outfile, std::map<std::string, std::vector<std::string>> constraints) {
    std::ofstream fout(outfile);

    std::map<std::string, std::vector<std::string> > reverseConstraints;

    for (auto constraint: constraints) {
        for (auto targetNode: constraint.second) {
            reverseConstraints[targetNode].push_back(constraint.first);
        }
    }

    for (auto reverseConstraint: reverseConstraints) {
        std::ostringstream oss;
        oss << reverseConstraint.first;
        for (auto contextNode: reverseConstraint.second) {
            oss << " " << contextNode;
        }
        oss << std::endl;
        fout << oss.str();
    }

    fout.close();
}


