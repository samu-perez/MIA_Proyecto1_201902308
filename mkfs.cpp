#include <iostream>
#include <list>
#include <math.h>

#include "nodo.h"
#include "listaMount.cpp"

using namespace std;

extern ListaMount *listaMount;

void formatearEXT2(int inicio, int tamano, string direccion);

void MKFS(Nodo *raiz){
    string id = "";
    string type = "";
    int fs = 2;

    list<Nodo> :: iterator it;
    for(it = raiz->hijos.begin(); it != raiz->hijos.end(); ++it){

        if(it->tipo == "id"){
            id = it->valor;
        } else if(it->tipo == "type"){
            type = it->valor;
        } else if(it->tipo == "fs"){
            if(it->valor == "2fs") fs = 2;
            else if(it->valor == "3fs") fs = 3;
        } 
    }

    if(id != ""){   //Parametro obligatorio
        NodoMount *aux = listaMount->getMount(id);
        if(aux != NULL){
            int indice = buscarParticionPE(aux->direccion,aux->nombre);
            if(indice != -1){
                MBR masterboot;
                FILE *fp = fopen(aux->direccion.c_str(),"rb+");
                fread(&masterboot, sizeof(MBR), 1, fp);
                int inicio = masterboot.mbr_partition[indice].part_start;
                int tamano = masterboot.mbr_partition[indice].part_size;
                if(fs == 2)
                    formatearEXT2(inicio, tamano, aux->direccion);
                else
                    cout <<"Formato EXT3" <<endl;
                    //formatearEXT2(inicio,tamano,aux->direccion);
                fclose(fp);
            } else{
                indice = buscarParticionL(aux->direccion, aux->nombre);
            }
        } else
            cout << "ERROR: no se encuentra montada una particion con ese id" << endl;
    } else
        cout << "ERROR: parametro -id no definido" << endl;
    
}

void formatearEXT2(int inicio, int tamano, string direccion){
    double n = (tamano - static_cast<int>(sizeof(SuperBloque)))/(4 + static_cast<int>(sizeof(InodoTable)) +3*static_cast<int>(sizeof(BloqueArchivo)));
    int nEstructuras = static_cast<int>(floor(n));  //Numero de inodos
    int nBloques = 3*nEstructuras;

    SuperBloque super;
    super.s_filesystem_type = 2;
    super.s_inodes_count = nEstructuras;
    super.s_blocks_count = nBloques;
    super.s_free_blocks_count = nBloques -2;
    super.s_free_inodes_count = nEstructuras -2;
    super.s_mtime = time(nullptr);
    super.s_umtime = 0;
    super.s_mnt_count = 0;
    super.s_magic = 0xEF53;
    super.s_inode_size = sizeof(InodoTable);
    super.s_block_size = sizeof(BloqueArchivo);
    super.s_first_ino = 2;
    super.s_first_blo = 2;
    super.s_bm_inode_start = inicio + static_cast<int>(sizeof(SuperBloque));
    super.s_bm_block_start = inicio + static_cast<int>(sizeof(SuperBloque)) + nEstructuras;
    super.s_inode_start = inicio + static_cast<int>(sizeof (SuperBloque)) + nEstructuras + nBloques;
    super.s_block_start = inicio + static_cast<int>(sizeof(SuperBloque)) + nEstructuras + nBloques + (static_cast<int>(sizeof(InodoTable))*nEstructuras);

    InodoTable inodo;
    BloqueCarpeta bloque;

    char buffer1 = '0';
    char buffer2 = '1';
    char buffer3 = '2';

    FILE *fp = fopen(direccion.c_str(),"rb+");

    //SUPERBLOQUE
    fseek(fp,inicio,SEEK_SET);
    fwrite(&super,sizeof(SuperBloque),1,fp);
   
    //BITMAP DE INODOS
    for(int i = 0; i < nEstructuras; i++){
        fseek(fp,super.s_bm_inode_start + i,SEEK_SET);
        fwrite(&buffer1,sizeof(char),1,fp);
    }
    
    //bit para / y users.txt en BM
    fseek(fp,super.s_bm_inode_start,SEEK_SET);
    fwrite(&buffer2,sizeof(char),1,fp);
    fwrite(&buffer2,sizeof(char),1,fp);
    
    //BITMAP DE BLOQUES
    for(int i = 0; i < nBloques; i++){
        fseek(fp,super.s_bm_block_start + i,SEEK_SET);
        fwrite(&buffer1,sizeof(char),1,fp);
    }
    
    //Bit para / y users.txt en BM
    fseek(fp,super.s_bm_block_start,SEEK_SET);
    fwrite(&buffer2,sizeof(char),1,fp);
    fwrite(&buffer3,sizeof(char),1,fp);
    
    //Inodo para carpeta root
    inodo.i_uid = 1;
    inodo.i_gid = 1;
    inodo.i_size = 0;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);
    inodo.i_block[0] = 0;
    for(int i = 1; i < 15;i++)
        inodo.i_block[i] = -1;
    inodo.i_type = '0';
    inodo.i_perm = 664;
    fseek(fp,super.s_inode_start,SEEK_SET);
    fwrite(&inodo,sizeof(InodoTable),1,fp);
    
    //Bloque para carpeta root
    strcpy(bloque.b_content[0].b_name,"."); //Actual (el mismo)
    bloque.b_content[0].b_inodo=0;

    strcpy(bloque.b_content[1].b_name,"..");    //Padre
    bloque.b_content[1].b_inodo=0;

    strcpy(bloque.b_content[2].b_name,"users.txt");
    bloque.b_content[2].b_inodo=1;

    strcpy(bloque.b_content[3].b_name,".");
    bloque.b_content[3].b_inodo=-1;
    fseek(fp,super.s_block_start,SEEK_SET);
    fwrite(&bloque,sizeof(BloqueCarpeta),1,fp);

    //Inodo para users.txt
    inodo.i_uid = 1;
    inodo.i_gid = 1;
    inodo.i_size = 27;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);
    inodo.i_block[0] = 1;
    for(int i = 1; i < 15;i++){
        inodo.i_block[i] = -1;
    }
    inodo.i_type = '1';
    inodo.i_perm = 755;
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(InodoTable)),SEEK_SET);
    fwrite(&inodo,sizeof(InodoTable),1,fp);
    
    //Bloque para users.txt
    BloqueArchivo archivo;
    memset(archivo.b_content,0,sizeof(archivo.b_content));
    strcpy(archivo.b_content,"1,G,root\n1,U,root,root,123\n");
    fseek(fp,super.s_block_start + static_cast<int>(sizeof(BloqueCarpeta)),SEEK_SET);
    fwrite(&archivo, sizeof(BloqueArchivo), 1, fp);

    cout << "*EXT2 - Particion formateado con exito*" << endl;
    fclose(fp);
}