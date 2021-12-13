#include "../include/literalMap.h"
#include <algorithm>


Lmap::Lmap(Lmap::AcType type, Lmap::Space mathSpace) {
    this->type = type;
    this->mathSpace = mathSpace;
}

void Lmap::loadFromCnf(Cnf cnf, std::vector <std::string> srcVarOrdering, std::vector <Lmap::AcVarType> acVarToType,
                       std::vector<double> acVarToWeight) {
    this->srcVarValToIndicator = cnf.getSrcVarDetails();
    // Note: ordering doesn't really matter, except for providing a unique way of specifying parameter positions
    // Any consistent order here will do (i.e. doesn't matter that there isn't an explicit "order" when we do the
    // combined CNF constraints approach, we can use any order).
    for (int i = 0; i < srcVarOrdering.size(); i++) {
        this->srcVarOrderingLookup[srcVarOrdering[i]] = i;
    }

    this->acVarToType = acVarToType;
    long long numAcVars = cnf.getNumCnfVars();
    acVarToIndicatorInfo = std::vector<IndicatorInfo> (numAcVars);
    acVarToParameterInfo = std::vector<ParameterInfo> (numAcVars);

    for (const auto& pr: srcVarValToIndicator) {
        std::string srcVarName = pr.first;
        for (int val = 0; val < pr.second.size(); val++) {
            int acVar = pr.second[val];
            IndicatorInfo& info = acVarToIndicatorInfo[acVar];
            info.srcVarName = srcVarName;
            info.srcVarVal = val;
            info.negWeight = 1.0;
            info.posWeight = acVarToWeight[acVar];
        }
    }

    std::vector<cnfClause> clauses = cnf.getClauses();
    for (auto clause: clauses) {
        std::vector<std::pair<long long, bool> > literals = clause.getLiterals();
        // Assume that in parameter clauses, the parameter comes last, and the indicator
        // corresponding to the target variable comes second last.
        bool parameterClause = (acVarToType[literals[literals.size() - 1].first] == AcVarType::PARAMETER);

        if (parameterClause) {
            long long acVar = literals[literals.size() - 1].first;
            ParameterInfo& info = acVarToParameterInfo[acVar];
            long long targetIndAcVar = literals[literals.size() - 2].first;

            info.srcVarName = acVarToIndicatorInfo[targetIndAcVar].srcVarName;
            info.srcVarVal = acVarToIndicatorInfo[targetIndAcVar].srcVarVal;
            info.negWeight = 1.0;
            info.posWeight = acVarToWeight[acVar];
            int targetNumValues = srcVarValToIndicator[info.srcVarName].size();

            // Vector of parents
            // first part of tuple is the ordering of the corresponding source variable
            // second part of tuple is the number of values that source variable can take
            // third part of tuple is the value taken by the indicator
            std::vector<std::tuple<int, int, int> > parentsDetails;
            for (int i = 0; i < literals.size() - 2; i++) {
                IndicatorInfo& parentInfo = acVarToIndicatorInfo[literals[i].first];
                info.parentsInfo.push_back(parentInfo);

                std::tuple<int, int, int> parentDetails;
                std::get<0>(parentDetails) = srcVarOrderingLookup[parentInfo.srcVarName];
                std::get<1>(parentDetails) = srcVarValToIndicator[parentInfo.srcVarName].size();
                std::get<2>(parentDetails) = parentInfo.srcVarVal;

                parentsDetails.push_back(parentDetails);
            }

            std::sort(parentsDetails.begin(), parentsDetails.end()); // sorts by ordering, in asc order
            // this allows us to compute a "position" for each parameter, i.e. a unique
            // index for this parameter among all parameters in this CPT
            long long position = 0;
            long long perUnit = 1;

            for (int i = parentsDetails.size() - 1; i > -1; i--) {
                position += std::get<2>(parentsDetails[i]) * perUnit;
                perUnit *= std::get<1>(parentsDetails[i]);
            }

            position += info.srcVarVal * perUnit;
            perUnit *= targetNumValues;

            if (srcPotPosToIndicator.find(info.srcVarName) == srcPotPosToIndicator.end()) {

                srcPotPosToIndicator[info.srcVarName] = std::vector<long long> (perUnit);
            }
            srcPotPosToIndicator[info.srcVarName][position] = acVar;

            info.position = position;
        }
    }

}

