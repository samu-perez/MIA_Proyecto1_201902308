#include <iostream>
#include <list>
#include <regex>

#include "nodo.h"
#include "listaMount.cpp"

using namespace std;

extern ListaMount *listaMount;
extern bool loggedin;
extern Sesion sesionActual;

int log_in(string direccion, string nombre, string user, string password);
int validarDatos(string user, string password, string direccion);
char logicaFit(string direccion, string nombre);
int buscarGrupo(string name);

void LOGIN(Nodo *raiz){
    string user = "";
    string password = "";
    string id = "";

    list<Nodo> :: iterator it;
    for(it = raiz->hijos.begin(); it != raiz->hijos.end(); ++it){

        if(it->tipo == "usuario"){
            user = it->valor;
            user = regex_replace(user, regex("\""), "");
        } else if(it->tipo == "password"){
            password = it->valor;
            password = regex_replace(password, regex("\""), "");
        } else if(it->tipo == "id"){
            id = it->valor;
        } 
    }

    
    if(user != ""){
        if(password != ""){
            if(id != ""){
                if(!loggedin){
                    NodoMount *aux = listaMount->getMount(id);
                    if(aux != nullptr){
                        int res = log_in(aux->direccion, aux->nombre, user, password);
                        if(res == 1){
                            loggedin = true;
                                cout << "*Sesion iniciada con exito*" << endl;
                        }else if(res == 2)
                            cout << "ERROR: Contrasena incorrecta" << endl;
                        else if(res == 0)
                            cout << "ERROR: Usuario no encontrado" << endl;
                    }else
                        cout << "ERROR: no se encuentra ninguna particion montada con ese id " << endl;
                }else
                     cout << "ERROR: cierre sesion para poder iniciar otra sesion" << endl;
            }else
                cout << "ERROR: parametro -id no definido" << endl;
        }else
            cout << "ERROR: parametro -password no definido" << endl;
    }else
        cout << "ERROR: parametro -usuario no definido" << endl;
    
}

int log_in(string direccion, string nombre, string user, string password){
    int indice = buscarParticionPE(direccion, nombre);
    if(indice != -1){
        MBR masterboot;
        SuperBloque super;
        InodoTable inodo;
        FILE *fp = fopen(direccion.c_str(),"rb+");
        fread(&masterboot, sizeof(MBR), 1, fp);
        fseek(fp, masterboot.mbr_partition[indice].part_start, SEEK_SET);
        fread(&super, sizeof(SuperBloque), 1, fp);
        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(InodoTable)), SEEK_SET);
        fread(&inodo, sizeof(InodoTable), 1, fp);
        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(InodoTable)), SEEK_SET);
        inodo.i_atime = time(nullptr);
        fwrite(&inodo, sizeof(InodoTable), 1, fp);
        fclose(fp);
        sesionActual.inicioSuper = masterboot.mbr_partition[indice].part_start;
        sesionActual.fit = masterboot.mbr_partition[indice].part_fit;
        sesionActual.inicioJournal = masterboot.mbr_partition[indice].part_start + static_cast<int>(sizeof(SuperBloque));
        sesionActual.tipo_sistema = super.s_filesystem_type;
        return validarDatos(user, password, direccion);
    }else{
        indice = buscarParticionL(direccion, nombre);
        if(indice != -1){
            SuperBloque super;
            InodoTable inodo;
            FILE *fp = fopen(direccion.c_str(),"rb+");
            fseek(fp,indice + static_cast<int>(sizeof(EBR)),SEEK_SET);
            fread(&super, sizeof(SuperBloque), 1, fp);
            fseek(fp, super.s_inode_start + static_cast<int>(sizeof(InodoTable)), SEEK_SET);
            fread(&inodo, sizeof(InodoTable), 1, fp);
            fseek(fp, super.s_inode_start + static_cast<int>(sizeof(InodoTable)), SEEK_SET);
            inodo.i_atime = time(nullptr);
            fwrite(&inodo, sizeof(InodoTable), 1, fp);
            fclose(fp);
            sesionActual.inicioSuper = indice + static_cast<int>(sizeof(EBR));
            sesionActual.fit = logicaFit(direccion, nombre);
            return validarDatos(user, password, direccion);
        }
    }
    return 0;
}

