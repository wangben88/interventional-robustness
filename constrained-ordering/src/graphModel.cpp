//
// Created by Benjie Wang on 11/10/2020.
//

#include "../include/graphModel.h"
#include <fstream>
#include <sstream>
#include <algorithm>

GraphModel::GraphModel(const GraphModel &g) {
    this->adjMatrix = g.adjMatrix; // deep copy
    this->nameToIdx = g.nameToIdx;
    this->idxToName = g.idxToName;
}

GraphModel::GraphModel(std::vector<std::vector<bool>> adjMatrix) {
    this->adjMatrix = adjMatrix;
    this->directed = false;
}

void GraphModel::readNET(std::string infile) {
    std::ifstream fin(infile);

    std::string line;

    while (std::getline(fin, line)) {
        parseNETLine(line);
    }
}

void GraphModel::dropDirection() {
    for (int idx1 = 0; idx1 < adjMatrix.size(); idx1++) {
        for (int idx2 = idx1 + 1; idx2 < adjMatrix.size(); idx2++) {
            if ((adjMatrix[idx1][idx2]) || (adjMatrix[idx2][idx1])) {
                adjMatrix[idx1][idx2] = true;
                adjMatrix[idx2][idx1] = true;
            }
        }
    }
    directed = false;
}

GraphModel GraphModel::moralize() {
    GraphModel moralizedGraph(*this);
    std::cout << this->adjMatrix[1][2] << " " << this->adjMatrix[2][1] << std::endl;
    for (int destIdx = 0; destIdx < adjMatrix[0].size(); destIdx++) {
        std::vector<int> parIdxs;
        for (int candParIdx = 0; candParIdx < adjMatrix.size(); candParIdx++) {
            if (adjMatrix[candParIdx][destIdx]) {
                parIdxs.push_back(candParIdx);
            }
        }

        // join parents
        for (int i = 0; i < parIdxs.size(); i++) {
            for (int j = i + 1; j < parIdxs.size(); j++) {
                moralizedGraph.adjMatrix[parIdxs[i]][parIdxs[j]] = true;
            }
        }

    }
    moralizedGraph.dropDirection();

    return moralizedGraph;
}

std::vector<int> GraphModel::getOrdering(GraphModel::Heuristic h, GraphModel::Constraint c,
                                                 std::map<int, std::vector<int>> constraintMap,
                                                 const std::vector<std::string>& priorities)
                                                 {
    // This function gradually constructs an ordering using the constrained MinFill heuristic. It proceeds by
    // gradually adding nodes to the ordering while maintaining an undirected graph which is used to compute the
    // heuristic at each step.

    std::vector<std::vector<bool> > inducedAdjMatrix = adjMatrix; // induced graph used to compute heuristic
    std::set<int> remainingNodeIdxs; // set of nodes left to add to the ordering
    std::vector<int> ordering; // current ordering
    int treewidth = -1;

    for (int idx = 0; idx < adjMatrix.size(); idx++) {
        remainingNodeIdxs.insert(idx);
    }

    while (!remainingNodeIdxs.empty()) {
        std::vector<int> scores = calcHeuristic(h, remainingNodeIdxs, inducedAdjMatrix);

        std::vector<int> remainingNodeIdxsVec(remainingNodeIdxs.size());
        std::copy(remainingNodeIdxs.begin(), remainingNodeIdxs.end(), remainingNodeIdxsVec.begin());
        if (priorities.empty()) {
            auto comparator = [&scores](int a, int b) { return scores[a] < scores[b]; };
            std::stable_sort(remainingNodeIdxsVec.begin(), remainingNodeIdxsVec.end(), comparator);
        }
        else {
            auto comparator = [&scores, &priorities, &remainingNodeIdxsVec](int a, int b){
                return (scores[a] == scores[b])
                       ? (priorities[a] < priorities[b])
                       : (scores[a] < scores[b]);
            };
            std::stable_sort(remainingNodeIdxsVec.begin(), remainingNodeIdxsVec.end(), comparator);
        }

        bool removed = false;
        int removedIdx = -1;
        for (auto nodeIdx: remainingNodeIdxsVec) {
            if (canRemove(c, constraintMap, nodeIdx, remainingNodeIdxs)) {
                int connect = 0;
                for (int otherIdx = 0; otherIdx < inducedAdjMatrix.size(); otherIdx++) {
                    if (inducedAdjMatrix[nodeIdx][otherIdx]) {
                        connect++;
                    }
                }
                treewidth = (connect > treewidth)? connect : treewidth;

                removeNode(nodeIdx, remainingNodeIdxs, inducedAdjMatrix);
                remainingNodeIdxs.erase(nodeIdx);
                ordering.push_back(nodeIdx);

                removed = true;
                removedIdx = nodeIdx;
                break;
            }
        }

        if (!removed) {
            throw std::logic_error("STUCK: no nodes can be ordered next");
        }
    }
    return ordering;
}

std::vector<std::string> GraphModel::getOrdering(GraphModel::Heuristic h, GraphModel::Constraint c,
                                                 std::map<std::string, std::vector<std::string>> constraintMap,
                                                 const std::vector<std::string>& priorities) {
    std::vector<int> orderingInt = this->getOrdering(h, c, this->constraintMapToInt(constraintMap),
                                                     priorities);

    std::vector<std::string> orderingStr(orderingInt.size());

    for (int i = 0; i < orderingInt.size(); i++) {
        orderingStr[i] = idxToName[orderingInt[i]];
    }

    return orderingStr;
}

