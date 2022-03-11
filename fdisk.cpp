#include <iostream>
#include <list>
#include <regex>

#include "nodo.h"
//#include "struct.cpp"

using namespace std;

void crearParticionPrimaria(string direccion, string nombre, int size, char fit, char unit);
bool existeParticion(string direccion, string nombre);

void FDISK(Nodo *raiz){
    int size = 0;
    int add = 0;
    char unit = 0;
    char type = 0;
    char fit = 0;
    string name = "";
    string delete_ = "";
    string path = "";

    list<Nodo> :: iterator it;
    for(it = raiz->hijos.begin(); it != raiz->hijos.end(); it++){
        //cout<<it->tipo<<"-"<<it->valor<<endl;

        if(it->tipo == "size"){
            size = stoi(it->valor);
        } else if(it->tipo == "unit"){
            unit = it->valor[0];
            if(unit == 'b' || unit == 'B'){
                unit = 'b';
            } else if(unit == 'k' || unit == 'K'){
                unit = 'k';
            } else if(unit == 'm' || unit == 'M'){
                unit = 'm';
            }
        } else if(it->tipo == "path"){
            path = it->valor;
            path = regex_replace(path, regex("\""), "");
            path = regex_replace(path, regex("/home/"), "/home/samuel/Escritorio/");
        } else if(it->tipo == "type"){
            type = it->valor[0];
            if(type == 'p' || type == 'P'){
                type = 'P';
            } else if(type == 'e' || type == 'E'){
                type = 'E';
            } else if(type == 'l' || type == 'L'){
                type = 'L';
            }
        } else if(it->tipo == "fit"){
            fit = it->hijos.front().valor[0];
            if(fit == 'b'){
                fit = 'B';
            }else if(fit == 'f'){
                fit = 'F';
            }else if(fit == 'w'){
                fit = 'W';
            }
        } else if(it->tipo == "delete"){
            delete_ = it->valor;
        } else if(it->tipo == "name"){
            name = it->valor;
            name = regex_replace(name, regex("\""), "");
        } else if(it->tipo == "add"){
            add = stoi(it->valor);
        }
    }

    cout<<"SIZE - "<<size<<endl;
    cout<<"UNIT - "<<unit<<endl;
    cout<<"PATH - "<<path<<endl;
    cout<<"TYPE - "<<type<<endl;
    cout<<"FIT - "<<fit<<endl;
    cout<<"DELETE - "<<delete_<<endl;
    cout<<"NAME - "<<name<<endl;
    cout<<"ADD - "<<add<<endl;

    if(delete_ != ""  && add != 0){
        cout<< "ERROR: Parametro -delete|-add demas" << endl;
    } else {
        if(delete_ != ""){
            cout<<"Eliminar particion"<<endl;
        } else if(add != 0){
            cout<<"Agregar Quitar particion"<<endl;
        } else{
            if(type != 0){  //Si especifica tipo de particion
                if(type == 'P'){
                    cout<<"Particion Primaria"<<endl;
                    crearParticionPrimaria(path, name, size, fit, unit);
                } else if(type == 'E'){
                    cout<<"Particion Extendida"<<endl;
                } else if(type == 'L'){
                    cout<<"Particion Logica"<<endl;
                }
            } else{    //Si no especifica, se considera particion primaria
                cout<<"Particion Primaria"<<endl;
                crearParticionPrimaria(path, name, size, fit, unit);
            }
        }
    }

}

