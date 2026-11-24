#include <iostream>
#include <fstream>
#include <string>
#include <vector> //Estructura de datos dinamica array flexible
#include <regex> //Comprueba si una expresion regular coincide con la cadena destino
#include <cctype> //Para clasificar y transformar caracteres
#include <cstddef> //Manejo de tipos y macros

//Tipos de tokens
typedef enum {
    TOKEN_MNEMONICO,
    TOKEN_REGISTRO,
    TOKEN_IDENTIFICADOR,
    TOKEN_NUMERO,
    TOKEN_COMA,
    TOKEN_CORCHETE_ABRE,
    TOKEN_CORCHETE_CIERRA,
    TOKEN_OPERADOR,
    TOKEN_DIRECTIVA,
    TOKEN_COMENTARIO, 
    TOKEN_FIN_LINEA,
    TOKEN_FIN_ARCHIVO,
    TOKEN_DESCONOCIDO,
    TOKEN_DOS_PUNTOS
} TipoToken;

//Estructura del token
typedef struct {
    TipoToken tipo;
    std::string lexema;
    size_t numeroLinea;
    size_t columna;
} Token;

//Prototipos de lectura
std::string siguienteLinea();
bool esFinDeArchivo(const std::string& linea);
int lineasLeidas();

//Tablas con palabras reservadas
static const std::vector<std::string> MNEMONICOS = {
    "MOV", "ADD", "SUB", "CMP", "JMP"
};

static const std::vector<std::string> REGISTROS = {
    "EAX", "EBX", "ECX", "EDX", "ESI", "EDI", "ESP", "EBP"
};

static const std::vector<std::string> DIRECTIVAS = {
    "BITS", "SECTION", "GLOBAL", "DB", "DW", "DD"
};

//Reconocer con regex los patrones de los tokens

//Numero decimal
static const std::regex RE_DECIMAL(R"(^[0-9]+$)");

// Número hexadecimal: 0x1F  o  1Ah  o  0FFh
static const std::regex RE_HEX_0X(R"(^0[xX][0-9a-fA-F]+$)");
static const std::regex RE_HEX_H (R"(^[0-9][0-9a-fA-F]*[hH]$)");

// Identificador / etiqueta
static const std::regex RE_IDENTIFICADOR(R"(^[A-Za-z_][A-Za-z0-9_]*$)");

