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
#include "parser.h"


void loadBnCnf(const std::string& bnCnfFile,
               std::vector<cnfClause>& bnClauses,
               std::map<std::string, std::vector<long long> >& srcVarNameValToIndicatorNodeIndex,
               std::vector<std::string>& srcVars,
               std::vector<Lmap::AcVarType>& acVarToType,
               std::vector<double>& acVarToWeight,
               std::vector<std::string>& acVarToPriority,
               long long& maxIndicatorVarIdx
)
{
    // CNF clauses for the Bayesian network

    std::ifstream fin(bnCnfFile);

    std::string line;

    std::string firstPart = "c";
    std::string secondPart;

    long long numVars; // number of CNF variables
    long long numClauses;  // number of bnClauses in the CNF
    int numSrcVars; // number of source (i.e. Bayesian network) variables

    // Read preamble of CNF file
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
    acVarToPriority = std::vector<std::string> (numVars);

    // Read in CNF clauses for the Bayesian network
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
        bnClauses.push_back(clause);
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

    maxIndicatorVarIdx = 0;
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
        srcVars.push_back(srcVarName);

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
}


std::vector<cnfClause> adjustClassifierClauses(Cnf& classifierCnf,
                             std::map<std::string, std::vector<long long> >& srcVarNameValToIndicatorNodeIndex,
                             std::vector<Lmap::AcVarType>& acVarToType,
                             std::vector<double>& acVarToWeight
                             )
{

    // The classifier CNF contains two types of variables, namely indicator variables (corresponding to values of
    // variables in the Bayesian network), and intermediate variables.
    // This step aligns the index assigned to each indicator variable to that in the Bayesian network CNF in the last
    // step, while ensuring that the intermediate variables have indexes unique from any already assigned.
    // Additionally, the metadata for the combined CNF is updated.

    std::vector<cnfClause> classifierClauses = classifierCnf.getClauses();
    std::map<std::string, std::vector<long long> > srcVarValToClassifierCnfIndex = classifierCnf.getSrcVarDetails();

    // Add the sink, i.e. predictor node to the combined CNF records.
    srcVarNameValToIndicatorNodeIndex["Sink"] = std::vector<long long> (srcVarValToClassifierCnfIndex["Sink"].size());

    // Map from classifier cnf index to combined cnf index
    std::vector<long long> classToFullIdxMap(classifierCnf.getNumCnfVars(), -1);
    long long numCombinedCnfVars = acVarToType.size();

    // Update the indices and metadata
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
                // add sink onto end
                classToFullIdxMap[indices[pos]] = numCombinedCnfVars;

                srcVarNameValToIndicatorNodeIndex["Sink"][pos] = numCombinedCnfVars;
                acVarToType.push_back(Lmap::INDICATOR); // "Sink" is an indicator for the prediction
                acVarToWeight.push_back(1.0);
                numCombinedCnfVars++;
            }
        }

    }

    for (int origIndex = 0; origIndex < classToFullIdxMap.size(); origIndex++) {
        // Not set, i.e. intermediate variables
        if (classToFullIdxMap[origIndex] == -1) {
            classToFullIdxMap[origIndex] = numCombinedCnfVars;
            acVarToType.push_back(Lmap::CLASSIFIER);
            acVarToWeight.push_back(1.0);
            numCombinedCnfVars++;
        }
    }

    for (cnfClause& classifierClause: classifierClauses) {
        classifierClause.remapLiterals(classToFullIdxMap);
    }

    return classifierClauses;
}

std::map<int, std::vector<int> > constructCnfConstraints(Cnf& combinedCnf,
                                                         const std::string& constraintFile,
                                                         const long long& maxIndicatorVarIdx,
                                                         const long long& numCombinedCnfVars)
{
    std::map<std::string, std::vector<std::string> > constraintMap;

    constraintMap = readConstraints(constraintFile);

    // Convert string-based constraints to int-based constraints
    std::map<int, std::vector<int> > combinedCnfConstraintMap = combinedCnf.constraintsToCnfConstraints(constraintMap);

    // Add constraints that all parameters/classifier variables come before indicator variables
    // Note, this does not include the indicators for the "Sink" variable (the classification is fully determined by
    // the other BN variables, so it will not correspond to any sum node in the AC).
    std::vector<int> nonIndicators;
    for (int idx = maxIndicatorVarIdx + 1; idx < numCombinedCnfVars; idx++) {
        nonIndicators.push_back(idx);
    }

    for (int idx = 0; idx < maxIndicatorVarIdx + 1; idx++) {
        combinedCnfConstraintMap[idx].insert(combinedCnfConstraintMap[idx].end(), nonIndicators.begin(), nonIndicators.end());
    }

    return combinedCnfConstraintMap;
}




