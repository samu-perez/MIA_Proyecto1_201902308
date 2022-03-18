#ifndef NODO_H
#define NODO_H

#include <iostream>
#include <list>
using namespace std;

class Nodo {
    public:
        Nodo(string tipo, string valor);
        string tipo, valor;
        list<Nodo> hijos;
        void add(Nodo n);
};

#endif