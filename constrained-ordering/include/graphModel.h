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


// Stores a (directed or undirected) graph as an adjacency matrix, along with various methods which operate on this
// graph. Used to reason about ordering constraints and orderings on variables of a Bayesian network.
// In particular, implements the constrained MinFill heuristic for selecting a variable ordering which satisfies
// required constraints.
class GraphModel {

public:
    enum Heuristic {MIN_FILL}; // Which heuristic to use (currently only MIN_FILL)
    enum Constraint {PARTIAL_ORDER, NONE}; // What type of ordering constraints to impose

    GraphModel(){};

    // Creates copy of GraphModel passed as parameter (with its own independent adjacency matrix)
    GraphModel(const GraphModel& g);;

    GraphModel(std::vector<std::vector<bool> > adjMatrix);

    // Fills in adjacency matrix by reading from a .net Bayesian network file (directed edges)
    void readNET(std::string infile);

    // Drops the direction of edges by making every edge a bidirectional edge
    void dropDirection();

    // Moralizes the graph, that is, connects any two nodes which are parents of the same node, and then drops direction
    GraphModel moralize();

    // Wrapper for getOrdering which returns names rather than indexes
    std::vector<std::string> getOrdering(Heuristic h, Constraint c, std::map<std::string,
                                         std::vector<std::string> > constraintMap);

    // Returns an ordering of the variables (indexes) satisfying the constraints in constraintMap, by utilising the
    // heuristic h. Assumes the graph is undirected (i.e. from a Bayesian network, we have moralized the graph).
    // Input constraints are of the form <node> <parenti>, where the constraint stipulates that <node> must come
    // before <parenti>.
    std::vector<int> getOrdering(Heuristic h, Constraint c, std::map<int, std::vector<int> > constraintMap);

    // Adds topological constraints (i.e. node must appear before descendant in directed graph) to the constraints in
    // the parameter constraintMap
    std::map<std::string, std::vector<std::string> > addTopologicalConstraints(std::map<std::string,
                                                                               std::vector<std::string> > constraintMap);

private:
    // Convert back and forth between variable index (0, 1, ...) and name e.g. (Age, SocioEcon, ...)
    std::vector<std::string> idxToName;
    std::map<std::string, int> nameToIdx;

    std::vector<std::vector<bool> > adjMatrix;

    bool firstPot = true; // internal variable used to aid reading of .net files

    bool directed = true;

    void parseNETLine(std::string line);

    ////////////////////////
    // FUNCTIONS FOR CONSTRUCTING ORDERING

    // Calculates the heuristic h, as part of the algorithm for generating orderings.
    std::vector<int> calcHeuristic(Heuristic h, const std::set<int>& remainingNodeIdxs,
                                   const std::vector<std::vector<bool> >& inducedAdjMatrix);

    // Checks if putting node nodeIdx next in the ordering would violate any constraint in constraintMap.
    // A constraint is violated if it requires that some nodes appear before nodeIdx in the ordering, and they
    // are not present in the ordering yet.
    bool canRemove(Constraint c, std::map<int, std::vector<int> > constraintMap,
                   int nodeIdx, const std::set<int>& remainingNodeIdxs);

    // Updates the inducedAdjMatrix so that connections involving the removed node are removed, and also any two nodes
    // connected to the removed node are connected pairwise (this is used in the greedy algorithm).
    void removeNode(int nodeIdx, const std::set<int>& remainingNodeIdxs,
                    std::vector<std::vector<bool> >& inducedAdjMatrix);

    ///////////////////////
    // UTILITIES

    std::map<int, std::vector<int> > constraintMapToInt (std::map<std::string,
                                                         std::vector<std::string>> constraintMapString);

    std::map<std::string, std::vector<std::string> > constraintMapToString(std::map<int, std::vector<int> >
            constraintMapInt);
};


#endif //CONSTRAINED_ORDERING_GRAPHMODEL_H