//Operadores aritmeticos
static bool esOperador(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

//Delimitadores
static bool esComa(char c) {
    return c == ',';
}

static bool esCorcheteAbre(char c){
    return c == '[';
}

static bool esCorcheteCierra(char c){
    return c == ']';
}


//Calsificacion de la palabra
static TipoToken clasificacionPalabra(const std::string& lex){
    
    //Verificar si es un mnemonico
    for (const auto& mnemonico : MNEMONICOS){
        if (lex == mnemonico){
            return TOKEN_MNEMONICO;
        }
    }

    //Verificar si es un registro
    for (const auto& registro : REGISTROS){
        if (lex == registro){
            return TOKEN_REGISTRO;
        }
    }

    //Verificar si es una directiva
    for (const auto& directiva : DIRECTIVAS){
        if (lex == directiva){
            return TOKEN_DIRECTIVA;
        }
    }

    //Verificar si es un numero decimal
    if (std::regex_match(lex, RE_DECIMAL)){
        return TOKEN_NUMERO;
    }

    //Verificar si es un numero hexadecimal
    if (std::regex_match(lex, RE_HEX_0X) || std::regex_match(lex, RE_HEX_H)){
        return TOKEN_NUMERO;
    }

    //Verificar si es un identificador
    if (std::regex_match(lex, RE_IDENTIFICADOR)){
        return TOKEN_IDENTIFICADOR;
    }

    //Si no coincide con ningun tipo conocido, es desconocido
    return TOKEN_DESCONOCIDO;
}


// Tokenizacion de la linea completita

static void tokenizarLinea(const std::string& linea, size_t numeroLinea, std::vector<Token>& salida){
    const size_t n = linea.size();
    size_t i = 0;

    while (i < n){
        const char c = linea[i];
        
        //Espacios en blanco ignorarlos
        if (c==' ' || c=='\t'){
            i++;
            continue;
        }

        //Comentarios
        if (c == ';'){
            Token t;
            t.tipo = TOKEN_COMENTARIO;
            t.lexema = linea.substr(i);
            t.numeroLinea = numeroLinea;
            t.columna = i + 1;
            salida.push_back(t);
            i = n;
            break;
        }

        if (c == ':'){
            salida.push_back({TOKEN_DOS_PUNTOS, std::string(1, c), numeroLinea, i + 1});
            i++;
            continue;    
        }

        //Delimitadores
        if (esComa(c)){
            salida.push_back({TOKEN_COMA, std::string(1, c), numeroLinea, i + 1});
            i++;
            continue;
        }

        if (esCorcheteAbre(c)){
            salida.push_back({TOKEN_CORCHETE_ABRE, std::string(1, c), numeroLinea, i + 1});
            i++;
            continue;
        }

        if (esCorcheteCierra(c)){
            salida.push_back({TOKEN_CORCHETE_CIERRA, std::string(1, c), numeroLinea, i + 1});
            i++;
            continue;
        }

        //Operadores
        if (esOperador(c)){
            salida.push_back({TOKEN_OPERADOR, std::string(1, c), numeroLinea, i + 1});
            i++;
            continue;
        }

        //Palabras, letras, digitos, '_'
        if (std::isalnum((unsigned char)c) || c == '_'){
            size_t inicio = i;
            while (i < n) {
                char cc = linea[i];
                if (std::isalnum((unsigned char)cc) || cc == '_'){
                    i++;
                } else {
                    break;
                }
            }
            std::string lex = linea.substr(inicio, i - inicio);
            Token t;
            t.tipo = clasificacionPalabra(lex);
            t.lexema = lex;
            t.numeroLinea = numeroLinea;
            t.columna = inicio + 1;
            salida.push_back(t);
            continue;
        }
        
        //Para cualquier otro caracter desconocido
        {
            Token t;
            t.tipo = TOKEN_DESCONOCIDO;
            t.lexema = std::string(1, c);
            t.numeroLinea = numeroLinea;
            t.columna = i + 1;
            salida.push_back(t);
        }
        i++;
    }

    //Fin de linea
    Token eol;
    eol.tipo = TOKEN_FIN_LINEA;
    eol.lexema = "\n";
    eol.numeroLinea = numeroLinea;
    eol.columna = n + 1;
    salida.push_back(eol);
}


static std::vector<Token> bufferTokens;
static size_t indiceBuffer = 0;
static bool finDetectado = false;

static void emitirFinDeArchivo(size_t linea, size_t col){
    Token t;
    t.tipo = TOKEN_FIN_ARCHIVO;
    t.lexema = "";
    t.numeroLinea = linea;
    t.columna = col;
    bufferTokens.push_back(t);
    finDetectado = true;
}

Token siguienteToken(){

    //Hay Tokens disponibles en el buffer
    if (indiceBuffer < bufferTokens.size()){
        return bufferTokens[indiceBuffer++];
    }

    //Si se ha detectado el final del archivo
    if (finDetectado){
        Token t;
        t.tipo = TOKEN_FIN_ARCHIVO;
        t.lexema = "";
        t.numeroLinea = bufferTokens.empty() ? 0: bufferTokens.back().numeroLinea;
        t.columna = bufferTokens.empty() ? 0: bufferTokens.back().columna;
        return t;
    }

    //Pedir linea nueva de lectura
    std::string linea = siguienteLinea();
    if (esFinDeArchivo(linea)){
        emitirFinDeArchivo((size_t)lineasLeidas(), 1);
        return bufferTokens[indiceBuffer++];
    }

    //Limpiar el buffer y tokenizar la nueva linea
    bufferTokens.clear();
    indiceBuffer = 0;
    tokenizarLinea(linea, (size_t)lineasLeidas(), bufferTokens);
    return bufferTokens[indiceBuffer++];   

}