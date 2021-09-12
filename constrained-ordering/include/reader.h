//
// Created by Benjie Wang on 18/12/2020.
//

#ifndef CONSTRAINED_ORDERING_READER_H
#define CONSTRAINED_ORDERING_READER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>

std::map<std::string, std::vector<std::string> > readConstraints(std::string infile);

void writeConstraints(std::string outfile, std::map<std::string, std::vector<std::string> > constraints);
#endif //CONSTRAINED_ORDERING_READER_H
