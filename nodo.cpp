#include "nodo.h"

Nodo::Nodo(string t, string v){
    tipo = t;
    valor = v;
    hijos = list<Nodo>();
}

void Nodo::add(Nodo n){
    hijos.push_back(n);
}