void Lmap::write(std::string outfile) {
    std::ofstream fout(outfile);
    fout << "c Following is the literal map:" << std::endl;
    fout << "c" << std::endl;

    fout << "cc$K$";
    switch (type) {
        case ALWAYS_SUM:
            fout << "ALWAYS_SUM";
            break;
        case ALWAYS_MAX:
            fout << "ALWAYS_MAX";
            break;
        case SOMETIMES_SUM_SOMETIMES_MAX:
            fout << "SOMETIMES_SUM_SOMETIMES_MAX";
            break;
    }
    fout << std::endl;

    fout << "cc$S$";
    switch (mathSpace) {
        case NORMAL:
            fout << "NORMAL";
            break;
        case LOG_E:
            fout << "LOG_E";
            break;
    }
    fout << std::endl;

    fout << "cc$N$" << acVarToType.size() << std::endl;

    fout << "cc$v$" << srcVarValToIndicator.size() << std::endl;

    for (const auto& pr: srcVarValToIndicator) {
        fout << "cc$V$" << pr.first << "$" << pr.second.size() << std::endl;
    }

    fout << "cc$t$" << srcPotPosToIndicator.size() << std::endl;

    for (const auto& pr: srcPotPosToIndicator) {
        fout << "cc$T$" << pr.first << "$" << pr.second.size() << std::endl;
    }

    for (long long acVar = 0; acVar < acVarToType.size(); acVar++) {
        // we move from 0 to 1-indexing
        switch (acVarToType[acVar]) {
            case INDICATOR:
            {
                double negWeight = acVarToIndicatorInfo[acVar].negWeight;
                double posWeight = acVarToIndicatorInfo[acVar].posWeight;
                std::string srcVarName = acVarToIndicatorInfo[acVar].srcVarName;
                int srcVarVal = acVarToIndicatorInfo[acVar].srcVarVal;
                fout << "cc$C$" << -(acVar + 1) << "$" << negWeight << "$+$" << std::endl;
                fout << "cc$I$" << acVar + 1 << "$" << posWeight << "$+$" << srcVarName << "$" << srcVarVal << std::endl;
            }
                break;
            case PARAMETER:
            {
                double negWeight = acVarToParameterInfo.at(acVar).negWeight;
                double posWeight = acVarToParameterInfo[acVar].posWeight;
                std::string srcVarName = acVarToParameterInfo[acVar].srcVarName;
                int srcVarVal = acVarToParameterInfo[acVar].srcVarVal;
                long long potPos = acVarToParameterInfo[acVar].position;

                fout << "cc$C$" << -(acVar + 1) << "$" << negWeight << "$+$" << std::endl;
                fout << "cc$P$" << acVar + 1 << "$" << posWeight << "$+$" << srcVarName << "$" << potPos;

                fout << "$" << acVarToParameterInfo[acVar].parentsInfo.size() + 1;
                fout << "$" << srcVarName << "$" << srcVarVal;
                for (const auto& info: acVarToParameterInfo[acVar].parentsInfo) {
                    fout << "$" << info.get().srcVarName << "$" << info.get().srcVarVal;
                }
                fout << std::endl;

            }
                break;
            case CLASSIFIER:
            {
                fout << "cc$C$" << -(acVar + 1) << "$" << 1.0 << "$+$" << std::endl;
                fout << "cc$C$" << acVar + 1 << "$" << 1.0 << "$+$" << std::endl;
            }
                break;
        }
    }

    fout.close();
}
