%{
    #include <iostream>
    #include "scanner.h"
    #include "../nodo.h"

    using namespace std;

    extern int yylineno;
    extern char *yytext;
    extern int yyfila;
    extern int yycolum;
    extern Nodo *raiz; // Raiz del arbol

    int yyerror(const char* mensaje){
        cout<<yytext<<" "<<mensaje<<" "<<yyfila<<" "<<yycolum<<std::endl;
        return 0;
    }
%}

%union{
    char text [400];
    class Nodo *nodo;
}

%token <text> mkdisk
%token <text> rmdisk
%token <text> fdisk
%token <text> exec
%token <text> size
%token <text> fit
%token <text> unit
%token <text> path
%token <text> type
%token <text> name
%token <text> del
%token <text> add
%token <text> bf
%token <text> ff
%token <text> wf
%token <text> fast
%token <text> full

%token <text> igual
%token <text> num
%token <text> caracter
%token <text> cadena
%token <text> identificador
%token <text> extension
%token <text> ruta

%type <nodo> INICIO
%type <nodo> COMANDO
%type <nodo> MKDISK
%type <nodo> PARAMETRO_MK
%type <nodo> RMDISK
%type <nodo> FDISK
%type <nodo> PARAMETRO_FK
%type <nodo> AJUSTE
%type <nodo> SCRIPT

%start INICIO

%%
INICIO: COMANDO {raiz = $$;};

COMANDO: mkdisk MKDISK {
        $$ = new Nodo("MKDISK", "");
        $$->add(*$2);
    }
    | RMDISK { $$ = $1; }
    | fdisk FDISK {
        $$ = new Nodo("FDISK", "");
        $$->add(*$2);
    }
    | SCRIPT {$$ = $1;};

MKDISK: MKDISK PARAMETRO_MK {
        $$ = $1;
        $$->add(*$2);
    }
    | PARAMETRO_MK {
        $$ = new Nodo("PARAMETRO", "");
        $$->add(*$1);
    };

PARAMETRO_MK: size igual num {$$ = new Nodo("size", $3);}
    | fit igual AJUSTE {
        $$ = new Nodo("fit", "");
        $$->add(*$3);
    }
    | unit igual caracter {$$ = new Nodo("unit", $3);}
    | path igual cadena {$$ = new Nodo("path", $3);}
    | path igual ruta {$$ = new Nodo("path", $3);};

RMDISK: rmdisk path igual ruta {
        $$ = new Nodo("RMDISK", "");
        Nodo *ruta = new Nodo("path", $4);
        $$->add(*ruta);
    }
    | rmdisk path igual cadena {
        $$ = new Nodo("RMDISK", "");
        Nodo *ruta = new Nodo("path", $4);
        $$->add(*ruta);
    };

FDISK: FDISK PARAMETRO_FK {
        $$ = $1;
        $$->add(*$2);
    }
    | PARAMETRO_FK {
        $$ = new Nodo("PARAMETRO", "");
        $$->add(*$1);
    };

PARAMETRO_FK: PARAMETRO_MK { $$ = $1; }
    | type igual caracter { $$ = new Nodo("type", $3); }
    | del igual fast { $$ = new Nodo("delete", "fast"); }
    | del igual full { $$ = new Nodo("delete", "full"); }
    | name igual identificador { $$ = new Nodo("name", $3); }
    | name igual cadena { $$ = new Nodo("name", $3); }
    | add igual num { $$ = new Nodo("add", $3); };

AJUSTE: bf {$$ = new Nodo("ajuste", "bf");}
    | ff {$$ = new Nodo("ajuste", "ff");}
    | wf {$$ = new Nodo("ajuste", "wf");};

SCRIPT: exec path igual cadena {
        $$ = new Nodo("EXEC", "");
        Nodo *path = new Nodo("path", $4);
        $$->add(*path);
    }
    | exec path igual ruta {
        $$ = new Nodo("EXEC", "");
        Nodo *path = new Nodo("path", $4);
        $$->add(*path);
    }; 