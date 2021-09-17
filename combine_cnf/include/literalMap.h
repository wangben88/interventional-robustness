//
// Created by Benjie Wang on 22/09/2020.
//

#ifndef BW_OBDD_TO_CNF_LITERALMAP_H
#define BW_OBDD_TO_CNF_LITERALMAP_H

#include <vector>
#include <string>
#include <map>
#include <functional>
#include "logicNode.h"

struct IndicatorInfo{
    std::string srcVarName;
    int srcVarVal;
    double negWeight, posWeight;
};

struct ParameterInfo{
    std::string srcVarName;
    int srcVarVal;
    double negWeight, posWeight;
    long long position;

    std::vector<std::reference_wrapper<IndicatorInfo> > parentsInfo;
};


// Collects information on the mapping between source variables, AC/CNF variables and literals, in order to
// print to an lmap file (similar to how ACE does it)
// Unlike ACE, we do not guarantee that the AC parameter variables appear immediately after their corresponding
// indicator variables. Instead, we tag the parameter variables with the corresponding source variables and
// values they represent.
class Lmap {
public:
    enum AcType {ALWAYS_SUM, ALWAYS_MAX, SOMETIMES_SUM_SOMETIMES_MAX};
    enum Space {NORMAL, LOG_E};
    enum AcVarType {INDICATOR, PARAMETER, CLASSIFIER};

    Lmap(AcType type, Space mathSpace);

    void loadFromCnf (Cnf cnf, std::vector<std::string> srcVarOrdering, std::vector<AcVarType> acVarToType, std::vector<double> acVarToWeight);

    void write(std::string outfile);

private:
    AcType type;
    Space mathSpace;

    std::map<std::string, int> srcVarOrderingLookup;
    std::map<std::string, std::vector<long long> > srcVarValToIndicator;
    std::map<std::string, std::vector<long long> > srcPotPosToIndicator;
    std::vector<AcVarType> acVarToType;

    std::vector<IndicatorInfo> acVarToIndicatorInfo; // undefined for acVars which are not indicators
    std::vector<ParameterInfo> acVarToParameterInfo; //undefined for acVars which are not parameters







};

#endif //BW_OBDD_TO_CNF_LITERALMAP_H
