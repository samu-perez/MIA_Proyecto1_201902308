#include <iostream>
#include <list>
#include <regex>

#include "nodo.h"
#include "listaMount.cpp"

using namespace std;

extern ListaMount *listaMount;

void crearParticionPrimaria(string direccion, string nombre, int size, char fit, char unit);
void crearParticionExtendida(string direccion, string nombre, int size, char fit, char unit);
void crearParticionLogica(string direccion, string nombre, int size, char fit, char unit);
void deleteParticion(string direccion, string nombre, string typeDelete);
void addParticion(string direccion, string nombre, int add, char unit);
bool existeParticion(string direccion, string nombre);
int buscarParticionPE(string direccion, string nombre);
int buscarParticionL(string direccion, string nombre);

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

    if(delete_ != ""  && add != 0){
        cout<< "ERROR: Parametro -delete|-add demas" << endl;
    } else {
        if(delete_ != ""){
            cout<<"Eliminar particion"<<endl;
            deleteParticion(path, name, delete_);
        } else if(add != 0){
            cout<<"Agregar Quitar particion"<<endl;
            addParticion(path, name, add, unit);
        } else{
            if(type != 0){  //Si especifica tipo de particion
                if(type == 'P'){
                    cout<<"Particion Primaria"<<endl;
                    crearParticionPrimaria(path, name, size, fit, unit);
                } else if(type == 'E'){
                    cout<<"Particion Extendida"<<endl;
                    crearParticionExtendida(path, name, size, fit, unit);
                } else if(type == 'L'){
                    cout<<"Particion Logica"<<endl;
                    crearParticionLogica(path, name, size, fit, unit);
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
                        cout << "*Particion primaria creada con exito*" <<  endl;
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
                        cout << "*Particion primaria creada con exito*" <<  endl;
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
                        cout << "*Particion primaria creada con exito*" <<  endl;
                    }
                } else{
                    cout << "ERROR: ya existe una particion con ese nombre" << endl;
                }

            } else{
                cout << "ERROR: la particion a crear excede el espacio libre" << endl;
            }
        } else{
            cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
        }
    fclose(fp);
    } else{
        cout << "ERROR: no existe el disco" << endl;
    }
}


