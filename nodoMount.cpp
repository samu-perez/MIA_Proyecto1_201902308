#ifndef NODOMOUNT
#define NODOMOUNT

#include <iostream>

using namespace std;

class NodoMount{
    public: 
        string direccion, nombre;
        char letra;
        int num;
        NodoMount *siguiente;
        NodoMount(string, string, char, int);
};

NodoMount:: NodoMount(string direccion_, string nombre_, char letra_, int num_){
    direccion = direccion_;
    nombre = nombre_;
    letra = letra_;
    num = num_;
    siguiente = NULL;
}

#endif // NODOMOUNT