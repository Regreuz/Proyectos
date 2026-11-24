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

//Prototipos del lexer
Token siguienteToken();
void imprimirToken(const Token& t);
const char* nombreTipoToken(TipoToken t);

//Estructuras del parser

typedef enum {
    OP_REGISTRO,
    OP_INMEDIATO,
    OP_ETIQUETA,
    OP_MEMORIA
} TipoOperando;

typedef struct{
    TipoOperando tipo;
    std::string texto;
    int tamanio;
} Operando;

typedef struct{
    std::string mnemonico;
    std::vector<Operando> operandos;
    size_t numeroLinea;
} Instruccion;

//Transformar todo a mayusculas para evitar que funciones solo para un caso

static std::string Mayusculas(const std::string& s){
    std::string r = s;
    for (char& c : r) c = (char)std::toupper((unsigned char)c);
    return r;
}

//Tamanio de los registros, cuantos bits

int tamR(const std::string& reg_in){
    const std::string reg = Mayusculas(reg_in);

    if (reg == "AL" || reg == "AH" || reg == "BL" || reg == "BH" ||
        reg == "CL" || reg == "CH" || reg == "DL" || reg == "DH"){
            return 8;
        }

    if (reg == "AX" || reg == "BX" || reg == "CX" || reg == "DX" ||
        reg == "SI" || reg == "DI" || reg == "SP" || reg == "BP"){
            return 16;
        }
    
    return 32;
}

//Cantidad de operandos por mnemonicos
int cantidadOperandos(const std::string& mnemonico){
    if (mnemonico == "MOV" || mnemonico == "ADD" || mnemonico == "SUB" || mnemonico == "CMP"){
        return 2;
    }
    if (mnemonico == "JMP"){
        return 1;
    }
    return 0;
}

//Lectura de un operando
bool leerOperando(const std::vector<Token>& tokens, size_t& i, Operando& op, std::string& error){
    if (tokens[i].tipo == TOKEN_REGISTRO){
        op.tipo = OP_REGISTRO;
        op.texto = tokens[i].lexema;
        op.tamanio = tamR(tokens[i].lexema);
        i++;
        return true;
    }

    if (tokens[i].tipo == TOKEN_NUMERO){
        op.tipo = OP_INMEDIATO;
        op.texto = tokens[i].lexema;
        op.tamanio = 32;
        i++;
        return true;
    }

    if (tokens[i].tipo == TOKEN_IDENTIFICADOR){
        op.tipo = OP_ETIQUETA;
        op.texto = tokens[i].lexema;
        op.tamanio = 0;
        i++;
        return true;
    }

    if (tokens[i].tipo == TOKEN_CORCHETE_ABRE){
        std::string texto = "[";
        i++;

        if (tokens[i].tipo == TOKEN_CORCHETE_CIERRA){
            error = "Memoria vacia: []";
            return false;
        }

        while(tokens[i].tipo != TOKEN_CORCHETE_CIERRA){
            if (tokens[i].tipo == TOKEN_FIN_LINEA || tokens[i].tipo == TOKEN_FIN_ARCHIVO){
                error = "falta ']' en el operando de memoria";
                return false;
            }

            texto += tokens[i].lexema;
            i++;
        }

        texto += "]";
        i++;

        op.tipo = OP_MEMORIA;
        op.texto = texto;
        op.tamanio = 32;
        return true;
    }

    error = "Operando invalido: " + tokens[i].lexema;
    return false; 
}