void crearParticionExtendida(string direccion, string nombre, int size, char fit, char unit){
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
            sizeBytes = size * 1024 * 1024;
        }
    }else{
        sizeBytes = size * 1024;
    }

    FILE *fp;
    MBR masterboot;
    if((fp = fopen(direccion.c_str(), "rb+"))){
        bool particion = false; //particion disponible
        bool extendida = false; //hay particion extendida
        int numParticion = 0;   //numero de particion
        fseek(fp, 0, SEEK_SET);
        fread(&masterboot, sizeof(MBR), 1, fp);
        for(int i = 0; i < 4; i++){
            if (masterboot.mbr_partition[i].part_type == 'E'){
                extendida = true;
                break;
            }
        }
        if(!extendida){
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
                
                cout << "Espacio disponible: " << (masterboot.mbr_size - espacioUsado) <<" Bytes"<< endl;
                cout << "Espacio necesario:  " << sizeBytes << " Bytes" << endl;
                
                //Verificar que haya espacio suficiente para crear la particion
                if((masterboot.mbr_size - espacioUsado) >= sizeBytes){
                    if(!(existeParticion(direccion, nombre))){
                        if(masterboot.mbr_disk_fit == 'F'){
                            masterboot.mbr_partition[numParticion].part_type = 'E';
                            masterboot.mbr_partition[numParticion].part_fit = auxFit;
                            //start
                            if(numParticion == 0){
                                masterboot.mbr_partition[numParticion].part_start = sizeof(masterboot);
                            }else{
                                masterboot.mbr_partition[numParticion].part_start =  masterboot.mbr_partition[numParticion-1].part_start + masterboot.mbr_partition[numParticion-1].part_size;
                            }
                            masterboot.mbr_partition[numParticion].part_size = sizeBytes;
                            masterboot.mbr_partition[numParticion].part_status = '0';
                            strcpy(masterboot.mbr_partition[numParticion].part_name, nombre.c_str());
                            //Se guarda de nuevo el MBR
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            //Se guarda la particion extendida
                            fseek(fp, masterboot.mbr_partition[numParticion].part_start, SEEK_SET);
                            EBR extendedBoot;
                            extendedBoot.part_fit = auxFit;
                            extendedBoot.part_status = '0';
                            extendedBoot.part_start = masterboot.mbr_partition[numParticion].part_start;
                            extendedBoot.part_size = 0;
                            extendedBoot.part_next = -1;
                            strcpy(extendedBoot.part_name, "");
                            fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                            for(int i = 0; i < (sizeBytes - (int)sizeof(EBR)); i++){
                                fwrite(&buffer, 1, 1, fp);
                            }
                            cout << "*Particion extendida creada con exito*"<< endl;
                        }else if(masterboot.mbr_disk_fit == 'B'){
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
                            masterboot.mbr_partition[bestIndex].part_type = 'E';
                            masterboot.mbr_partition[bestIndex].part_fit = auxFit;
                            //start
                            if(bestIndex == 0){
                                masterboot.mbr_partition[bestIndex].part_start = sizeof(masterboot);
                            }else{
                                masterboot.mbr_partition[bestIndex].part_start =  masterboot.mbr_partition[bestIndex-1].part_start + masterboot.mbr_partition[bestIndex-1].part_size;
                            }
                            masterboot.mbr_partition[bestIndex].part_size = sizeBytes;
                            masterboot.mbr_partition[bestIndex].part_status = '0';
                            strcpy(masterboot.mbr_partition[bestIndex].part_name,nombre.c_str());
                            //Se guarda de nuevo el MBR
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            //Se guarda la particion extendida
                            fseek(fp, masterboot.mbr_partition[bestIndex].part_start, SEEK_SET);
                            EBR extendedBoot;
                            extendedBoot.part_fit = auxFit;
                            extendedBoot.part_status = '0';
                            extendedBoot.part_start = masterboot.mbr_partition[bestIndex].part_start;
                            extendedBoot.part_size = 0;
                            extendedBoot.part_next = -1;
                            strcpy(extendedBoot.part_name, "");
                            fwrite(&extendedBoot,sizeof (EBR), 1, fp);
                            for(int i = 0; i < (sizeBytes - (int)sizeof(EBR)); i++){
                                fwrite(&buffer, 1, 1, fp);
                            }
                            cout << "*Particion extendida creada con exito*"<< endl;
                        }else if(masterboot.mbr_disk_fit == 'W'){
                            int  worstIndex = numParticion;
                            for(int i = 0; i < 4; i++){
                                if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size >= sizeBytes)){
                                    if(i != numParticion){
                                        if(masterboot.mbr_partition[worstIndex].part_size < masterboot.mbr_partition[i].part_size){
                                            worstIndex = i;
                                            break;
                                        }
                                    }
                                }
                            }
                            masterboot.mbr_partition[worstIndex].part_type = 'E';
                            masterboot.mbr_partition[worstIndex].part_fit = auxFit;
                            //start
                            if(worstIndex == 0){
                                masterboot.mbr_partition[worstIndex].part_start = sizeof(masterboot);
                            }else{
                                masterboot.mbr_partition[worstIndex].part_start =  masterboot.mbr_partition[worstIndex-1].part_start + masterboot.mbr_partition[worstIndex-1].part_size;
                            }
                            masterboot.mbr_partition[worstIndex].part_size = sizeBytes;
                            masterboot.mbr_partition[worstIndex].part_status = '0';
                            strcpy(masterboot.mbr_partition[worstIndex].part_name, nombre.c_str());
                            //Se guarda de nuevo el MBR
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            //Se guarda la particion extendida
                            fseek(fp, masterboot.mbr_partition[worstIndex].part_start, SEEK_SET);
                            EBR extendedBoot;
                            extendedBoot.part_fit = auxFit;
                            extendedBoot.part_status = '0';
                            extendedBoot.part_start = masterboot.mbr_partition[worstIndex].part_start;
                            extendedBoot.part_size = 0;
                            extendedBoot.part_next = -1;
                            strcpy(extendedBoot.part_name, "");
                            fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                            for(int i = 0; i < (sizeBytes - (int)sizeof(EBR)); i++){
                                fwrite(&buffer, 1, 1, fp);
                            }
                            cout << "*Particion extendida creada con exito*"<< endl;
                        }
                    } else{
                        cout << "ERROR: ya existe una particion con ese nombre" << endl;
                    }
                } else{
                    cout << "ERROR: la particion a crear excede el tamano libre" << endl;
                }
            } else{
                cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
            }
        } else{
            cout << "ERROR: ya existe una particion extendida en este disco" << endl;
        }
    fclose(fp);
    } else{
        cout << "ERROR: no existe el disco" << endl;
    }
}


