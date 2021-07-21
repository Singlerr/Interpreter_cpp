//
// Created by Singlerr on 2021-07-11.
//
#include <map>

#ifndef INTERPRETER_CPP_SYMBOLTABLE_H
#define INTERPRETER_CPP_SYMBOLTABLE_H

using namespace std;

class symboltable {
public:
    map<wchar_t*,wchar_t*> table;
    bool exists(wchar_t *name);
    wchar_t * getvalue(wchar_t * name);
    void registerValue(wchar_t *name, wchar_t * value);
};


#endif //INTERPRETER_CPP_SYMBOLTABLE_H
