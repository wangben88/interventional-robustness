#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdio.h>

#define THROW_BUFFER_SIZE 512
class throw_string_error : public std::string, public std::exception {
    private:
        char buffer[THROW_BUFFER_SIZE];
    public:
        throw_string_error( const char* format, ... ) {
            va_list args;
            va_start(args, format);
            vsprintf(buffer,format, args);
            va_end(args);

        };
        ~throw_string_error() throw() {};
        const char* what() const throw() { return buffer; } ;
};

#endif