void crearParticionLogica(string direccion, string nombre, int size, char fit, char unit){
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
            sizeBytes = size * 1024 * 1024;
        }
    }else{
        sizeBytes = size * 1024;
    }

    FILE *fp;
    MBR masterboot;
    if((fp = fopen(direccion.c_str(), "rb+"))){
        int numExtendida = -1;
        fseek(fp, 0, SEEK_SET);
        fread(&masterboot, sizeof(MBR), 1, fp);

        //Verificar si existe una particion extendida
        for(int i = 0; i < 4; i++){
            if(masterboot.mbr_partition[i].part_type == 'E'){
                numExtendida = i;
                break;
            }
        }
        if(!existeParticion(direccion, nombre)){
            if(numExtendida != -1){
                EBR extendedBoot;
                int cont = masterboot.mbr_partition[numExtendida].part_start;
                fseek(fp, cont, SEEK_SET);
                fread(&extendedBoot, sizeof(EBR), 1, fp);
                if(extendedBoot.part_size == 0){    //Si es la primera
                    if(masterboot.mbr_partition[numExtendida].part_size < sizeBytes){
                        cout << "ERROR: la particion logica a crear excede el espacio disponible de la particion extendida " << endl;
                    }else{
                        extendedBoot.part_status = '0';
                        extendedBoot.part_fit = auxFit;
                        extendedBoot.part_start = ftell(fp) - sizeof(EBR); //Para regresar al inicio de la extendida
                        extendedBoot.part_size = sizeBytes;
                        extendedBoot.part_next = -1;
                        strcpy(extendedBoot.part_name, nombre.c_str());
                        fseek(fp, masterboot.mbr_partition[numExtendida].part_start, SEEK_SET);
                        fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                        cout << "*Particion logica creada con exito*"<< endl;
                    }
                }else{
                    while((extendedBoot.part_next != -1) && (ftell(fp) < (masterboot.mbr_partition[numExtendida].part_size + masterboot.mbr_partition[numExtendida].part_start))){
                        fseek(fp, extendedBoot.part_next, SEEK_SET);
                        fread(&extendedBoot, sizeof(EBR), 1, fp);
                    }
                    int espacioNecesario = extendedBoot.part_start + extendedBoot.part_size + sizeBytes;
                    if(espacioNecesario <= (masterboot.mbr_partition[numExtendida].part_size + masterboot.mbr_partition[numExtendida].part_start)){
                        extendedBoot.part_next = extendedBoot.part_start + extendedBoot.part_size;
                        //Escribimos el next del ultimo EBR
                        fseek(fp, ftell(fp) - sizeof (EBR), SEEK_SET);
                        fwrite(&extendedBoot, sizeof(EBR), 1,fp);
                        //Escribimos el nuevo EBR
                        fseek(fp, extendedBoot.part_start + extendedBoot.part_size, SEEK_SET);
                        extendedBoot.part_status = 0;
                        extendedBoot.part_fit = auxFit;
                        extendedBoot.part_start = ftell(fp);
                        extendedBoot.part_size = sizeBytes;
                        extendedBoot.part_next = -1;
                        strcpy(extendedBoot.part_name,nombre.c_str());
                        fwrite(&extendedBoot,sizeof(EBR), 1, fp);
                        cout << "*Particion logica creada con exito*"<< endl;
                    }else{
                        cout << "ERROR: la particion logica a crear excede el espacio disponible de la particion extendida" << endl;
                    }
                }
            }else{
                cout << "ERROR: se necesita una particion extendida para crear la particion logica " << endl;
            }
        }else{
            cout << "ERROR: ya existe una particion con ese nombre" << endl;
        }

    fclose(fp);
    }else{
        cout << "ERROR: no existe el disco" << endl;
    }

}


