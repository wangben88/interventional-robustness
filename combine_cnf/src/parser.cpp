#include "../include/parser.h"
#include <string>
#include <sstream>
#include <fstream>
#include <regex>
#include <iostream>
#include <cmath>
#include "../include/literalMap.h"
#include "reader.h"
#include "graphModel.h"
#include "logicNode.h"

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

// Given
std::pair<Cnf, Lmap> loadCnfSpecial(std::string infile, Cnf classifierCnf, std::string constraintfile, std::string outfileprefix) {

    std::map<std::string, std::vector<long long> > srcVarNameValToIndicatorNodeIndex; // contains CNF variable index for the indicator lambda_{X = x} for src variable name X and value x
    std::vector<std::string> ordering; // according to orderingfile, the desired ordering of CNF indices for the source varaibles

    std::vector<Lmap::AcVarType> acVarToType;
    std::vector<double> acVarToWeight;
    // 0 -indexed, so 1 less than you see in the file
    // previously, I hardcoded this: now it is read automatically
    /*
    srcVarNameValToIndicatorNodeIndex["Admit"] = {0, 1};
    srcVarNameValToIndicatorNodeIndex["EntranceExam"] = {2, 3};
    srcVarNameValToIndicatorNodeIndex["FirstTimeApplicant"] = {4, 5};
    srcVarNameValToIndicatorNodeIndex["GPA"] = {6, 7};
    srcVarNameValToIndicatorNodeIndex["WorkExperience"] = {8, 9};
    */
    /*
    srcVarNameValToIndicatorNodeIndex["Age"] = {0, 1, 2};
    srcVarNameValToIndicatorNodeIndex["BirthAsphyxia"] = {3, 4};
    srcVarNameValToIndicatorNodeIndex["CO2"] = {5, 6, 7};
    srcVarNameValToIndicatorNodeIndex["CO2Report"] = {8, 9};
    srcVarNameValToIndicatorNodeIndex["CardiacMixing"] = {10, 11, 12, 13};
    srcVarNameValToIndicatorNodeIndex["ChestXray"] = {14, 15, 16, 17, 18};
    srcVarNameValToIndicatorNodeIndex["Disease"] = {19, 20, 21, 22, 23, 24};
    srcVarNameValToIndicatorNodeIndex["DuctFlow"] = {25, 26, 27};
    srcVarNameValToIndicatorNodeIndex["Grunting"] = {28, 29};
    srcVarNameValToIndicatorNodeIndex["GruntingReport"] = {30, 31};
    srcVarNameValToIndicatorNodeIndex["HypDistrib"] = {32, 33};
    srcVarNameValToIndicatorNodeIndex["HypoxiaInO2"] = {34, 35, 36};
    srcVarNameValToIndicatorNodeIndex["LVH"] = {37, 38};
    srcVarNameValToIndicatorNodeIndex["LVHreport"] = {39, 40};
    srcVarNameValToIndicatorNodeIndex["LowerBodyO2"] = {41, 42, 43};
    srcVarNameValToIndicatorNodeIndex["LungFlow"] = {44, 45, 46};
    srcVarNameValToIndicatorNodeIndex["LungParench"] = {47, 48, 49};
    srcVarNameValToIndicatorNodeIndex["RUQO2"] = {50, 51, 52};
    srcVarNameValToIndicatorNodeIndex["Sick"] = {53, 54};
    srcVarNameValToIndicatorNodeIndex["XrayReport"] = {55, 56, 57, 58, 59};
    */

    // Read CNF file
    std::ifstream fin(infile);

    std::string line;

    std::string firstPart = "c";
    std::string secondPart;

    long long numVars;
    long long numClauses;
    int numSrcVars;

    while (firstPart == "c") {
        std::getline(fin, line);
        std::istringstream iss(line);
        iss >> firstPart;
        iss >> secondPart;

        if (secondPart == "Variables") {
            iss >> secondPart; // colon
            iss >> numSrcVars;
        }


        // reached start of CNF
        if (firstPart == "p") {
            iss >> numVars;
            iss >> numClauses;
        }
    }

    acVarToType = std::vector<Lmap::AcVarType> (numVars, Lmap::PARAMETER); // we will loop over the AC vars which are indicators later, to edit this
    acVarToWeight = std::vector<double> (numVars);
    std::vector<std::string> acVarToPriority = std::vector<std::string> (numVars); // for each AC var, we assign a priority which breaks ties when deciding on ordering. This is meant
    // primarily for ensuring that indicators for the same BN variable stay together. we use string comparison
    // all parameters are given "" as default value as we don't need to specify tie breaks

    std::vector<cnfClause> clauses; // stores CNF clauses describing the Bayesian Network

    // Read in CNF clauses
    for (int i = 0; i < numClauses; i++) {
        std::getline(fin, line);
        std::istringstream iss(line);
        int literal;
        cnfClause clause;
        while (iss >> literal) {

            if (literal == 0) {
                break;
            }
            bool positive = literal > 0;
            literal = positive ? (literal - 1) : (literal + 1);
            clause.addLiteral(abs(literal), positive);
        }
        clauses.push_back(clause);
    }

    // Read weights
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        std::string part;
        iss >> part; // "c"
        iss >> part;
        if (part == "literal-to-real-weight") {
            break;
        }
    }

    while (true) {
        std::getline(fin, line);
        std::istringstream iss(line);
        std::string part;
        iss >> part; // "c"
        iss >> part; // which ac vars
        int low, high;
        if (part.find("-") != std::string::npos) {
            int delimPos = part.find("-");
            low = std::stoi(part.substr(0, delimPos)) - 1;
            high = std::stoi(part.substr(delimPos + 1, part.size() - (delimPos + 1))) - 1;
        }
        else {
            low = std::stoi(part) - 1;
            high = low;
        }
        iss >> part; //":"
        iss >> part;
        double weight = std::stod(part);

        for (int acVar = low; acVar < high + 1; acVar++) {
            acVarToWeight[acVar] = weight;
        }

        if (high == numVars - 1) {
            break;
        }
    }

    // Read details about mapping from source variable names, to the variable indices in CNF

    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        std::string part;
        iss >> part; // "c"
        iss >> part;
        if (part == "variable-and-values-to-names") {
            // skip the next 4 useless lines
            for (int i = 0; i < 4; i++) {
                std::getline(fin, line);
            }
            break;
        }
    }

    int maxIndicatorVarIdx = 0;
    for (int srcVarIdx = 0; srcVarIdx < numSrcVars; srcVarIdx++) {
        std::getline(fin, line);
        std::istringstream iss(line);
        std::string part;
        int numValues; // how many different values this variable can take
        std::string srcVarName;


        iss >> part; // "c"
        iss >> part; // variable index
        iss >> numValues;
        iss >> srcVarName;
        srcVarName.erase(remove(srcVarName.begin(), srcVarName.end(), '\"'), srcVarName.end());

        // fill ordering with the default ordering
        ordering.push_back(srcVarName);

        std::vector<long long> indices(numValues);

        for (int value = 0; value < numValues; value++) {
            std::getline(fin, line);
            std::istringstream iss_val(line);
            iss_val >> part; // "c"
            iss_val >> indices[value];
            indices[value]--; // 1-indexing to 0-indexing
            acVarToType[indices[value]] = Lmap::INDICATOR;
            if (indices[value] > maxIndicatorVarIdx) {
                maxIndicatorVarIdx = indices[value];
            }
            acVarToPriority[indices[value]] = srcVarName;
        }
        srcVarNameValToIndicatorNodeIndex[srcVarName] = indices;
    }

    fin.close();


    // Adjust classifier clauses
    std::vector<cnfClause> classifierClauses = classifierCnf.getClauses();
    std::map<std::string, std::vector<long long> > srcVarValToClassifierCnfIndex = classifierCnf.getSrcVarDetails();

    srcVarNameValToIndicatorNodeIndex["Sink"] = std::vector<long long> (srcVarValToClassifierCnfIndex["Sink"].size());

    std::vector<long long> classToFullIdxMap(classifierCnf.getNumCnfVars(), -1); // classifier cnf index to full cnf index

    for (auto pr: srcVarValToClassifierCnfIndex) {
        std::string srcVarName = pr.first;
        std::vector<long long> indices = pr.second;
        if (srcVarName != "Sink") {
            for (int pos = 0; pos < indices.size(); pos++) {
                classToFullIdxMap[indices[pos]] = srcVarNameValToIndicatorNodeIndex[srcVarName][pos];
            }
        }
        else {
            for (int pos = 0; pos < indices.size(); pos++) {
                // add onto end
                classToFullIdxMap[indices[pos]] = numVars;

                srcVarNameValToIndicatorNodeIndex["Sink"][pos] = numVars;
                acVarToType.push_back(Lmap::INDICATOR); // "Sink" is an indicator for the prediction
                acVarToWeight.push_back(1.0);
                numVars++;
            }
        }

    }

    for (int origIndex = 0; origIndex < classToFullIdxMap.size(); origIndex++) {
        // Not set, i.e. intermediate variables
        if (classToFullIdxMap[origIndex] == -1) {
            classToFullIdxMap[origIndex] = numVars;
            acVarToType.push_back(Lmap::CLASSIFIER);
            acVarToWeight.push_back(1.0);
            numVars++;
        }
    }

    for (cnfClause& classifierClause: classifierClauses) {
        classifierClause.remapLiterals(classToFullIdxMap);
        //std::cout << idxMap[4] << std::endl;
    }

    numClauses += classifierClauses.size();

    // Combine
    Cnf combinedCnf;
    combinedCnf.setNumCnfVars(numVars);
    for (auto clause: clauses) {
        combinedCnf.addClause(clause);
    }
    for (auto clause: classifierClauses) {
        combinedCnf.addClause(clause);
    }
    combinedCnf.setSrcVarDetails(srcVarNameValToIndicatorNodeIndex);

    acVarToPriority.resize(numVars); // new size, now that we have added the classifier vars to the Cnf


    // start reconstructing ordering
    GraphModel combinedCnfGraph = combinedCnf.toGraph();
    std::map<std::string, std::vector<std::string> > constraintMap;

    constraintMap = readConstraints(constraintfile);

    // Convert string-based constraints to int-based constraints
    std::map<int, std::vector<int> > combinedCnfConstraintMap = combinedCnf.constraintsToCnfConstraints(constraintMap);

    // Add constraints that all parameters/classifier indicators come before variable indicators
    std::vector<int> nonIndicators;
    for (int idx = maxIndicatorVarIdx + 1; idx < numVars; idx++) {
        nonIndicators.push_back(idx);
    }

    for (int idx = 0; idx < maxIndicatorVarIdx + 1; idx++) {
        combinedCnfConstraintMap[idx].insert(combinedCnfConstraintMap[idx].end(), nonIndicators.begin(), nonIndicators.end());
    }

    // Only work out order for indicators and not parameters (excluding Sink indicator also)
    // Note: now working out ordering for all ac vars
    //std::vector<int> restrictIndicatorsOnly(maxIndicatorVarIdx + 1);
    //std::iota(restrictIndicatorsOnly.begin(), restrictIndicatorsOnly.end(), 0);

    std::vector<int> cnfOptimalOrdering = combinedCnfGraph.getOrdering(GraphModel::Heuristic::MIN_FILL, GraphModel::Constraint::PARTIAL_ORDER, combinedCnfConstraintMap,
                                                                       acVarToPriority);//, restrictIndicatorsOnly);
    // reverse for dt_method 3

    std::reverse(cnfOptimalOrdering.begin(), cnfOptimalOrdering.end());

    // keep the ordering of other ac vars (parameters, classifier intermediates, sink) the same
    // note: now unncessary
    //for (int otherIndex = maxIndicatorVarIdx + 1; otherIndex < numVars; otherIndex++) {
    //    cnfOptimalOrdering.push_back(otherIndex);
    //}

    // this maps from old index to new index
    std::vector<int> oldOrderingToNewOrdering(numVars);
    for (int newIndex = 0; newIndex < numVars; newIndex++) {
        int oldIndex = cnfOptimalOrdering[newIndex];
        oldOrderingToNewOrdering[oldIndex] = newIndex;
    }
    std::vector<long long> oldOrderingToNewOrderingll(oldOrderingToNewOrdering.begin(), oldOrderingToNewOrdering.end());

    // remaps according to new ordering
    for (cnfClause& clause: combinedCnf.clauses) {
        clause.remapLiterals(oldOrderingToNewOrderingll);
        //std::cout << idxMap[4] << std::endl;
    }

    // Fix ac var details
    //------------------------------------------------
    // change srcVarNameValToIndicatorNodeIndex according to new ordering
    for (auto& bnVar: srcVarNameValToIndicatorNodeIndex) {
        // Keeps "Sink" the same automatically
        for (auto &index: bnVar.second) {
            try {
                index = oldOrderingToNewOrdering.at(index);
            }
            catch (const std::out_of_range &oor) {
                std::cerr << "Error: Some indicator has not been ordered" << std::endl;
            }
        }
    }
    combinedCnf.setSrcVarDetails(srcVarNameValToIndicatorNodeIndex);

    // Copy old versions temporarily
    std::vector<Lmap::AcVarType> oldAcVarToType = acVarToType;
    std::vector<double> oldAcVarToWeight = acVarToWeight;
    for (int idx = 0; idx < numVars; idx++) {
        // ith new variable was at cnfOptimalOrdering.at(idx) previously
        acVarToType[idx] = oldAcVarToType[cnfOptimalOrdering.at(idx)];
        acVarToWeight[idx] = oldAcVarToWeight[cnfOptimalOrdering.at(idx)];
    }

    // find best ordering with constraints


    Lmap lm(Lmap::ALWAYS_SUM, Lmap::NORMAL);
    lm.loadFromCnf(combinedCnf, ordering, acVarToType, acVarToWeight);
    lm.write(outfileprefix + ".lmap");
    return {combinedCnf, lm};

}


