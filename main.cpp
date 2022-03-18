#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "nodo.h"
#include "analizador/scanner.h"
#include "analizador/parser.h"

#include "struct.cpp"
#include "mkdisk.cpp"
#include "rmdisk.cpp"
#include "fdisk.cpp"
#include "listaMount.cpp"
#include "mount-unmount.cpp"
#include "rep-exec.cpp"

ListaMount *listaMount = new ListaMount();

extern int yyparse();
extern Nodo *raiz; //Raiz del arbol

using namespace std;

void leerComando(char*);
void reconocerComando(Nodo *raiz);

int main(){
    char comando[400];
    while(true){
        cout << "Comando:~$ ";
        fgets(comando, sizeof(comando), stdin);
        leerComando(comando);
    }
    
    return 0;
}

void leerComando(char comando[400]){
    if(comando[0] != '#'){
        YY_BUFFER_STATE buffer = yy_scan_string(comando);
        if(yyparse() == 0){
            reconocerComando(raiz);
            
        } else {
            cout<<"ERROR: Comando no reconocido"<<endl;
        }
    }
}

void reconocerComando(Nodo *raiz){
    if(raiz->tipo == "MKDISK"){
        Nodo n = raiz->hijos.front();
        MKDISK(&n);

    } else if(raiz->tipo == "RMDISK"){
        RMDISK(raiz);

    } else if(raiz->tipo == "FDISK"){
        Nodo n = raiz->hijos.front();
        FDISK(&n);
        
    } else if(raiz->tipo == "MOUNT"){
        Nodo n = raiz->hijos.front();
        MOUNT(&n);

    } else if(raiz->tipo == "UNMOUNT"){
        UNMOUNT(raiz);
        
    } else if(raiz->tipo == "REP"){
        Nodo n = raiz->hijos.front();
        REP(&n);
        
    } else if(raiz->tipo == "EXEC"){
        EXEC(raiz);
    } else if(raiz->tipo == "PAUSE"){
        cout<<"Presione Enter para continuar..." <<endl;
        cin.get();
    } else{
        cout<<"Comando no reconocido"<<endl;
    }
}