void deleteParticion(string direccion, string nombre, string typeDelete){
    FILE *fp;
    if((fp = fopen(direccion.c_str(), "rb+"))){
        //Verificar que la particion no este montada
        bool mount = listaMount->buscar(direccion, nombre);
        if(!mount){
            MBR masterboot;
            fseek(fp, 0, SEEK_SET);
            fread(&masterboot, sizeof(MBR), 1, fp);
            int indice = -1;
            int indiceExtendida = -1;
            bool extendida = false;
            string opcion = "";
            char buffer = '\0';
            //Buscamos la particion primaria/extendida
            for(int i = 0; i < 4; i++){
                if((strcmp(masterboot.mbr_partition[i].part_name, nombre.c_str())) == 0){
                    indice = i;
                    if(masterboot.mbr_partition[i].part_type == 'E')
                        extendida = true;
                    break;
                }else if(masterboot.mbr_partition[i].part_type == 'E'){
                    indiceExtendida = i;
                }
            }
            
            cout << "-> ¿Seguro que desea eliminar la particion? Y/N : " ;
            getline(cin, opcion);
            
            if(opcion.compare("Y") == 0 || opcion.compare("y") == 0){
                if(indice != -1){   //Si se encontro en las principales
                    if(!extendida){ //primaria
                        if(typeDelete == "fast"){
                            masterboot.mbr_partition[indice].part_status = '1';
                            strcpy(masterboot.mbr_partition[indice].part_name, "");
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            cout << "*Particion primaria eliminada*" << endl;

                        }else{  //full
                            masterboot.mbr_partition[indice].part_status = '1';
                            strcpy(masterboot.mbr_partition[indice].part_name, "");
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot,sizeof(MBR),1,fp);
                            fseek(fp,masterboot.mbr_partition[indice].part_start, SEEK_SET);
                            fwrite(&buffer, 1, masterboot.mbr_partition[indice].part_size, fp);
                            cout << "*Particion primaria eliminada*" << endl;
                        }
                    }else{  //extendida
                        if(typeDelete == "fast"){
                            masterboot.mbr_partition[indice].part_status = '1';
                            strcpy(masterboot.mbr_partition[indice].part_name, "");
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            cout << "*Particion extendida eliminada*" << endl;
                        }else{//full
                            masterboot.mbr_partition[indice].part_status = '1';
                            strcpy(masterboot.mbr_partition[indice].part_name,"");
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            fseek(fp,masterboot.mbr_partition[indice].part_start, SEEK_SET);
                            fwrite(&buffer, 1, masterboot.mbr_partition[indice].part_size, fp);
                            cout << "*Particion extendida eliminada*" << endl;
                        }
                    }
                }else{  //Si es una particion logica
                    if(indiceExtendida != -1){
                        bool logica = false;    //Bandera para saber si existe
                        EBR extendedBoot;
                        fseek(fp, masterboot.mbr_partition[indiceExtendida].part_start, SEEK_SET);
                        fread(&extendedBoot, sizeof(EBR), 1, fp);
                        if(extendedBoot.part_size != 0){
                            fseek(fp, masterboot.mbr_partition[indiceExtendida].part_start,SEEK_SET);
                            while((fread(&extendedBoot,sizeof(EBR),1,fp))!=0 && (ftell(fp) < (masterboot.mbr_partition[indiceExtendida].part_start + masterboot.mbr_partition[indiceExtendida].part_size))) {
                                if(strcmp(extendedBoot.part_name, nombre.c_str()) == 0 && extendedBoot.part_status != '1'){
                                    logica = true;
                                    break;
                                }else if(extendedBoot.part_next == -1){ //Ya es la ultima y no se encontro
                                    break;
                                }
                            }
                        }
                        if(logica){
                            if(typeDelete == "fast"){
                                extendedBoot.part_status = '1';
                                strcpy(extendedBoot.part_name, "");
                                fseek(fp, ftell(fp)-sizeof(EBR), SEEK_SET);
                                fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                                cout << "*Particion logica eliminada*" << endl;
                            }else{  //full
                                extendedBoot.part_status = '1';
                                strcpy(extendedBoot.part_name, "");
                                fseek(fp, ftell(fp)-sizeof(EBR), SEEK_SET);
                                fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                                fwrite(&buffer, 1, extendedBoot.part_size, fp);
                                cout << "*Particion logica eliminada*" << endl;
                            }
                        }else{
                            cout << "ERROR: no se encuentra la particion a eliminar" << endl;
                        }
                    }else{
                        cout << "ERROR: no se encuentra la particion a eliminar" << endl;
                    }
                }
            }else if(opcion.compare("N") || opcion.compare("n") == 0){
                cout << "Cancelado" << endl;;
            }else{
                cout << "Opcion incorrecta" << endl;
            }
        }else{
            cout << "ERROR: desmontar la particion para poder eliminarla" << endl;
        }
    fclose(fp);
    }else{
        cout << "ERROR: no existe el disco" << endl;
    }
}

