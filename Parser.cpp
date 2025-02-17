#include "Parser.h"
#include "Exceptions.h"

using namespace std;

// Note: could be implemented using some sort of lookup table
// data structure indexed by a pair of enums.
Mode Parser::determineMode(const Token t1, const Token t2) {
    switch(t1) {
        case LINEFEED:
            return FLOWCONT;
        case SPACE:
            return STACKMANIP;
        case TAB:
            switch(t2) {
                case LINEFEED:
                    return IO;
                case SPACE:
                    return ARITH;
                case TAB:
                    return HEAPACC;
                default:
                    throw UnreachableTokenException();
            }
        default:
            throw UnreachableTokenException();
    }
}

vector<Token> Parser::tokenize(const string &program) {
    vector<Token> tokens;

    for(auto k = program.begin(); k != program.end(); k++) {
        switch(*k) {
            case '\n':
                tokens.push_back(LINEFEED);
                break;
            case ' ':
                tokens.push_back(SPACE);
                break;
            case '\t':
                tokens.push_back(TAB);
                break;
        }
    }
    return tokens;
}

Program Parser::tokensToProgram(const vector<Token> &tokens) {
    size_t amount = tokens.size();
    if(amount < 3) {
        exit(0); // Empty program
    }

    Mode m;
    try {
        m = determineMode(tokens[0], tokens[1]);
    } catch (exception& e) { // Unreachable token exception
        throw e;
    }
    Program p;

    // FLOWCONT and STACKMANIP are only 1 token long, the rest is 2 tokens long
    size_t start = ((m == FLOWCONT || m == STACKMANIP) ? 1 : 2);

    for(size_t k = start; k < amount; k++) {
        if(k == amount) {
            cout << "k has reached max" << endl; // We might want to use an exception here...
        }

        if(m == STACKMANIP) {
            processStackManip(tokens, p, k);
        } else if(m == ARITH) {
            processArith(tokens, p, k);
        } else if(m == HEAPACC) {
            processHeapAcc(tokens, p, k);
        } else if(m == FLOWCONT) {
            processFlowCont(tokens, p, k);
        } else if(m == IO) {
            processIO(tokens, p, k);
        } else {
            throw UnreachableTokenException();
        }
        k++; // Proceed to next instruction
        if(k != amount) {
            m = determineMode(tokens[k], tokens[k + 1]);
            if(!(m == STACKMANIP || m == FLOWCONT)) {
                k++;
            }
        }
    }
    return p;
}

// Side-effect: mutates the index from the for-loop in tokensToProgram
// Labels are also represented as numbers, so labels will be handled as well.
long long Parser::tokensToNumber(const vector<Token> &tokens, size_t &k) {
    size_t amount = tokens.size();
    long long sign;
    long long sum = 0;

    k++;
    if(tokens[k] == SPACE) {
        sign = 1;
    } else if(tokens[k] == TAB) {
        sign = -1;
    } else {
        throw UndefinedSignException();
    }
    k++;
    while(true) {
        if(k == amount) {
            throw PrematureEndException();
        }
        if(tokens[k] == LINEFEED) { // A number is terminated by a LINEFEED
            break;
        }
        sum *= 2;
        if (tokens[k++] == TAB) {
            sum += 1;
        }
    }
    return sign * sum;
}

