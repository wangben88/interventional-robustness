#include <logicNode.h>
#include <fstream>

OddNode::OddNode(long long id, std::string srcVarName, std::vector <long long> children, OddNode::NodeType type, int sink_num) {
    this->id = id;
    this->srcVarName = srcVarName;
    this->children = children;
    this->type = type;
    this->sink_num = sink_num;
}

void OddNode::setId(long long id) {
    this->id = id;
}

long long OddNode::getId() {
    return this->id;
}

void OddNode::setSinkNum(int sink_num) {
    this->sink_num = sink_num;
}

int OddNode::getSinkNum() {
    return this->sink_num;
}

void OddNode::setChild(int val, long long childId) {
    this->children[val] = childId;
}

long long OddNode::getChild(int index) {
    return this->children[index];
}

int OddNode::getCardinality() {
    return this->children.size();
}

OddNode::NodeType OddNode::getType() {
    return this->type;
}

std::string OddNode::getSrcVarName() {
    return srcVarName;
}

OddNode Odd::addNode(long long int id, std::string srcVarName, std::vector<long long> childrenId, OddNode::NodeType type, int sink_num) {
    std::vector<long long> children;
    for (auto childId: childrenId) {
        if (idToIndex.count(childId) > 0) {
            children.push_back(childId);
        }
        else {
            throw std::logic_error("Some child node does not exist in the ODD yet");
        }
    }
    //if (childrenId.size() > 0) {
    //    std::cout << (*children[0]).getSrcVarName() << std::endl;
    //}
    OddNode newNode(id, srcVarName, children, type, sink_num);
    idToIndex[id] = nodes.size();
    if (id == 0) {
        root = nodes.size();
    }
    nodes.push_back(newNode);
    return newNode;
}

long long Odd::getSize() {
    return nodes.size();
}

OddNode Odd::getRoot() {
    if (root == -1) {
        throw std::logic_error("Root node of ODD has not been added yet");
    }
    return nodes[root];
}