std::map<std::string, std::vector<std::string> >
GraphModel::addTopologicalConstraints(std::map<std::string, std::vector<std::string>> constraintMap) {
    if (!directed) {
        throw std::logic_error("ERROR: cannot add topological constraints, graph is undirected");
    }
    for (int srcIdx = 0; srcIdx < adjMatrix.size(); srcIdx++) {
        for (int destIdx = 0; destIdx < adjMatrix[0].size(); destIdx++) {
            if (adjMatrix[srcIdx][destIdx]) {
                constraintMap[idxToName[srcIdx]].push_back(idxToName[destIdx]);
            }
        }
    }
    return constraintMap;
}

void GraphModel::parseNETLine(std::string line) {
    std::istringstream iss(line);
    std::string type; iss >> type;
    if (type == "node") {
        std::string nodeName;
        iss >> nodeName;
        nameToIdx[nodeName] = idxToName.size();
        idxToName.push_back(nodeName);
    }
    else if (type == "potential") {
        if (firstPot) {
            adjMatrix = std::vector<std::vector<bool> > (idxToName.size(), std::vector<bool> (idxToName.size()));
            firstPot = false;
        }

        int chIdx;
        std::string token;

        iss >> token; // opening {
        iss >> token; // child node name
        chIdx = nameToIdx[token];
        iss >> token; // separator |
        iss >> token; // first parent node name
        while (token != ")") {
            int parIdx = nameToIdx[token];
            adjMatrix[parIdx][chIdx] = true;

            iss >> token;
        }
    }

}

std::vector<int> GraphModel::calcHeuristic(GraphModel::Heuristic h, const std::set<int> &remainingNodeIdxs,
                                           const std::vector<std::vector<bool>> &inducedAdjMatrix) {
    std::vector<int> scores(inducedAdjMatrix.size());
    if (h == Heuristic::MIN_FILL) {
        for (int nodeIdx: remainingNodeIdxs) {
            std::vector<int> neighbourIdxs;
            for (int otherIdx = 0; otherIdx < inducedAdjMatrix.size(); otherIdx++) {
                if (inducedAdjMatrix[nodeIdx][otherIdx]) {
                    neighbourIdxs.push_back(otherIdx);
                }
            }

            int fillCount = 0;
            for (int i = 0; i < neighbourIdxs.size(); i++) {
                for (int j = i + 1; j < neighbourIdxs.size(); j++) {
                    if (!inducedAdjMatrix[neighbourIdxs[i]][neighbourIdxs[j]]) {
                        fillCount++;
                    }
                }
            }
            scores[nodeIdx] = fillCount;
        }

    }
    return scores;
}

bool GraphModel::canRemove(GraphModel::Constraint c, std::map<int, std::vector<int>> constraintMap,
                           int nodeIdx, const std::set<int> &remainingNodeIdxs) {
    if (c == NONE) {
        return true;
    }
    for (int beforeNode: constraintMap[nodeIdx]) {
        if (remainingNodeIdxs.find(beforeNode) != remainingNodeIdxs.end()) {
            return false;
        }
    }
    return true;
}

void GraphModel::removeNode(int nodeIdx, const std::set<int> &remainingNodeIdxs,
                            std::vector<std::vector<bool>> &inducedAdjMatrix) {
    std::vector<int> neighbourIdxs;
    for (int otherIdx = 0; otherIdx < inducedAdjMatrix.size(); otherIdx++) {
        if (inducedAdjMatrix[nodeIdx][otherIdx]) {
            neighbourIdxs.push_back(otherIdx);
            inducedAdjMatrix[nodeIdx][otherIdx] = false;
            inducedAdjMatrix[otherIdx][nodeIdx] = false;
        }
    }

    for (int i = 0; i < neighbourIdxs.size(); i++) {
        for (int j = i + 1; j < neighbourIdxs.size(); j++) {
            inducedAdjMatrix[neighbourIdxs[i]][neighbourIdxs[j]] = true;
            inducedAdjMatrix[neighbourIdxs[j]][neighbourIdxs[i]] = true;
        }
    }
}

std::map<int, std::vector<int> >
GraphModel::constraintMapToInt(std::map<std::string, std::vector<std::string>> constraintMapString) {
    std::map<int, std::vector<int> > constraintMapInt;
    for (auto const& constraintString: constraintMapString) {
        std::vector<std::string> beforeStrings = constraintString.second;
        std::vector<int> beforeInts(beforeStrings.size());
        for (int i = 0; i < beforeStrings.size(); i++) {
            beforeInts[i] = nameToIdx[beforeStrings[i]];
        }

        constraintMapInt[nameToIdx[constraintString.first]] = beforeInts;
    }

    return constraintMapInt;
}

std::map<std::string, std::vector<std::string> >
GraphModel::constraintMapToString(std::map<int, std::vector<int>> constraintMapInt) {
    std::map<std::string, std::vector<std::string> > constraintMapString;
    for (auto const& constraintInt: constraintMapInt) {
        std::vector<int> beforeInts = constraintInt.second;
        std::vector<std::string> beforeStrings(beforeInts.size());
        for (int i = 0; i < beforeInts.size(); i++) {
            beforeStrings[i] = idxToName[beforeInts[i]];
        }

        constraintMapString[idxToName[constraintInt.first]] = beforeStrings;
    }

    return constraintMapString;
}