void Parser::processStackManip(const vector<Token> &tokens, Program &p, size_t &k) {
    if(tokens[k] == SPACE) { // PUSH
        p.push_back(Instruction(PUSH, tokensToNumber(tokens, k)));
    } else if(tokens[k] == TAB) {
        k++;
        if(tokens[k] == SPACE) { // COPY
            p.push_back(Instruction(COPY, tokensToNumber(tokens, k)));
        } else if(tokens[k] == LINEFEED) { // SLIDE
            p.push_back(Instruction(SLIDE, tokensToNumber(tokens, k)));
        } else {
            throw UnreachableTokenException();
        }
    } else if(tokens[k] == LINEFEED) { // DUP, SWAP or DISCARD
        // Could perhaps also be done more concisely
        // with the ?:-operator, but I reckon something will go wrong
        // in the exception branch
        k++;
        if(tokens[k] == SPACE) { // DUP
            p.push_back(DUP);
        } else if(tokens[k] == TAB) { // SWAP
            p.push_back(SWAP);
        } else if(tokens[k] == LINEFEED) { // DISCARD
            p.push_back(DISCARD);
        } else {
            throw UnreachableTokenException();
        }
    } else {
        throw UnreachableTokenException();
    }
}

void Parser::processArith(const vector<Token> &tokens, Program &p, size_t &k) {
    if(tokens[k] == SPACE) {
        k++;
        if(tokens[k] == SPACE) { // ADD
            p.push_back(ADD);
        } else if(tokens[k] == TAB) { // SUB
            p.push_back(SUB);
        } else if(tokens[k] == LINEFEED) { // MUL
            p.push_back(MUL);
        } else {
            throw UnreachableTokenException();
        }
    } else if(tokens[k] == TAB) {
        k++;
        if(tokens[k] == SPACE) { // DIV
            p.push_back(DIV);
        } else if(tokens[k] == TAB) { // MOD
            p.push_back(MOD);
        } else {
            throw UnreachableTokenException();
        }
    } else {
        throw UnreachableTokenException();
    }
}

void Parser::processHeapAcc(const vector<Token> &tokens, Program &p, size_t &k) {
    if(tokens[k] == SPACE) { // STORE
        p.push_back(STORE);
    } else if(tokens[k] == TAB) { // RETRIEVE
        p.push_back(RETRIEVE);
    } else {
        throw UnreachableTokenException();
    }
}

void Parser::processFlowCont(const vector<Token> &tokens, Program &p, size_t &k) {
    if(tokens[k] == SPACE) {
        k++;
        if(tokens[k] == SPACE) { // MARK
            p.push_back(Instruction(MARK, tokensToNumber(tokens, k)));
        } else if(tokens[k] == TAB) { // CALL
            p.push_back(Instruction(CALL, tokensToNumber(tokens, k)));
        } else if(tokens[k] == LINEFEED) { // JUMP
            p.push_back(Instruction(JUMP, tokensToNumber(tokens, k)));
        } else {
            throw UnreachableTokenException();
        }
    } else if(tokens[k] == TAB) {
        k++;
        if(tokens[k] == SPACE) { // JUMPZERO
            p.push_back(Instruction(JUMPZERO, tokensToNumber(tokens, k)));
        } else if(tokens[k] == TAB) { // JUMPNEG
            p.push_back(Instruction(JUMPNEG, tokensToNumber(tokens, k)));
        } else if(tokens[k] == LINEFEED) { // ENDSUB
            p.push_back(ENDSUB);
        } else {
            throw UnreachableTokenException();
        }
    } else if(tokens[k] == LINEFEED && tokens[++k] == LINEFEED) { // ENDPROG
        p.push_back(ENDPROG);
    } else {
        throw UnreachableTokenException();
    }
}

void Parser::processIO(const vector<Token> &tokens, Program &p, size_t &k) {
    if(tokens[k] == SPACE) {
        k++;
        if(tokens[k] == SPACE) { // WRITEC
            p.push_back(WRITEC);
        } else if(tokens[k] == TAB) { // WRITEN
            p.push_back(WRITEN);
        } else {
            throw UnreachableTokenException();
        }
    } else if(tokens[k] == TAB) {
        k++;
        if(tokens[k] == SPACE) { // READC
            p.push_back(READC);
        } else if(tokens[k] == TAB) { // READN
            p.push_back(READN);
        } else {
            throw UnreachableTokenException();
        }
    } else {
        throw UnreachableTokenException();
    }
}