void Odd::dump() {
    for (auto node: nodes) {
        std::cout << node.getSrcVarName() << " ";
        for (int i = 0; i < node.getCardinality(); i++) {
            std::cout << nodes[idToIndex[node.getChild(i)]].getSrcVarName() << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<OddNode> Odd::getNodes() const {
    return nodes;
}

Odd::Odd(std::vector<std::pair<std::string, int>> srcVarNamesNumValues) {
    this->srcVarNameNumValues = srcVarNamesNumValues;
    for (int srcVarIndex = 0; srcVarIndex < srcVarNamesNumValues.size(); srcVarIndex++) {
        this->srcVarNameToSrcVarIndex[srcVarNamesNumValues[srcVarIndex].first] = srcVarIndex;
    }
    root = -1;
}

std::vector<std::pair<std::string, int> > Odd::getSrcVariableDetails() {
    return this->srcVarNameNumValues;
}

NnfNode::NnfNode(long long idx, std::vector<int> children, NnfNode::NodeType type, std::string srcVarName, int srcVarVal) {
    this->idx = idx;
    this->children = children;
    this->type = type;
    this->srcVarName = srcVarName;
    this->srcVarVal = srcVarVal;
}

void NnfNode::setChild(int val, int childIdx) {
    this->children[val] = childIdx;
}

int NnfNode::getCardinality() {
    return this->children.size();
}

NnfNode::NodeType NnfNode::getType() {
    return type;
}

std::string NnfNode::getSrcVarName() {
    return this->srcVarName;
}

int NnfNode::getChild(int index) {
    return this->children[index];
}

long long NnfNode::getIndex() {
    return idx;
}

int NnfNode::getSrcVarVal() {
    return srcVarVal;
}

NnfNode
Nnf::addNode(std::vector<long long int> childrenIdxs, NnfNode::NodeType type, std::string srcVarName, int srcVarVal) {
    std::vector<int> children;
    for (auto childIdx: childrenIdxs) {
        if (childIdx < this->getSize()) {
            //children.push_back(this->nodes[childId]);
            children.push_back(childIdx);
        }
        else {
            throw std::logic_error("Some child node does not exist in the NNF yet");
        }
    }

    NnfNode newNode(this->getSize(), children, type, srcVarName, srcVarVal);

    if (!srcVarName.empty()) { // i.e. is a indicator leaf node
        this->srcVarNameValToIndicatorNodeIndex[srcVarName][srcVarVal] = this->getSize();
    }
    this->nodes.push_back(newNode);


    return newNode;
}

long long Nnf::getSize() {
    return this->nodes.size();
}

void Nnf::loadFromOdd(const Odd &diagram) {
    if (!nodes.empty()) {
        throw std::logic_error("NNF must be empty when loading from ODD");
    }

    std::vector<OddNode> oddNodes = diagram.getNodes();

    std::map<long long, long long> oddIdToOrNodeIndex;

    std::map<std::string, bool> srcVarSeen; // true if we have already encountered an ODD node for that srcVar

    //std::map<std::string, std::vector<long long> > srcVarNameValToIndicatorNodeIndex;
    //std::cout << oddNodes.size() << std::endl;
    for (auto oddNode: oddNodes) {
        //std::cout << "FAIL" << std::endl;
        if (oddNode.getType() == OddNode::SINK) {
            //std::cout << oddNode.getId() << std::endl;
            oddIdToOrNodeIndex[oddNode.getId()] = this->getSize();
            this->addNode({}, NnfNode::LEAF, oddNode.getSrcVarName(), oddNode.getSinkNum());
        }
        else {

            //long long firstNewNodeIndex = this->nodes.size(); // all new nodes appear after the current end
            // Add conjunctions (and indicators):
            std::vector<long long> conjIndices;
            bool thisVarSeen = srcVarSeen[oddNode.getSrcVarName()]; // do we have the indicators for this variable
            //std::cout << oddNode.getCardinality() << std::endl;
            for (int chIndex = 0; chIndex < oddNode.getCardinality(); chIndex++) {

                long long chNodeId = oddNode.getChild(chIndex);

                //std::cout << chNodeId << " ";
                if (oddIdToOrNodeIndex.count(chNodeId) == 0) {

                    throw std::logic_error("Could not create NNF from ODD (ODD nodes are not in reverse topological order)");
                }

                // indicator node
                long long indNodeIndex;
                if (thisVarSeen) {
                    indNodeIndex = this->srcVarNameValToIndicatorNodeIndex[oddNode.getSrcVarName()][chIndex];
                }
                else {
                    indNodeIndex = this->getSize();
                    srcVarSeen[oddNode.getSrcVarName()] = true;
                    this->addNode({}, NnfNode::LEAF, oddNode.getSrcVarName(), chIndex);
                    //srcVarNameValToIndicatorNodeIndex[oddNode.getSrcVarName()].push_back(indNodeIndex);
                }


                // conjunction
                conjIndices.push_back(this->getSize());
                this->addNode({indNodeIndex, oddIdToOrNodeIndex[chNodeId]}, NnfNode::CONJ);

            }
            //std::cout << std::endl;

            // disjunction
            oddIdToOrNodeIndex[oddNode.getId()] = this->getSize();
            this->addNode(conjIndices, NnfNode::DISJ);
        }
    }
}

std::vector<NnfNode> Nnf::getNodes() const {
    return nodes;
}

void Nnf::dump() {
    std::ofstream fin("delete.txt");
    fin << "Nnf Size: " << this->getSize() << std::endl;
    for (auto node: nodes) {
        std::cout << node.getIndex() << " " << node.getType() << " " << node.getSrcVarName() << " " << node.getSrcVarVal() << " ";
        for (int chIndex = 0; chIndex < node.getCardinality(); chIndex++) {
            std::cout << node.getChild(chIndex) << " ";
        }
        std::cout << std::endl;
    }

}

NnfNode Nnf::getRoot() const {
    return nodes[nodes.size() - 1];
}

std::map<std::string, std::vector<long long> > Nnf::getSrcVariableMapping() const {
    return srcVarNameValToIndicatorNodeIndex;
}

Nnf::Nnf(std::vector<std::pair<std::string, int>> srcVarNamesNumValues) {
    for (auto nameNumValues: srcVarNamesNumValues) {
        std::string srcVarName = nameNumValues.first;
        int srcVarNumValues = nameNumValues.second;
        this->srcVarNameValToIndicatorNodeIndex[srcVarName] = std::vector<long long> (srcVarNumValues);
    }
}

void cnfClause::addLiteral(long long int idx, bool positive) {
    this->literals.push_back({idx, positive});
}

std::string cnfClause::asString() {
    std::ostringstream oss;
    for (auto literal: literals) {
        // change to 1-indexing
        oss << (literal.second ? literal.first + 1 : -literal.first - 1) << " ";
    }
    oss << "0";
    return oss.str();
}

std::vector<std::pair<long long, bool> > cnfClause::getLiterals() const {
    return literals;
}

Cnf::Cnf() {
}

void Cnf::write(std::string outfile) {
    std::ofstream fout(outfile);
    fout << "p cnf " << numCnfVars << " " << clauses.size() << std::endl;
    for (auto clause: clauses) {
        fout << clause.asString() << std::endl;
    }

    // If src variable mapping has been set, print this information
    if (!this->srcVarNameValToIndicatorNodeIndex.empty()) {
        fout << "c ==============================================================================" << std::endl;
        for (const auto& nameIndices: this->srcVarNameValToIndicatorNodeIndex) {
            fout << "c        ";
            fout << nameIndices.first;
            for (auto index: nameIndices.second) {
                fout << " " << index + 1; //1-indexing in printed file
            }
            fout << std::endl;
        }
    }
}

// IMPORTANT: ONLY WORKS WITH OBDD WRITTEN FORMAT!
void Cnf::read(std::string infile) {
    std::ifstream fin(infile);


    std::string titleString;
    std::getline(fin, titleString);
    std::stringstream ss_t(titleString);
    std::string dummy;
    ss_t >> dummy; ss_t >> dummy; // p cnf

    int numVars, numClauses;
    ss_t >> numVars >> numClauses;
    this->numCnfVars = numVars;

    std::string clauseString;
    for (int i = 0; i < numClauses; i++) {
        std::getline(fin, clauseString);
        std::stringstream ss_c(clauseString);
        cnfClause clause;

        std::string literalStr; long long literalInt;
        while (ss_c >> literalStr) {
            literalInt = std::stoi(literalStr);
            if (literalInt != 0) {
                bool positive = literalInt > 0;
                long long idx = abs(literalInt) - 1;
                clause.addLiteral(idx, positive);
            }
        }

        this->addClause(clause);
    }

    this->srcVarNameValToIndicatorNodeIndex.clear(); // Clear the map if it already contains something
    std::string commentString;
    std::getline(fin, commentString); // c ====================
    while (std::getline(fin, commentString)) {
        std::string commentToken;
        std::stringstream ss_m(commentString);
        ss_m >> commentToken; // c

        std::vector<long long> valToIndicator;
        std::string varName, indicatorIdxStr; long long indicatorIdxInt;
        ss_m >> varName;

        this->srcVarNameValToIndicatorNodeIndex.insert({varName, std::vector<long long>()});

        while (ss_m >> indicatorIdxStr) {
            indicatorIdxInt = std::stoi(indicatorIdxStr);
            this->srcVarNameValToIndicatorNodeIndex[varName].push_back(indicatorIdxInt - 1);
        }
    }
}

void Cnf::addClause(cnfClause cl) {
    clauses.push_back(cl);
}

void Cnf::encodeNNF(Nnf nnfDiagram) {
    this->numCnfVars = nnfDiagram.getSize();

    std::vector<long long> sinkIndices; // where the sinks/prediction nodes are
    std::vector<NnfNode> nodes = nnfDiagram.getNodes();

    for (int idx = 0; idx < nodes.size(); idx++) {
        NnfNode node = nodes[idx];
        std::vector<long long> chIndices;
        for (int chI = 0; chI < node.getCardinality(); chI++) {
            chIndices.push_back(node.getChild(chI));
        }
        switch (node.getType()) {
            case NnfNode::LEAF:
                if (node.getSrcVarName() == "Sink") {
                    sinkIndices.push_back(idx);
                }
                else {
                    // perhaps record some information about the src variable, val, etc. for printing?
                }
                break;
            case NnfNode::DISJ:
                if (node.getIndex() == nnfDiagram.getRoot().getIndex()) {
                    addHeadlessDisjunction(chIndices);
                }
                else {
                    addDisjunction(idx, chIndices);
                }
                break;
            case NnfNode::CONJ:
                if (node.getIndex() == nnfDiagram.getRoot().getIndex()) {
                    //std::cout << node.getIndex() << std::endl;
                    throw std::logic_error("Root node of NNF is conjunction, this is not allowed");
                    // should not happen
                }
                else {
                    addConjunction(idx, chIndices);
                }
                break;

        }
        //std::cout << "HI" << idx << std::endl;
    }
    addHeadlessXOR(sinkIndices);

    this->srcVarNameValToIndicatorNodeIndex = nnfDiagram.getSrcVariableMapping();
}

void Cnf::addHeadlessDisjunction(std::vector<long long int> indices) {
    cnfClause clause;
    for (long long index: indices) {
        clause.addLiteral(index, true);
    }
    this->addClause(clause);
}

void Cnf::addHeadlessXOR(std::vector<long long int> indices) {
    addHeadlessDisjunction(indices);
    for (int i = 0; i < indices.size(); i++) {
        for (int j = i + 1; j < indices.size(); j++) {
            cnfClause clause;
            clause.addLiteral(indices[i], false);
            clause.addLiteral(indices[j], false);
            this->addClause(clause);
        }
    }
}

void Cnf::addDisjunction(long long int parentIndex, std::vector<long long int> chIndices) {
    cnfClause forwardCl;
    forwardCl.addLiteral(parentIndex, false);
    for (long long chIndex: chIndices) {
        forwardCl.addLiteral(chIndex, true);
    }
    this->addClause(forwardCl);

    for (long long chIndex : chIndices) {
        cnfClause backwardCl;
        backwardCl.addLiteral(parentIndex, true);
        backwardCl.addLiteral(chIndex, false);
        this->addClause(backwardCl);
    }
}

void Cnf::addConjunction(long long int parentIndex, std::vector<long long int> chIndices) {
    cnfClause backwardCl;
    backwardCl.addLiteral(parentIndex, true);
    for (long long chIndex: chIndices) {
        backwardCl.addLiteral(chIndex, false);
    }
    this->addClause(backwardCl);

    for (long long chIndex : chIndices) {
        cnfClause forwardCl;
        forwardCl.addLiteral(parentIndex, false);
        forwardCl.addLiteral(chIndex, true);
        this->addClause(forwardCl);
    }
}

GraphModel Cnf::toGraph() {
    std::vector<std::vector<bool> > adjMatrix = std::vector<std::vector<bool> > (this->numCnfVars, std::vector<bool>(this->numCnfVars));
    for (auto clause: this->clauses) {
        std::vector<std::pair<long long, bool> > literals = clause.getLiterals();
        for (int i = 0; i < literals.size(); i++) {
            // node is not connected to itself
            for (int j = i + 1; j < literals.size(); j++) {
                adjMatrix[literals[i].first][literals[j].first] = true;
                adjMatrix[literals[j].first][literals[i].first] = true;
            }
        }
    }

    return GraphModel(adjMatrix);
}

std::map<int, std::vector<int> >
Cnf::constraintsToCnfConstraints(std::map<std::string, std::vector<std::string>> constraintMap) {
    std::map<int, std::vector<int> > cnfConstraints;
    for (auto const& constraint: constraintMap) {
        std::vector<int> allBeforeCnfIdxs;
        for (std::string beforeString: constraint.second) {
            allBeforeCnfIdxs.insert(allBeforeCnfIdxs.begin(), this->srcVarNameValToIndicatorNodeIndex[beforeString].begin(), this->srcVarNameValToIndicatorNodeIndex[beforeString].end());
        }

        for (int cnfIndex: this->srcVarNameValToIndicatorNodeIndex[constraint.first]) {
            cnfConstraints[cnfIndex] = allBeforeCnfIdxs;
        }
    }
    return cnfConstraints;
}




//////////////////////////////////////////
// NEW NNF->CNF direct conversion
////////////////////////////////////////////
std::vector<cnfClause> Cnf::disjoinCnf(std::vector<cnfClause> cnf1, std::vector<cnfClause> cnf2) {
    std::vector<cnfClause> newCnf;
    for (auto clause1: cnf1) {
        for (auto clause2: cnf2) {
            cnfClause newClause(clause1);
            for (auto lit: clause2.getLiterals()) {
                newClause.addLiteral(lit.first, lit.second);
            }
            newCnf.push_back(newClause);
        }
    }
    return newCnf;
}

std::vector<cnfClause> Cnf::conjoinCnf(std::vector<cnfClause> cnf1, std::vector<cnfClause> cnf2) {
    std::vector<cnfClause> newCnf;
    newCnf.insert(newCnf.end(), cnf1.begin(), cnf1.end());
    newCnf.insert(newCnf.end(), cnf2.begin(), cnf2.end());
    return newCnf;
}

void Cnf::encodeNNF2(Nnf nnfDiagram) {
    std::vector<long long> sinkIndices; // where the sinks/prediction nodes are
    std::vector<NnfNode> nodes = nnfDiagram.getNodes();

    std::vector<std::vector<cnfClause> > nnfNodeIdxToClauses(nodes.size());

    for (int idx = 0; idx < nodes.size(); idx++) {
        NnfNode node = nodes[idx];
        std::vector<long long> chIndices;
        for (int chI = 0; chI < node.getCardinality(); chI++) {
            chIndices.push_back(node.getChild(chI));
        }
        switch (node.getType()) {
            case NnfNode::LEAF:
                nnfNodeIdxToClauses[idx] = {cnfClause()};
                nnfNodeIdxToClauses[idx][0].addLiteral(idx, true);
                if (node.getSrcVarName() == "Sink") {
                    sinkIndices.push_back(idx);
                }
                else {
                    // perhaps record some information about the src variable, val, etc. for printing?
                }
                break;
            case NnfNode::DISJ:
                nnfNodeIdxToClauses[idx] = nnfNodeIdxToClauses[chIndices[0]];
                for (int chI = 1; chI < node.getCardinality(); chI++) {
                    nnfNodeIdxToClauses[idx] = disjoinCnf(nnfNodeIdxToClauses[idx], nnfNodeIdxToClauses[chIndices[chI]]);
                }
                break;
            case NnfNode::CONJ:
                if (node.getIndex() == nnfDiagram.getRoot().getIndex()) {
                    //std::cout << node.getIndex() << std::endl;
                    throw std::logic_error("Root node of NNF is conjunction, this is not allowed");
                    // should not happen
                }
                else {
                    nnfNodeIdxToClauses[idx] = nnfNodeIdxToClauses[chIndices[0]];
                    for (int chI = 1; chI < node.getCardinality(); chI++) {
                        nnfNodeIdxToClauses[idx] = conjoinCnf(nnfNodeIdxToClauses[idx], nnfNodeIdxToClauses[chIndices[chI]]);
                    }
                }
                break;

        }
        //std::cout << "HI" << idx << std::endl;
    }
    //addHeadlessXOR(sinkIndices);

    if (nnfDiagram.getRoot().getIndex() != nodes.size() - 1) {
        throw std::logic_error("Problem with NNF");
    }

    std::vector<cnfClause> rootCnf = nnfNodeIdxToClauses[nnfDiagram.getRoot().getIndex()];

    std::map<std::string, std::vector<long long> > squashedSrcVarNameValToIndicatorNodeIndex;
    std::vector<long long> idxMap(nodes.size());

    int newIdx = 0;
    for (auto varIdxs: nnfDiagram.getSrcVariableMapping()) {
        for (auto oldIdx: varIdxs.second) {
            squashedSrcVarNameValToIndicatorNodeIndex[varIdxs.first].push_back(newIdx);
            idxMap[oldIdx] = newIdx;
            newIdx++;
        }
    }

    // Construct this Cnf
    this->numCnfVars = newIdx;
    for (auto clause: rootCnf) {
        clause.remapLiterals(idxMap);
        this->addClause(clause);
    }

    this->write("clascnft.txt");

    this->srcVarNameValToIndicatorNodeIndex = squashedSrcVarNameValToIndicatorNodeIndex;
}




