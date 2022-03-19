#include <iostream>
#include <list>
#include <regex>

#include "nodo.h"
#include "listaMount.cpp"
#include "reportes.cpp"

using namespace std;

extern ListaMount *listaMount;

string directorio_(string direccion);
string extension_(string direccion);
void leerComando(char*);

void REP(Nodo *raiz){
    string name = "";
    string path = "";
    string id = "";

    list<Nodo> :: iterator it;
    for(it = raiz->hijos.begin(); it != raiz->hijos.end(); ++it){

        if(it->tipo == "name"){
            name = it->valor;
        } else if(it->tipo == "path"){
            path = it->valor;
            path = regex_replace(path, regex("\""), "");
            path = regex_replace(path, regex("/home/"), "/home/samuel/Escritorio/");
        } else if(it->tipo == "id"){
            id = it->valor;
        }
    }

    
    if(path != ""){
        if(name != ""){
            if(id != ""){
                string direccion = listaMount->direccion(id);
                string ext = extension_(path);
                if(direccion != "NULL"){
                    string direct_ = directorio_(path);
                    string comando = "sudo mkdir -p \'"+direct_+"\'";
                    system(comando.c_str());
                    string comando2 = "sudo chmod -R 777 \'"+direct_+"\'";
                    system(comando2.c_str());
                    if(name == "mbr"){
                        graficarMBR(direccion, path, ext);

                    } else if(name == "disk"){
                        graficarDisk(direccion, path, ext);

                    } else if(name == "sb"){
                        NodoMount *aux = listaMount->getMount(id);
                        int indice = buscarParticionPE(aux->direccion, aux->nombre);
                        if(indice != -1){    //Primaria|Extendida
                            MBR masterboot;
                            FILE *fp = fopen(aux->direccion.c_str(),"rb+");
                            fread(&masterboot, sizeof(MBR), 1, fp);
                            fseek(fp, masterboot.mbr_partition[indice].part_start, SEEK_SET);
                            fclose(fp);
                            graficarSuperBloque(aux->direccion, path, ext, masterboot.mbr_partition[indice].part_start);
                        }else{
                            int indice = buscarParticionL(aux->direccion, aux->nombre);
                            if(indice != -1){
                                EBR extendedBoot;
                                FILE *fp = fopen(aux->direccion.c_str(),"rb+");
                                fseek(fp, indice, SEEK_SET);
                                fread(&extendedBoot, sizeof(EBR), 1, fp);
                                int start = static_cast<int>(ftell(fp));
                                fclose(fp);
                                graficarSuperBloque(aux->direccion, path, ext, start);
                            }
                        }
                    }
                } else{
                    cout << "ERROR: no se encuentra la particion" << endl;
                }
            } else{
                cout << "ERROR: parametro -id no definido" << endl;
            }
        } else{
            cout << "ERROR: parametro -name no definido" << endl;
        }
    } else{
        cout << "ERROR: parametro -path no definido" << endl;
    }
    
}

void EXEC(Nodo *raiz){
    string path = raiz->hijos.front().valor;
    FILE *fp;
    if((fp = fopen(path.c_str(),"r"))){
        char line[400] = "";
        memset(line, 0, sizeof(line));
        while(fgets(line, sizeof(line), fp)){
            if(line[0] != '\n'){
                cout << line << endl;
                leerComando(line);
            }
            memset(line, 0, sizeof(line));
        }
        fclose(fp);
    }else{
        cout << "ERROR: al abrir el archivo" << endl;
    }
}


string directorio_(string direccion){
    string delimiter = "/";
    size_t pos = 0;
    string res = "";
    while((pos = direccion.find(delimiter)) != string::npos){
        res += direccion.substr(0,pos) + "/";
        direccion.erase(0,pos + delimiter.length());
    }
    return res;
}

string extension_(string direccion){
    string aux = direccion;
    string delimiter = ".";
    size_t pos = 0;
    while((pos = aux.find(delimiter)) != string::npos){
        aux.erase(0, pos+delimiter.length());
    }
    return aux;
}
