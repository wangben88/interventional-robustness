#ifndef BW_OBDD_TO_CNF_LOGICNODE_H
#define BW_OBDD_TO_CNF_LOGICNODE_H

#include <stdexcept>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "graphModel.h"

class OddNode {

    public:
        enum NodeType { NORMAL, SINK };
        OddNode(long long id, std::string srcVarName, std::vector<long long> children, NodeType type, int sink_num = 0);;

        void setId(long long id);

        long long getId();

        void setSinkNum(int sink_num);

        int getSinkNum();


        void setChild(int val,long long childId);

        long long getChild(int index);

        int getCardinality();

        NodeType getType();

        std::string getSrcVarName();

    private:
        std::vector<long long> children; // ids
        std::string srcVarName;
        NodeType type;
        long long id;
        int sink_num;

};

class Odd {
    private:
        std::vector<OddNode> nodes; // reverse topological order
        std::map<long long, long long> idToIndex;

        std::map<std::string, int> srcVarNameToSrcVarIndex;
        std::vector<std::pair<std::string, int> > srcVarNameNumValues; // pairs (srcVarName, numValues) - binary, etc.

        int root;
    public:
        Odd(std::vector<std::pair<std::string, int> > srcVarNamesNumValues);

        OddNode addNode(long long id, std::string srcVarName, std::vector<long long> childrenId, OddNode::NodeType type, int sink_num = 0);

        long long getSize();;

        OddNode getRoot();

        std::vector<std::pair<std::string, int> > getSrcVariableDetails();

        void dump();

        // Reverse topological order
        std::vector<OddNode> getNodes() const;
};

// TODO: Combine this with Odd class? Since they share many similarities.
class NnfNode {
public:
    enum NodeType { CONJ, DISJ, LEAF };
    NnfNode(long long idx, std::vector<int> childrenIdxs, NodeType type, std::string srcVarName = "", int srcVarVal = 0);

    void setChild(int val, int childIdx);

    int getChild(int chIndex);;

    long long getIndex();

    int getCardinality();

    NodeType getType();

    std::string getSrcVarName();

    int getSrcVarVal();

private:
    long long idx; // this identifies the Nnfnode's position in say Nnf class.
    std::vector<int> children;
    std::string srcVarName;
    int srcVarVal;
    NodeType type;
};


class Nnf {
public:
    Nnf(std::vector<std::pair<std::string, int> > srcVarNamesNumValues);

    NnfNode addNode(std::vector<long long> childrenId, NnfNode::NodeType type, std::string srcVarName = "", int srcVarVal = 0);;

    long long getSize();

    // Assumes NNF is empty to start
    void loadFromOdd(const Odd& diagram);;

    void dump();

    NnfNode getRoot() const;

    std::vector<NnfNode> getNodes() const;

    std::map<std::string, std::vector<long long> > getSrcVariableMapping() const;


private:
    std::vector<NnfNode> nodes; // reverse topological order
    std::map<std::string, std::vector<long long> > srcVarNameValToIndicatorNodeIndex;
};


class cnfClause {
public:
    cnfClause() = default;
    cnfClause(const cnfClause& a) {
        this->literals = a.getLiterals();
    }
    void addLiteral(long long idx, bool positive);
    void remapLiterals(std::vector<long long> idxMap) {
        try {
            for (auto& literal: literals) {
                literal.first = idxMap.at(literal.first);
            }
        }
        catch (const std::out_of_range& oor) {
            std::cerr << "Contains some literal which is not in the provided idxMap";
            //std::cerr << "P ";
            //for (auto& literal: literals) {
            //    std::cerr << literal.first << " ";
            //}
            //std::cerr << std::endl;
        }
    };
    std::string asString();
    std::vector<std::pair<long long, bool> > getLiterals() const;

private:
    std::vector<std::pair<long long, bool> > literals;
};

class Cnf {
public:
    Cnf();

    void write(std::string outfile);;

    void read(std::string infile);;

    void addClause(cnfClause cl);

    void encodeNNF(Nnf nnfDiagram);;

    void encodeNNF2(Nnf nnfDiagram);

    // Next two are temporary getters, ideally we should want to perform the cnf merging inside the class.
    std::vector<cnfClause> getClauses() const {
        return clauses;
    };

    std::map<std::string, std::vector<long long> > getSrcVarDetails() {
        return srcVarNameValToIndicatorNodeIndex;
    }

    void setSrcVarDetails(std::map<std::string, std::vector<long long> > det) {
        this->srcVarNameValToIndicatorNodeIndex = det;
    }

    long long getNumCnfVars() {
        return numCnfVars;
    }

    void setNumCnfVars(long long numVars) {
        this->numCnfVars = numVars;
    }

    ///////////////////

    GraphModel toGraph();

    std::map<int, std::vector<int> > constraintsToCnfConstraints(std::map<std::string, std::vector<std::string> > constraintMap);

    std::vector<cnfClause> clauses;
private:

    long long numCnfVars;

    // Can only be used once encoded from NNF as this map is just copied over.
    std::map<std::string, std::vector<long long> > srcVarNameValToIndicatorNodeIndex;

    void addHeadlessDisjunction(std::vector<long long> indices);

    void addHeadlessXOR(std::vector<long long> indices);

    void addDisjunction(long long parentIndex, std::vector<long long> chIndices);

    void addConjunction(long long parentIndex, std::vector<long long> chIndices);

    std::vector<cnfClause> disjoinCnf(std::vector<cnfClause> cnf1, std::vector<cnfClause> cnf2);

    std::vector<cnfClause> conjoinCnf(std::vector<cnfClause> cnf1, std::vector<cnfClause> cnf2);
};


#endif