void addParticion(string direccion, string nombre, int add, char unit){
    int sizeBytes = 0;
    string tipo = "";

    if(add > 0) tipo = "add";
    else add = add*(-1);

    if(unit == 'm')
        sizeBytes = add * 1024 * 1024;
    else if(unit == 'k')
        sizeBytes = add * 1024;
    else
        sizeBytes = add;

    FILE *fp;
    if((fp = fopen(direccion.c_str(), "rb+"))){
        //Verificar que la particion no este montada
        bool mount = listaMount->buscar(direccion, nombre);
        if(!mount){
            MBR masterboot;
            fseek(fp, 0, SEEK_SET);
            fread(&masterboot, sizeof(MBR), 1, fp);
            int indice = -1;
            int indiceExtendida = -1;
            bool extendida = false;
            //Buscamos la particion primaria/extendida
            for(int i = 0; i < 4; i++){
                if((strcmp(masterboot.mbr_partition[i].part_name, nombre.c_str())) == 0){
                    indice = i;
                    if(masterboot.mbr_partition[i].part_type == 'E')
                        extendida = true;
                    break;
                }else if(masterboot.mbr_partition[i].part_type == 'E'){
                    indiceExtendida = i;
                }
            }
            if(indice != -1){   //Si se encontro en las principales
                if(!extendida){ //Primaria
                    if(tipo == "add"){  //Agregar
                        //Verificar que exista espacio libre a la derecha
                        if(indice != 3){
                            int p1 = masterboot.mbr_partition[indice].part_start + masterboot.mbr_partition[indice].part_size;
                            int p2 = masterboot.mbr_partition[indice+1].part_start;
                            if((p2 - p1) != 0){ //Hay fragmentacion
                                int fragmentacion = p2 - p1;
                                if(fragmentacion >= sizeBytes){
                                    masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size + sizeBytes;
                                    fseek(fp,0,SEEK_SET);
                                    fwrite(&masterboot, sizeof(MBR), 1, fp);
                                    cout << "*Se agrego espacio a la particion*" << endl;
                                } else{
                                    cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                                }
                            } else{  //Espacio no usado
                                if(masterboot.mbr_partition[indice + 1].part_status == '1'){
                                    if(masterboot.mbr_partition[indice + 1].part_size >= sizeBytes){
                                        masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size + sizeBytes;
                                        masterboot.mbr_partition[indice + 1].part_size = (masterboot.mbr_partition[indice + 1].part_size - sizeBytes);
                                        masterboot.mbr_partition[indice + 1].part_start = masterboot.mbr_partition[indice + 1].part_start + sizeBytes;
                                        fseek(fp, 0, SEEK_SET);
                                        fwrite(&masterboot, sizeof(MBR), 1, fp);
                                        cout << "*Se agrego espacio a la particion*" << endl;
                                    }else{
                                        cout << "ERROR: no hay espacio suficiente para agregarlo a la particion" << endl;
                                    }
                                }
                                cout<< "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion"<<endl;
                            }
                        } else{
                            int p = masterboot.mbr_partition[indice].part_start + masterboot.mbr_partition[indice].part_size;
                            int total = masterboot.mbr_size + (int)sizeof(MBR);
                            if((total-p) != 0){
                                int fragmentacion = total - p;
                                if(fragmentacion >= sizeBytes){
                                    masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size + sizeBytes;
                                    fseek(fp, 0, SEEK_SET);
                                    fwrite(&masterboot, sizeof(MBR), 1, fp);
                                    cout << "*Se agrego espacio a la particion*" << endl;
                                }else{
                                    cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                                }
                            }else{
                                cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                            }
                        }
                    }else{  //Quitar espacio
                        //Que no borre la particion
                        if(sizeBytes >= masterboot.mbr_partition[indice].part_size){
                            cout << "ERROR: quitarle esta cantidad de espacio a la particion la borraria" << endl;
                        } else{
                            masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size - sizeBytes;
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&masterboot, sizeof(MBR), 1, fp);
                            cout << "*Se quito espacio a la particion*" << endl;
                        }
                    }
                } else{//Extendida
                    if(tipo == "add"){  //Agregar
                        //Verificar que exista espacio libre a la derecha
                        if(indice != 3){
                            int p1 = masterboot.mbr_partition[indice].part_start + masterboot.mbr_partition[indice].part_size;
                            int p2 = masterboot.mbr_partition[indice+1].part_start;
                            if((p2 - p1) != 0){ //Hay fragmentacion
                                int fragmentacion = p2-p1;
                                if(fragmentacion >= sizeBytes){
                                    masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size + sizeBytes;
                                    fseek(fp, 0, SEEK_SET);
                                    fwrite(&masterboot, sizeof(MBR), 1, fp);
                                    cout << "*Se agrego espacio a la particion*" << endl;
                                }else{
                                    cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                                }
                            }else{
                                if(masterboot.mbr_partition[indice + 1].part_status == '1'){
                                    if(masterboot.mbr_partition[indice + 1].part_size >= sizeBytes){
                                        masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size + sizeBytes;
                                        masterboot.mbr_partition[indice + 1].part_size = (masterboot.mbr_partition[indice + 1].part_size - sizeBytes);
                                        masterboot.mbr_partition[indice + 1].part_start = masterboot.mbr_partition[indice + 1].part_start + sizeBytes;
                                        fseek(fp, 0, SEEK_SET);
                                        fwrite(&masterboot, sizeof(MBR), 1, fp);
                                        cout << "*Se agrego espacio a la particion*" << endl;
                                    } else{
                                        cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                                    }
                                }
                            }
                        } else{
                            int p = masterboot.mbr_partition[indice].part_start + masterboot.mbr_partition[indice].part_size;
                            int total = masterboot.mbr_size + (int)sizeof(MBR);
                            if((total-p) != 0){ //Hay fragmentacion
                                int fragmentacion = total - p;
                                if(fragmentacion >= sizeBytes){
                                    masterboot.mbr_partition[indice].part_size = masterboot.mbr_partition[indice].part_size + sizeBytes;
                                    fseek(fp, 0, SEEK_SET);
                                    fwrite(&masterboot, sizeof(MBR), 1, fp);
                                    cout << "*Se agrego espacio a la particion*" << endl;
                                } else{
                                    cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                                }
                            } else{
                                cout << "ERROR: no hay suficiente espacio a la derecha para agregarlo a la particion" << endl;
                            }
                        }
                    } else{ //Quitar espacio
                        //Que no borre la particion
                        if(sizeBytes >= masterboot.mbr_partition[indiceExtendida].part_size){
                            cout << "ERROR: quitarle esta cantidad de espacio a la particion la borraria" << endl;
                        } else{
                            EBR extendedBoot;
                            fseek(fp, masterboot.mbr_partition[indiceExtendida].part_start, SEEK_SET);
                            fread(&extendedBoot, sizeof(EBR), 1, fp);
                            while((extendedBoot.part_next != -1) && (ftell(fp) < (masterboot.mbr_partition[indiceExtendida].part_size + masterboot.mbr_partition[indiceExtendida].part_start))){
                                fseek(fp,extendedBoot.part_next, SEEK_SET);
                                fread(&extendedBoot, sizeof(EBR), 1, fp);
                            }
                            int ultimaLogica = extendedBoot.part_start+extendedBoot.part_size;
                            int aux = (masterboot.mbr_partition[indiceExtendida].part_start + masterboot.mbr_partition[indiceExtendida].part_size) - sizeBytes;
                            if(aux > ultimaLogica){ //No toca ninguna logica
                                masterboot.mbr_partition[indiceExtendida].part_size = masterboot.mbr_partition[indiceExtendida].part_size - sizeBytes;
                                fseek(fp, 0, SEEK_SET);
                                fwrite(&masterboot, sizeof(MBR), 1, fp);
                                cout << "*Se quito espacio a la particion*" << endl;
                            } else{
                                cout << "ERROR: quitar este espacio dañaria la particion logica" << endl;
                            }
                        }
                    }
                }
            } else{  //Posiblemente logica
                if(indiceExtendida != -1){
                    int logica = buscarParticionL(direccion, nombre);
                    if(logica != -1){
                        if(tipo == "add"){
                            //Verificar que exista espacio libre a su derecha
                            EBR extendedBoot;
                            fseek(fp, logica, SEEK_SET);
                            fread(&extendedBoot, sizeof(EBR), 1, fp);

                        } else{  //Quitar
                            //Verificar que no la elimine
                            EBR extendedBoot;
                            fseek(fp, logica, SEEK_SET);
                            fread(&extendedBoot, sizeof(EBR), 1, fp);
                            if(sizeBytes >= extendedBoot.part_size){
                                cout << "ERROR: quitar este espacio eliminaria la particion logica" << endl;
                            } else{
                                extendedBoot.part_size = extendedBoot.part_size - sizeBytes;
                                fseek(fp, logica, SEEK_SET);
                                fwrite(&extendedBoot, sizeof(EBR), 1, fp);
                                cout << "*Se quito espacio a la particion*" << endl;
                            }
                        }
                    } else{
                        cout << "ERROR: no se encuentra la particion a redimensionar" << endl;
                    }
                } else{
                    cout << "ERROR: no se encuentra la particion a redimensionar" << endl;
                }
            }
        } else{
             cout << "ERROR: desmontar la particion para poder redimensionar" << endl;
        }
    fclose(fp);
    } else{
        cout << "ERROR: el disco donde se desea agregar/quitar unidades no existe" << endl;
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

int buscarParticionPE(string direccion, string nombre){
    FILE *fp;
    if((fp = fopen(direccion.c_str(),"rb+"))){
        MBR masterboot;
        fseek(fp, 0, SEEK_SET);
        fread(&masterboot, sizeof(MBR), 1, fp);
        for(int i = 0; i < 4; i++){
            if(masterboot.mbr_partition[i].part_status != '1'){
                if(strcmp(masterboot.mbr_partition[i].part_name, nombre.c_str()) == 0){
                    return i;
                }
            }
        }
    }
    return -1;
}

int buscarParticionL(string direccion, string nombre){
    FILE *fp;
    if((fp = fopen(direccion.c_str(),"rb+"))){
        int extendida = -1;
        MBR masterboot;
        fseek(fp, 0, SEEK_SET);
        fread(&masterboot, sizeof(MBR), 1, fp);
        for(int i = 0; i < 4; i++){
            if(masterboot.mbr_partition[i].part_type == 'E'){
                extendida = i;
                break;
            }
        }
        if(extendida != -1){
            EBR extendedBoot;
            fseek(fp, masterboot.mbr_partition[extendida].part_start, SEEK_SET);
            while(fread(&extendedBoot, sizeof(EBR), 1, fp) !=0 && (ftell(fp) < masterboot.mbr_partition[extendida].part_start + masterboot.mbr_partition[extendida].part_size)){
                if(strcmp(extendedBoot.part_name, nombre.c_str()) == 0){
                    return (ftell(fp) - sizeof(EBR));
                }
            }
        }
        fclose(fp);
    }
    return -1;
}