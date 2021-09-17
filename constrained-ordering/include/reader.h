//
// Created by Benjie Wang on 18/12/2020.
//

#ifndef CONSTRAINED_ORDERING_READER_H
#define CONSTRAINED_ORDERING_READER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>

// Reads constraints from file. Constraint files consist of lines of the following format:
// <node> <parent1> <parent2> ... <parentk>
// e.g. GoodStudent Age SocioEcon
// Output constraint structure is of the form
// <node> <parenti>
// indicating that <node> must come before <parenti> in the ordering.
std::map<std::string, std::vector<std::string> > readConstraints(std::string infile);

void writeConstraints(std::string outfile, std::map<std::string, std::vector<std::string> > constraints);
#endif //CONSTRAINED_ORDERING_READER_H