//Analisis de la instruccion de inicio a fin de la linea
bool analizarInstruccion(const std::vector<Token>& tokens, Instruccion& instr, std::string& error){
    
    if (tokens.empty()) return false;

    if (tokens[0].tipo == TOKEN_FIN_LINEA) return true; //Para la linea en blanco

    if (tokens[0].tipo != TOKEN_MNEMONICO){
        error = "Se esperaba un mnemonico";
        return false;
    }

    instr.mnemonico = tokens[0].lexema;
    instr.numeroLinea = tokens[0].numeroLinea;

    const int c_op = cantidadOperandos(instr.mnemonico);

    //Hay algun desconocido en la linea? Prevenimos
    size_t i = 1;

    while (tokens[i].tipo != TOKEN_FIN_LINEA && tokens[i].tipo != TOKEN_FIN_ARCHIVO){
        if (tokens[i].tipo == TOKEN_DESCONOCIDO){
            error = "Simbolo no identificado: " + tokens[i].lexema;
            return false;
        }
        i++;
    }

    i = 1;

    //Para el operando 1

    if (c_op >= 1){
        if(tokens[i].tipo == TOKEN_FIN_LINEA){
            error = "Faltan operandos";
            return false;
        }

        Operando op1;
        if(!leerOperando(tokens, i, op1, error)) return false;
        instr.operandos.push_back(op1);
    }

    //Para el operando 2
    if (c_op == 2){
        if (tokens[i].tipo != TOKEN_COMA){
            error = "Falta la coma entre los operandos";
            return false;
        }
        i++;

        if (tokens[i].tipo == TOKEN_FIN_LINEA){
            error = "Faltan operandos";
            return false;
        }

        Operando op2;
        if (!leerOperando(tokens, i, op2, error)) return false;
        instr.operandos.push_back(op2);
    }

    //Prevenir tokens excedentes
    if (tokens[i].tipo != TOKEN_FIN_LINEA && tokens[i].tipo != TOKEN_FIN_ARCHIVO){
        error = "Se ha sobresado la cantidad de Tokens";
        return false;
    }
    return true;
}

//Parseo, nos dara true si la linea es valida, false si hay errores
bool parsearLinea(const std::vector<Token>& tokens, Instruccion* instrSalida, std::string& error){
    //Casos que debemos cubrir 
    if (tokens.empty()) return true;
    if (tokens[0].tipo == TOKEN_FIN_LINEA) return true;
    if (tokens[0].tipo == TOKEN_COMENTARIO) return true;
    if (tokens[0].tipo == TOKEN_DIRECTIVA) return true;

    //Para las etiquetas
    if (tokens[0].tipo == TOKEN_IDENTIFICADOR && tokens.size() >= 2 && tokens[1].tipo == TOKEN_DOS_PUNTOS){
        return true;
    } 

    //Instrucciones
    if (tokens[0].tipo == TOKEN_MNEMONICO){
        Instruccion instr;
        if (!analizarInstruccion(tokens, instr, error)) return false;
        if (instrSalida) *instrSalida = instr;
        return true;
    }

    error = "Error, la linea no puede reconocerse";
    return false;
}

//Parser que exige tokens al lexer

bool parsearArchivo(std::vector<Instruccion>& instrucciones){

    bool ok = true;
    std::vector<Token> lineaActual;

    while (true){
        Token t = siguienteToken();

        if (t.tipo == TOKEN_FIN_ARCHIVO){
            //Primero procesar si la utima linea quedo pendiente
            if (!lineaActual.empty()){
                std::string error;
                Instruccion instr;

                if(!parsearLinea(lineaActual, &instr, error)){
                    std::cerr << "[parse] linea" << t.numeroLinea << ":" << error << std::endl;
                    ok = false;
                } else if(instr.operandos.size() > 0 || !instr.mnemonico.empty()){
                    instrucciones.push_back(instr);
                }
                lineaActual.clear();
            }
            break;
        }

        lineaActual.push_back(t);

        if (t.tipo == TOKEN_FIN_LINEA){
            std::string error;
            Instruccion instr;
            if(!parsearLinea(lineaActual, &instr, error)){
                std::cerr << "[parse] linea " << t.numeroLinea << ": " << error << std::endl;
                ok = false;
            } else if(instr.operandos.size() > 0 || !instr.mnemonico.empty()){
                instrucciones.push_back(instr);
            }
            lineaActual.clear();
        }
    }

    return ok;
}
