//
#include <string.h>
#include <stdlib.h>
#include <iostream>

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

/*
char *get_basename(char *filename) {
    return basename(filename);
}
*/

void remove_ext(const char *filename){
    const char *dot = strrchr(filename, '.');
    char *s = (char*) dot;
    if(s != nullptr)
        s[0] = '\0';
}
