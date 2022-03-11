#include <iostream>
#include <list>
#include <string>
#include <regex>

#include "nodo.h"
//#include "struct.cpp"
using namespace std;

void crearArchivo(string direccion);
string directorio(string direccion);

void MKDISK(Nodo *raiz){
    cout<<raiz->tipo<<endl;
    int size = 0;
    char fit = 0;
    char unit = 0;
    string path = "";

    list<Nodo> :: iterator it;
    for(it = raiz->hijos.begin(); it != raiz->hijos.end(); ++it){
        cout<<it->tipo<<"-"<<it->valor<<endl;

        if(it->tipo == "size"){
            size = stoi(it->valor);
        } else if(it->tipo == "fit"){
            fit = it->hijos.front().valor[0];
            if(fit == 'b'){
                fit = 'B';
            }else if(fit == 'f'){
                fit = 'F';
            }else if(fit == 'w'){
                fit = 'W';
            }
        } else if(it->tipo == "unit"){
            unit = it->valor[0];
            if(unit == 'k' || unit == 'K'){
                unit = 'k';
            } else if(unit == 'm' || unit == 'M'){
                unit = 'm';
            }
        } else if(it->tipo == "path"){
            path = it->valor;
            path = regex_replace(path, regex("\""), "");
            cout<<"PATHH "<<path<<endl;
            path = regex_replace(path, regex("/home/"), "/home/samuel/Escritorio/");
            cout<<"REPATHH "<<path<<endl;
        }
    }

    MBR masterboot;
    int totalSize = 0;
    crearArchivo(path);

    masterboot.mbr_date = time(nullptr);
    masterboot.mbr_disk_signature = (int)time(nullptr);

    if(unit != 0){  //Si hay parametro unit
        if(unit == 'm'){
            masterboot.mbr_size = size * 1048576;
            totalSize = size * 1024;
        } else {
            masterboot.mbr_size = size * 1024;
            totalSize = size;
        }
    } else {
        masterboot.mbr_size = size * 1048576;
        totalSize = size * 1024;
    }

    if(fit != 0) masterboot.mbr_disk_fit = fit;
    else masterboot.mbr_disk_fit = 'F';

    //Inicializar las particiones en el MBR
    for(int i = 0; i < 4; i++){
        masterboot.mbr_partition[i].part_status = '0';
        masterboot.mbr_partition[i].part_type = '0';
        masterboot.mbr_partition[i].part_fit = '0';
        masterboot.mbr_partition[i].part_size = 0;
        masterboot.mbr_partition[i].part_start = -1;
        strcpy(masterboot.mbr_partition[i].part_name, "");
    }

    string comando = "dd if=/dev/zero of=\"" + path + "\" bs=1024 count=" + to_string(totalSize);
    system(comando.c_str());
    FILE *fp = fopen(path.c_str(), "rb+");
    fseek(fp, 0, SEEK_SET);
    fwrite(&masterboot, sizeof(MBR), 1, fp);
    fclose(fp);

}

void crearArchivo(string direccion){
    string aux = directorio(direccion);
    cout<<aux<<endl;
    string comando = "sudo mkdir -p \'" + aux + "\'";
    //string comando = "mkdir -p \'" + aux + "\'";
    system(comando.c_str());
    string comando2 = "sudo chmod -R 777 \'" + aux + "\'";
    //string comando2 = "chmod -R 755 \'" + aux + "\'";
    system(comando2.c_str());
    string arch = direccion;
    cout<<"ARCH "<< arch<<endl;
    FILE *fp = fopen(arch.c_str(), "wb");
    if((fp = fopen(arch.c_str(), "wb")))
        fclose(fp);
    else
        cout << "Error al crear el archivo" << endl;
}

string directorio(string direccion){
    string delimiter = "/";
    size_t pos = 0;
    string res = "";
    while((pos = direccion.find(delimiter)) != string::npos){
        res += direccion.substr(0,pos) + "/";
        direccion.erase(0,pos + delimiter.length());
    }
    return res;
}