std::pair<Cnf, Lmap> buildCombinedCnf(const std::string& bnCnfFile,
                                      const std::string& dfCnfFile,
                                      const std::string& constraintFile,
                                      const std::string& outfilePrefix) {

    //////////////////////////////////////////////////////////////////////////////////
    // Step 0: Define and maintain relevant information for combined CNF
    //////////////////////////////////////////////////////////////////////////////////

    // Local variables containing information ("metadata") about CNF variables

    std::map<std::string, std::vector<long long> > srcVarNameValToIndicatorNodeIndex; // Contains CNF variable index for the indicator lambda_{X = x} for src variable name X and value x
    std::vector<std::string> srcVars; // List of all BN variables

    std::vector<Lmap::AcVarType> acVarToType; // Maps CNF index to type of variable (indicator, parameter, or classifier/intermediate)
    std::vector<double> acVarToWeight; // Maps CNF index to weight
    std::vector<std::string> acVarToPriority; // for each AC var, we assign a "priority" which breaks ties when deciding on ordering.
    // This is meant primarily for ensuring that indicators for the same BN variable stay together. For indicators, the priority is
    // given by the name of the corresponding BN variable, and we use string comparison for comparing priorities.
    // For parameters or classifier/intermediate variables, we use the default string value "" as there is no need to tie break.

    long long maxIndicatorVarIdx; // maximum CNF variable index corresponding to an indicator (i.e. all higher indexes
                                  // are parameter/classifier variables).


    // Bayesian network CNF clauses + Classifier CNF clauses
    std::vector<cnfClause> bnClauses;
    std::vector<cnfClause> classifierClauses;

    ////////////////////////////////////////////////////////////////////////////////
    // Step 1: Read CNF file for Bayesian network (including metadata)
    ////////////////////////////////////////////////////////////////////////////////

    loadBnCnf(bnCnfFile,
              bnClauses,
              srcVarNameValToIndicatorNodeIndex,
              srcVars,
              acVarToType,
              acVarToWeight,
              acVarToPriority,
              maxIndicatorVarIdx);

    //////////////////////////////////////////////////////////////////////////////////
    // Step 2: Adjust classifier CNF clauses
    //////////////////////////////////////////////////////////////////////////////////

    // Add classifier clauses only if classifier is provided
    if (!dfCnfFile.empty()) {
        Cnf classifierCnf;
        classifierCnf.read(dfCnfFile);

        classifierClauses= adjustClassifierClauses(classifierCnf,
                                                   srcVarNameValToIndicatorNodeIndex,
                                                   acVarToType,
                                                   acVarToWeight);
    }
    //////////////////////////////////////////////////////////////////////////////////
    // Step 3: Combine Bayesian network CNF and classifier CNF
    //////////////////////////////////////////////////////////////////////////////////

    Cnf combinedCnf;
    long long numCombinedCnfVars = acVarToType.size();

    combinedCnf.setNumCnfVars(numCombinedCnfVars);
    for (auto clause: bnClauses) {
        combinedCnf.addClause(clause);
    }

    if (!dfCnfFile.empty()) {
        for (auto clause: classifierClauses) {
            combinedCnf.addClause(clause);
        }
        combinedCnf.setSrcVarDetails(srcVarNameValToIndicatorNodeIndex);

        acVarToPriority.resize(numCombinedCnfVars); // new size, now that we have added the classifier vars to the Cnf
                                                    // newly added variables do not need priority tie-breaks
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Step 4: Construct optimal ordering for CNF variables
    //////////////////////////////////////////////////////////////////////////////////

    // Use the constrained min-fill heuristic to obtain the optimal elimination ordering for the CNF variables
    // Then remap the indices of the CNF variables so that the the natural ordering of indices follows this elimination
    // ordering.

    GraphModel combinedCnfGraph = combinedCnf.toGraph();


    // Loads constraints from file, and adds constraints that parameters/classifier vars come before indicators.
    std::map<int, std::vector<int> > combinedCnfConstraintMap = constructCnfConstraints(combinedCnf,
                                                                                        constraintFile,
                                                                                        maxIndicatorVarIdx,
                                                                                        numCombinedCnfVars);

    // Finds heuristic optimal ordering
    std::vector<int> cnfOptimalOrdering = combinedCnfGraph.getOrdering(GraphModel::Heuristic::MIN_FILL, GraphModel::Constraint::PARTIAL_ORDER, combinedCnfConstraintMap,
                                                                       acVarToPriority);//, restrictIndicatorsOnly);

    // reverse for dt_method 3
    std::reverse(cnfOptimalOrdering.begin(), cnfOptimalOrdering.end());


    // Remaps variable indexes in CNF such that the numerical ordering reflects the optimal one.
    std::vector<int> oldOrderingToNewOrdering(numCombinedCnfVars);
    for (int newIndex = 0; newIndex < numCombinedCnfVars; newIndex++) {
        int oldIndex = cnfOptimalOrdering[newIndex];
        oldOrderingToNewOrdering[oldIndex] = newIndex;
    }
    std::vector<long long> oldOrderingToNewOrderingll(oldOrderingToNewOrdering.begin(), oldOrderingToNewOrdering.end());

    for (cnfClause& clause: combinedCnf.clauses) {
        clause.remapLiterals(oldOrderingToNewOrderingll);
    }

    // Update "metadata"
    // Change srcVarNameValToIndicatorNodeIndex according to new ordering
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

    // Update mapping to variable type and weight
    std::vector<Lmap::AcVarType> oldAcVarToType = acVarToType;
    std::vector<double> oldAcVarToWeight = acVarToWeight;
    for (int idx = 0; idx < numCombinedCnfVars; idx++) {
        // ith new variable was at cnfOptimalOrdering.at(idx) previously
        acVarToType[idx] = oldAcVarToType[cnfOptimalOrdering.at(idx)];
        acVarToWeight[idx] = oldAcVarToWeight[cnfOptimalOrdering.at(idx)];
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Step 5: Return the combined CNF (together with metadata in LMAP)
    //////////////////////////////////////////////////////////////////////////////////
    Lmap lm(Lmap::ALWAYS_SUM, Lmap::NORMAL);
    lm.loadFromCnf(combinedCnf, srcVars, acVarToType, acVarToWeight);
    lm.write(outfilePrefix + ".lmap");
    return {combinedCnf, lm};

}
