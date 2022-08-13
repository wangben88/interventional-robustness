#ifndef COMBINE_CNF_BUILDCNF_H
#define COMBINE_CNF_BUILDCNF_H

#endif //COMBINE_CNF_BUILDCNF_H

// If no ordering given, follow the default ordering in the CNF file.
std::pair<Cnf, Lmap> buildCombinedCnf(const std::string& bnCnfFile,
                                      const std::string& dfCnfFile = "",
                                      const std::string& constraintFile = "",
                                      const std::string& outfilePrefix = "out");

void loadBnCnf(const std::string& bnCnfFile,
               std::vector<cnfClause>& bnClauses,
               std::map<std::string, std::vector<long long> >& srcVarNameValToIndicatorNodeIndex,
               std::vector<std::string>& srcVars,
               std::vector<Lmap::AcVarType>& acVarToType,
               std::vector<double>& acVarToWeight,
               std::vector<std::string>& acVarToPriority,
               long long& maxIndicatorVarIdx
);


std::vector<cnfClause> adjustClassifierClauses(Cnf& classifierCnf,
                             std::map<std::string, std::vector<long long> >& srcVarNameValToIndicatorNodeIndex,
                             std::vector<Lmap::AcVarType>& acVarToType,
                             std::vector<double>& acVarToWeight
);

std::map<int, std::vector<int> > constructCnfConstraints(const Cnf& combinedCnf,
                                                         const std::string& constraintFile,
                                                         const long long& maxIndicatorVarIdx,
                                                         const long long& numCombinedCnfVars);