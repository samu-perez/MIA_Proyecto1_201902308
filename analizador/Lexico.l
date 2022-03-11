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
id {letra}({letra}|{entero}|"_")*
cadena "\""[^"\""]*"\""
extension "."{id}
diagonal "/"
ruta ({diagonal}{id})+({diagonal}{id}{extension})

%%
{
    "mkdisk"    { return mkdisk; }
    "rmdisk"    { return rmdisk; }
    "fdisk"     { return fdisk; }
    "exec"      { return exec; }
    "-size"     { return size; }
    "-fit"      { return fit; }
    "-unit"     { return unit; }
    "-path"     { return path;}
    "-type"     { return type; }
    "-name"     { return name; }
    "-delete"   { return del; }
    "-add"      { return add; }
    "bf"        { return bf; }
    "ff"        { return ff; }
    "wf"        { return wf; }
    "Fast"      { return fast; }
    "Full"      { return full; }

    "="         {return igual;}

    {entero}    {strcpy(yylval.text, yytext); return num;}
    {caracter}  {strcpy(yylval.text, yytext); return caracter;}
    {cadena}    {strcpy(yylval.text, yytext); return cadena;}
    {id}        {strcpy(yylval.text, yytext); return identificador;}
    {ruta}      {strcpy(yylval.text, yytext); return ruta;}
    {extension} {strcpy(yylval.text, yytext); return extension;}

    [[:blank:]] {}

    .           {cout<<"Error lexico, no se esperaba token:"<<yytext<<endl;}

}
%%