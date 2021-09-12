#ifndef BW_OBDD_TO_CNF_PARSER_H
#define BW_OBDD_TO_CNF_PARSER_H

#include "logicNode.h"
#include "literalMap.h"

/*
 * Each ODD node has an index, which is the index given to them by the .odd file. In the case of sinks S0, S1, etc.
 * we give them indices -1, -2, -3, ... respectively.
 * In the Odd class we store the nodes in a vector, which will contain the nodes in the order they are read.
 * Of course this will be different from the node indices in general, so we maintain a mapping from indices to
 * the position in the vector, so that we can appropriately link the children of nodes as they are being read.
 */

/*
 * Meaning of index is sometimes overloaded. It is both used to refer to which child of a node, as well as
 * in Nnf to refer to which node in the vector of nodes.
 */

Odd loadOdd(std::string infile, int numSinks);

// If no ordering given, follow the default ordering in the CNF file.
std::pair<Cnf, Lmap> loadCnfSpecial(std::string infile, Cnf classifierCnf, std::string orderingfile = "", std::string constraintfile = "");


#endif //BW_OBDD_TO_CNF_PARSER_H
