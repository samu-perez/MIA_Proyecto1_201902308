#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include "nodo.h"
#include "analizador/scanner.h"
#include "analizador/parser.h"

#include "struct.cpp"
#include "mkdisk.cpp"
#include "rmdisk.cpp"
#include "fdisk.cpp"

//extern int yyrestart(FILE* entrada);
extern int yyparse();
extern Nodo *raiz; // Raiz del arbol

using namespace std;

void leerComando(char*);
void reconocerComando(Nodo *raiz);
void printux();

int main(){
    char comando[400];

    //printux();
    
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
            cout<<comando<<endl;
            reconocerComando(raiz);
            
        } else {
            cout<<"Comando no reconocido"<<endl;
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
        
    } else if(raiz->tipo == "EXEC"){
        cout<<raiz->tipo<<endl;
        Nodo n = raiz->hijos.front();
        cout<<n.tipo + ": "<<n.valor<<endl;
    } else{
        cout<<"Comando no reconocido"<<endl;
    }
}

void printux(){
    MBR mbr;
    FILE *archivo_binario;
    archivo_binario = fopen("Disco3.dk", "rb+");
    int cont = 0;
    
    cout << "-----------------------------------------" << endl;
    while (cont < 10){
        fseek(archivo_binario,cont*sizeof(MBR), SEEK_SET);
        fread(&mbr, sizeof(mbr), 1, archivo_binario);
        cout<<"Date "<< mbr.mbr_date<<endl;
        cout<<"Fit "<< mbr.mbr_disk_fit<<endl;
        cout<<"Signature "<< mbr.mbr_disk_signature<<endl;
        cout<<"Size "<< mbr.mbr_size<<endl;
        cont++;
    }
    cont = 0;
    fclose(archivo_binario);
    cout << "-----------------------------------------" << endl;
}