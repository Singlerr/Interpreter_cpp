//
// Created by Singlerr on 2021-07-11.
//
#include <stdio.h>
#include <wchar.h>
#include "symboltable.h"
using namespace std;

wchar_t * symboltable::getvalue(wchar_t *name) {
    return table[name];
}

bool symboltable::exists(wchar_t *name) {
    return table.find(name) != table.end();
}
void symboltable::registerValue(wchar_t *name, wchar_t *value) {
    table.insert(pair<wchar_t*,wchar_t*>(name,value));
}