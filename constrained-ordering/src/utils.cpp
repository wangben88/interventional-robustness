//
// Created by Benjie Wang on 12/10/2020.
//

#include <string.h>
#include <stdlib.h>
#include <iostream>

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

void remove_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    char *s = (char*) dot;
    if(s != nullptr)
        s[0] = '\0';
}