int validarDatos(string user, string password, string direccion){
    FILE *fp = fopen(direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    SuperBloque super;
    InodoTable inodo;

    fseek(fp,sesionActual.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,fp);
    //Leemos el inodo del archivo users.txt
    fseek(fp, super.s_inode_start + static_cast<int>(sizeof(InodoTable)), SEEK_SET);
    fread(&inodo,sizeof(InodoTable),1,fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BloqueArchivo archivo;
            fseek(fp, super.s_block_start, SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo, sizeof(BloqueArchivo), 1, fp);
            }
            strcat(cadena, archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    char *token = strtok_r(cadena,"\n",&end_str);
    while(token != nullptr){
        char id[2];
        char tipo[2];
        string group;
        char user_[12];
        char password_[12];
        char *end_token;
        char *token2 = strtok_r(token, ",", &end_token);
        strcpy(id,token2);
        if(strcmp(id,"0") != 0){    //Verificar que no sea un U/G eliminado
            token2=strtok_r(nullptr, ",", &end_token);
            strcpy(tipo,token2);
            if(strcmp(tipo,"U") == 0){
                token2 = strtok_r(nullptr, ",", &end_token);
                group = token2;
                token2 = strtok_r(nullptr, ",", &end_token);
                strcpy(user_,token2);
                token2 = strtok_r(nullptr, ",", &end_token);
                strcpy(password_,token2);
                if(strcmp(user_,user.c_str()) == 0){
                    if(strcmp(password_, password.c_str()) == 0){
                        sesionActual.direccion = direccion;
                        sesionActual.id_user = atoi(id);
                        sesionActual.id_grp = buscarGrupo(group);
                        return 1;
                    }else
                        return 2;
                }
            }
        }
        token = strtok_r(nullptr, "\n", &end_str);
    }

    return 0;
}

char logicaFit(string direccion, string nombre){
    FILE *fp;
    if((fp = fopen(direccion.c_str(),"rb+"))){
        int extendida = -1;
        MBR masterboot;
        fseek(fp,0,SEEK_SET);
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
            while(fread(&extendedBoot,sizeof(EBR),1,fp)!=0 && (ftell(fp) < masterboot.mbr_partition[extendida].part_start + masterboot.mbr_partition[extendida].part_size)){
                if(strcmp(extendedBoot.part_name, nombre.c_str()) == 0){
                    return extendedBoot.part_fit;
                }
            }
        }
        fclose(fp);
    }
    return -1;
}

int buscarGrupo(string name){
    FILE *fp = fopen(sesionActual.direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    SuperBloque super;
    InodoTable inodo;

    fseek(fp, sesionActual.inicioSuper, SEEK_SET);
    fread(&super,sizeof(SuperBloque), 1, fp);
    //Leemos el inodo del archivo users.txt
    fseek(fp, super.s_inode_start + static_cast<int>(sizeof(InodoTable)), SEEK_SET);
    fread(&inodo, sizeof(InodoTable), 1, fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BloqueArchivo archivo;
            fseek(fp, super.s_block_start, SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo, sizeof(BloqueArchivo), 1, fp);
            }
            strcat(cadena, archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    char *token = strtok_r(cadena, "\n", &end_str);
    while(token != nullptr){
        char id[2];
        char tipo[2];
        char group[12];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);
        strcpy(id,token2);
        if(strcmp(id,"0") != 0){//Verificar que no sea un U/G eliminado
            token2 = strtok_r(nullptr,",",&end_token);
            strcpy(tipo,token2);
            if(strcmp(tipo,"G") == 0){
                strcpy(group,end_token);
                if(strcmp(group,name.c_str()) == 0)
                    return atoi(id);
            }
        }
        token = strtok_r(nullptr, "\n", &end_str);
    }

    return -1;
}

void LOGOUT(){
    if(loggedin){
        loggedin = false;
        sesionActual.id_user = -1;
        sesionActual.direccion = "";
        sesionActual.inicioSuper = -1;
        cout << "*Sesion finalizada*" << endl;
    }else
        cout << "ERROR: no existe una sesion activa" << endl;
}