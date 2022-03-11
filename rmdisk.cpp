#include <iostream>
#include <list>
#include <regex>

#include "nodo.h"

using namespace std;

void RMDISK(Nodo *raiz){
    string path = raiz->hijos.front().valor;
    path = regex_replace(path, regex("\""), "");
    path = regex_replace(path, regex("/home/"), "/home/samuel/Escritorio/");

    FILE *fp;
    if(fp = fopen(path.c_str(), "r")){
        string opcion = "";
        cout << "-> ¿Seguro que desea eliminar el disco? Y/N : ";
        getline(cin, opcion);
        if(opcion == "Y" || opcion == "y"){
            string comando = "rm \"" + path + "\"";
            system(comando.c_str());
            cout << "Disco eliminado" << endl;
        }else if(opcion == "N" || opcion == "n"){
            cout << "Cancelado" << endl;;
        }else{
            cout << "Opcion incorrecta" << endl;
        }
        fclose(fp);
    } else {
        cout << "No existe el disco a eliminar" << endl;
    }
}