void crearParticionPrimaria(string direccion, string nombre, int size, char fit, char unit){
    char auxFit = 0;
    char auxUnit = 0;
    int sizeBytes = 0;
    char buffer = '1';

    if(fit != 0) auxFit = fit;
    else auxFit = 'W';

    if(unit != 0){
        auxUnit = unit;
        if(auxUnit == 'b'){
            sizeBytes = size;
        }else if(auxUnit == 'k'){
            sizeBytes = size * 1024;
        }else{
            sizeBytes = size*1024*1024;
        }
    }else{
        sizeBytes = size * 1024;
    }

    FILE *fp;
    MBR masterboot;

    if((fp = fopen(direccion.c_str(), "rb+"))){
        bool particion = false; //particion disponible
        int numParticion = 0;   //numero de particion
        fseek(fp, 0, SEEK_SET);
        fread(&masterboot, sizeof(MBR), 1, fp);
        //Verificar si existe una particion disponible
        for(int i = 0; i < 4; i++){
            if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size >= sizeBytes)){
                particion = true;
                numParticion = i;
                break;
            }
        }

        if(particion){
            //Verificar el espacio libre del disco
            int espacioUsado = 0;
            for(int i = 0; i < 4; i++){
                if(masterboot.mbr_partition[i].part_status != '1'){
                    espacioUsado += masterboot.mbr_partition[i].part_size;
                }
            }
            
            cout << "Espacio disponible: " << (masterboot.mbr_size - espacioUsado) << " Bytes" << endl;
            cout << "Espacio necesario:  " << sizeBytes << " Bytes" << endl;
            
            //Verificar que haya espacio suficiente para crear la particion
            if((masterboot.mbr_size - espacioUsado) >= sizeBytes){
                if(!existeParticion(direccion, nombre)){
                    if(masterboot.mbr_disk_fit == 'F'){ //FIRST FIT
                        masterboot.mbr_partition[numParticion].part_type = 'P';
                        masterboot.mbr_partition[numParticion].part_fit = auxFit;
                        //start
                        if(numParticion == 0){
                            masterboot.mbr_partition[numParticion].part_start = sizeof(masterboot);
                        }else{
                            masterboot.mbr_partition[numParticion].part_start = masterboot.mbr_partition[numParticion-1].part_start + masterboot.mbr_partition[numParticion-1].part_size;
                        }
                        masterboot.mbr_partition[numParticion].part_size = sizeBytes;
                        masterboot.mbr_partition[numParticion].part_status = '0';
                        strcpy(masterboot.mbr_partition[numParticion].part_name, nombre.c_str());
                        //Se guarda de nuevo el MBR
                        fseek(fp, 0, SEEK_SET);
                        fwrite(&masterboot, sizeof(MBR), 1, fp);
                        //Se guardan los bytes de la particion
                        fseek(fp,masterboot.mbr_partition[numParticion].part_start, SEEK_SET);
                        for(int i = 0; i < sizeBytes; i++){
                            fwrite(&buffer, 1, 1, fp);
                        }
                        cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    }else if(masterboot.mbr_disk_fit == 'B'){   //BEST FIT
                        int bestIndex = numParticion;
                        for(int i = 0; i < 4; i++){
                            if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size >= sizeBytes)){
                                if(i != numParticion){
                                    if(masterboot.mbr_partition[bestIndex].part_size > masterboot.mbr_partition[i].part_size){
                                        bestIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        masterboot.mbr_partition[bestIndex].part_type = 'P';
                        masterboot.mbr_partition[bestIndex].part_fit = auxFit;
                        //start
                        if(bestIndex == 0){
                            masterboot.mbr_partition[bestIndex].part_start = sizeof(masterboot);
                        }else{
                            masterboot.mbr_partition[bestIndex].part_start = masterboot.mbr_partition[bestIndex-1].part_start + masterboot.mbr_partition[bestIndex-1].part_size;
                        }
                        masterboot.mbr_partition[bestIndex].part_size = sizeBytes;
                        masterboot.mbr_partition[bestIndex].part_status = '0';
                        strcpy(masterboot.mbr_partition[bestIndex].part_name,nombre.c_str());
                        //Se guarda de nuevo el MBR
                        fseek(fp,0,SEEK_SET);
                        fwrite(&masterboot,sizeof(MBR),1,fp);
                        //Se guardan los bytes de la particion
                        fseek(fp,masterboot.mbr_partition[bestIndex].part_start,SEEK_SET);
                        for(int i = 0; i < sizeBytes; i++){
                            fwrite(&buffer, 1, 1, fp);
                        }
                        cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    }else if(masterboot.mbr_disk_fit == 'W'){   //WORST FIT
                        int  worstIndex = numParticion;
                        for(int i = 0; i < 4; i++){
                            if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size>=sizeBytes)){
                                if(i != numParticion){
                                    if(masterboot.mbr_partition[worstIndex].part_size < masterboot.mbr_partition[i].part_size){
                                        worstIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        masterboot.mbr_partition[worstIndex].part_type = 'P';
                        masterboot.mbr_partition[worstIndex].part_fit = auxFit;
                        //start
                        if(worstIndex == 0){
                            masterboot.mbr_partition[worstIndex].part_start = sizeof(masterboot);
                        }else{
                            masterboot.mbr_partition[worstIndex].part_start = masterboot.mbr_partition[worstIndex-1].part_start + masterboot.mbr_partition[worstIndex-1].part_size;
                        }
                        masterboot.mbr_partition[worstIndex].part_size = sizeBytes;
                        masterboot.mbr_partition[worstIndex].part_status = '0';
                        strcpy(masterboot.mbr_partition[worstIndex].part_name, nombre.c_str());
                        //Se guarda de nuevo el MBR
                        fseek(fp,0,SEEK_SET);
                        fwrite(&masterboot,sizeof(MBR),1,fp);
                        //Se guardan los bytes de la particion
                        fseek(fp, masterboot.mbr_partition[worstIndex].part_start, SEEK_SET);
                        for(int i = 0; i < sizeBytes; i++){
                            fwrite(&buffer, 1, 1, fp);
                        }
                        cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    }
                }else{
                    cout << "ERROR: ya existe una particion con ese nombre" << endl;
                }

            }else{
                cout << "ERROR: la particion a crear excede el espacio libre" << endl;
            }
        }else{
            cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
        }
    fclose(fp);
    }else{
        cout << "ERROR: no existe el disco" << endl;
    }
}

bool existeParticion(string direccion, string nombre){
    int extendida = -1;
    FILE *fp;
    if((fp = fopen(direccion.c_str(),"rb+"))){
        MBR masterboot;
        fseek(fp,0,SEEK_SET);
        fread(&masterboot, sizeof(MBR), 1, fp);
        for(int i = 0; i < 4; i++){
            if(strcmp(masterboot.mbr_partition[i].part_name,nombre.c_str()) == 0){
                fclose(fp);
                return true;
            }else if(masterboot.mbr_partition[i].part_type == 'E'){
                extendida = i;
            }
        }
        if(extendida != -1){
            fseek(fp, masterboot.mbr_partition[extendida].part_start, SEEK_SET);
            EBR extendedBoot;
            while((fread(&extendedBoot, sizeof(EBR), 1, fp))!=0 && (ftell(fp) < (masterboot.mbr_partition[extendida].part_size + masterboot.mbr_partition[extendida].part_start))){
                if(strcmp(extendedBoot.part_name,nombre.c_str()) == 0){
                    fclose(fp);
                    return true;
                }
                if(extendedBoot.part_next == -1){
                    fclose(fp);
                    return false;
                }
            }
        }
    }
    fclose(fp);
    return false;
}