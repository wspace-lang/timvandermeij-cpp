#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

class UnreachableTokenException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: unreachable token.";
    }
};

class OutOfBoundsException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: index out of bounds.";
    }
};

class PrematureEndException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: number or label ended prematurely.";
    }
};

class UndefinedSignException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: sign of the number is undefined.";
    }
};

class InstructionNotFoundException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: instruction has not been found.";
    }
};

class LabelNotFoundException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: label has not been found.";
    }
};

class SomeException: public std::exception {
    virtual const char *what() const throw () {
        return "Error: some unspecified exception has occurred. :(";
    }
};
#endif
