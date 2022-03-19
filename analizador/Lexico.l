%option case-insensitive
%option noyywrap
%option nounput

%{
  #include <iostream>
  #include "parser.h"
  #include "../nodo.h"

  Nodo *raiz;

  using namespace std;

  int yyfila = 1;
  int yycolum = 0;
%}

entero -?[0-9]+
letra [a-zA-Z]
caracter ({letra}|{entero})
id ({letra}|{entero}|"_")+
cadena "\""[^"\""]*"\""
extension "."{id}
diagonal "/"
ruta ({diagonal}{id})+({diagonal}{id}{extension})
pwd ({letra}|{entero}|[!$@+*])+

%x COM

%%
INITIAL
{
    "#"         { BEGIN COM; }

    "mkdisk"    { return mkdisk; }
    "rmdisk"    { return rmdisk; }
    "fdisk"     { return fdisk; }
    "mount"     { return mount; }
    "unmount"   { return unmount; }
    "rep"       { return rep; }
    "exec"      { return exec; }
    "-size"     { return size; }
    "-fit"      { return fit; }
    "-unit"     { return unit; }
    "-path"     { return path;}
    "-type"     { return type; }
    "-name"     { return name; }
    "-delete"   { return del; }
    "-add"      { return add; }
    "-id"       { return id; }
    "bf"        { return bf; }
    "ff"        { return ff; }
    "wf"        { return wf; }
    "Fast"      { return fast; }
    "Full"      { return full; }
    "mbr"       { return mbr; }
    "disk"      { return disk; }
 
    "mkfs"       { return mkfs; }
    "login"      { return login; }
    "logout"     { return logout; }
    "pause"      { return pausa; }
    "-fs"        { return fs; }
    "2fs"        { return fs2; }
    "3fs"        { return fs3; }
    "-usuario"   { return usuario; }
    "-password"  { return password; }
    "sb"         { return sb; }
    
    "="         {return igual;}

    {entero}    {strcpy(yylval.text, yytext); return num;}
    {caracter}  {strcpy(yylval.text, yytext); return caracter;}
    {cadena}    {strcpy(yylval.text, yytext); return cadena;}
    {id}        {strcpy(yylval.text, yytext); return identificador;}
    {ruta}      {strcpy(yylval.text, yytext); return ruta;}
    {extension} {strcpy(yylval.text, yytext); return extension;}
    {pwd}  {strcpy(yylval.text, yytext); return pwd;}

    [[:blank:]] {}

    .           {cout<<"Error lexico, no se esperaba token:"<<yytext<<endl;}

}

<COM>"\n"        { BEGIN INITIAL; };
<COM>.           { };

%%