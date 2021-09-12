//
// Created by Benjie Wang on 11/10/2020.
//

#include <vector>
#include <string>
#include <map>
#include <set>
#include <numeric>
#include <fstream>
#include <iostream>
#include <sstream>

#ifndef CONSTRAINED_ORDERING_GRAPHMODEL_H
#define CONSTRAINED_ORDERING_GRAPHMODEL_H



class GraphModel {

public:
    enum Heuristic {MIN_FILL};
    enum Constraint {PARTIAL_ORDER, NONE};

    GraphModel(){};
    GraphModel(const GraphModel& g);;
    GraphModel(std::vector<std::vector<bool> > adjMatrix);
    void readNET(std::string infile);;

    void dropDirection();

    GraphModel moralize();

    std::vector<std::string> getOrdering(Heuristic h, Constraint c, std::map<std::string, std::vector<std::string> > constraintMap, std::vector<int> restrictToIdxs = std::vector<int>());

    std::vector<int> getOrdering(Heuristic h, Constraint c, std::map<int, std::vector<int> > constraintMap, std::vector<int> restrictToIdxs = std::vector<int>());

    std::map<std::string, std::vector<std::string> > addTopologicalConstraints(std::map<std::string, std::vector<std::string> > constraintMap);

private:
    std::vector<std::string> idxToName;
    std::map<std::string, int> nameToIdx;
    std::vector<std::vector<bool> > adjMatrix;

    bool firstPot = true;

    bool directed = true;

    void parseNETLine(std::string line);

    std::vector<int> calcHeuristic(Heuristic h, const std::set<int>& remainingNodeIdxs, const std::vector<std::vector<bool> >& inducedAdjMatrix, std::ofstream& fout);

    bool canRemove(Constraint c, std::map<int, std::vector<int> > constraintMap, int nodeIdx, const std::set<int>& remainingNodeIdxs);

    void removeNode(int nodeIdx, const std::set<int>& remainingNodeIdxs, std::vector<std::vector<bool> >& inducedAdjMatrix);

    ////////////

    std::map<int, std::vector<int> > constraintMapToInt (std::map<std::string, std::vector<std::string>> constraintMapString);

    std::map<std::string, std::vector<std::string> > constraintMapToString(std::map<int, std::vector<int> > constraintMapInt);
};


#endif //CONSTRAINED_ORDERING_GRAPHMODEL_H
