#include <iostream>
#include <list>
#include <regex>

#include "nodo.h"
#include "listaMount.cpp"

using namespace std;

extern ListaMount *listaMount;

void MOUNT(Nodo *raiz){
    string path = "";
    string name = "";

    list<Nodo> :: iterator it;
    for(it = raiz->hijos.begin(); it != raiz->hijos.end(); ++it){

        if(it->tipo == "path"){
            path = it->valor;
            path = regex_replace(path, regex("\""), "");
            path = regex_replace(path, regex("/home/"), "/home/samuel/Escritorio/");
        } else if(it->tipo == "name"){
            name = it->valor;
        }
    }
    
    if(path != ""){ //Parametro obligatorio
        if(name != ""){ //Parametro obligtaorio
            int indiceP = buscarParticionPE(path, name);
            if(indiceP != -1){
                FILE *fp;
                if((fp = fopen(path.c_str(),"rb+"))){
                    MBR masterboot;
                    fseek(fp, 0, SEEK_SET);
                    fread(&masterboot, sizeof(MBR), 1, fp);
                    masterboot.mbr_partition[indiceP].part_status = '2';
                    fseek(fp, 0, SEEK_SET);
                    fwrite(&masterboot, sizeof(MBR), 1, fp);
                    fclose(fp);
                    //int letra = listaMount->buscarLetra(path, name);
                    int num = listaMount->buscarNumero(path, name);
                    if(num == -1){
                        cout << "ERROR: la particion ya esta montada" << endl;
                    } else{
                        //int num = listaMount->buscarNumero(path, name);
                        int letra = listaMount->buscarLetra(path, name);
                        char auxLetra = static_cast<char>(letra);
                        listaMount->insertar(path, name, auxLetra, num);
                        cout << "*Particion montada con exito*" << endl;
                        listaMount->mostrar();
                    }
                }else{
                    cout << "ERROR: disco no encontrado" << endl;
                }
            } else{ //Posiblemente logica
                int indiceP = buscarParticionL(path, name);
                if(indiceP != -1){
                    FILE *fp;
                    if((fp = fopen(path.c_str(), "rb+"))){
                        EBR extendedBoot;
                        fseek(fp, indiceP, SEEK_SET);
                        fread(&extendedBoot, sizeof(EBR),1,fp);
                        extendedBoot.part_status = '2';
                        fseek(fp, indiceP, SEEK_SET);
                        fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                        fclose(fp);

                        //int letra = listaMount->buscarLetra(path, name);
                        int num = listaMount->buscarNumero(path, name);
                        if(num == -1){
                            cout << "ERROR: la particion ya esta montada" << endl;
                        } else{
                            //int num = listaMount->buscarNumero(path, name);
                            int letra = listaMount->buscarLetra(path, name);
                            char auxLetra = static_cast<char>(letra);
                            listaMount->insertar(path, name, auxLetra, num);
                            cout << "*Particion montada con exito*" << endl << endl;
                            listaMount->mostrar();
                        }
                    } else{
                        cout << "ERROR: disco no encontrado" << endl;
                    }
                } else{
                    cout << "ERROR: particion no encontrada" << endl;
                }
            }
        } else{
            cout << "ERROR: parametro -name no definido" << endl;
        }
    }else{
        cout << "ERROR: parametro -path no definido" << endl;
    }
    
}

void UNMOUNT(Nodo *raiz){
    string id = raiz->hijos.front().valor;
    listaMount->eliminar(id);
    listaMount->mostrar();
}