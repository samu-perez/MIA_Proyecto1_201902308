#ifndef LISTAMOUNT
#define LISTAMOUNT

#include <iostream>
#include "nodoMount.cpp"

using namespace std;

class ListaMount{
    public:
        NodoMount *primero;
        ListaMount();
        void insertar(string, string, char, int);
        void mostrar();
        bool buscar(string, string);
        void eliminar(string);
        int buscarLetra(string, string);
        int buscarNumero(string, string);
        string direccion(string);
        NodoMount* getMount(string id);
};

ListaMount:: ListaMount(){
    primero = NULL;
}

void ListaMount:: insertar(string direccion, string nombre, char letra, int num){
    NodoMount *nuevo = new NodoMount(direccion, nombre, letra, num);
    NodoMount *aux = primero;
    if(primero == NULL){
        primero = nuevo;
    } else{
        while(aux->siguiente != NULL){
            aux = aux->siguiente;
        }
        aux->siguiente = nuevo;
    }
}

void ListaMount:: mostrar(){
    cout << "|----- Particiones montadas -----|" << endl;
    cout << "|------ Nombre    |    ID -------|" << endl;
    cout << "__________________________________" << endl;
    NodoMount *aux = primero;
    while(aux != NULL){
        cout << "    "<< aux->nombre<< "          " <<"08" <<aux->num <<aux->letra << endl;
        cout << "---------------------------------" << endl;
        aux = aux->siguiente;
    }
}

bool ListaMount:: buscar(string direccion, string nombre){
    NodoMount *aux = primero;
    while(aux != NULL){
        if((aux->direccion == direccion) && (aux->nombre == nombre)){
            return true;
        }
        aux = aux->siguiente;
    }
    return false;
}

void ListaMount:: eliminar(string id){
    NodoMount *aux = primero;
    string tempID = "08";     //201902308
    bool eliminado = false;

    if(primero != NULL) tempID += to_string(aux->num) + aux->letra;

    if(id == tempID){
        primero = aux->siguiente;
        eliminado = true;
    }else{
        NodoMount *aux2 = NULL;
        while(aux != NULL){
            tempID = "08";
            tempID += to_string(aux->num) + aux->letra;
            if(id == tempID){
                aux2->siguiente = aux->siguiente;
                eliminado = true;
            }
            aux2 = aux;
            aux = aux->siguiente;
        }
    }
    if(eliminado) cout<< "*Particion desmontada con exito*" <<endl;
    else cout<< "ERROR: no se encuentra la partición montada" <<endl;
}

int ListaMount:: buscarNumero(string direccion, string nombre){
    NodoMount *aux = primero;
    int num = 1;
    while(aux != NULL){
        if((direccion == aux->direccion) && (nombre == aux->nombre)){
            return -1;
        }else{
            if(direccion == aux->direccion){
                return aux->num;
            }else if(num == aux->num){
                num++;
            }
        }
        aux = aux->siguiente;
    }
    return num;
}

int ListaMount:: buscarLetra(string direccion, string nombre){
    int letra = 'a';
    NodoMount *aux = primero;
    while(aux != NULL){
        if((direccion == aux->direccion) && (letra <= aux->letra)){
            letra++;
        }
        aux = aux->siguiente;
    }
    return letra;
}


string ListaMount:: direccion(string id){
    NodoMount *aux = primero;
    while(aux != NULL){
        string tempID = "08";
        tempID += to_string(aux->num) + aux->letra;
        if(id == tempID){
            return aux->direccion;
        }
        aux = aux->siguiente;
    }
    return "NULL";
}

NodoMount* ListaMount:: getMount(string id){
    NodoMount *aux = primero;
    while(aux != NULL){
        string tempID = "08" + to_string(aux->num) + aux->letra;
        if(id == tempID){
            return aux;
        }
        aux = aux->siguiente;
    }
    return NULL;
}


#endif // LISTAMOUNT