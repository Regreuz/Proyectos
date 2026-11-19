#include <iostream>
#include <fstream>
#include <string>

//Caracter especial para el fin del archivo
static const char FIN_ARCHIVO = '\0';
//Tamaño maximo de la linea a leer
static const std::size_t MAX_LINEA = 256;

//Estado del lector
static std::ifstream archivoFuente;
static bool archivoAbierto = false;
static int numLineas = 0;

//Inicalizacion del lector
void inicializarLectura(std::ifstream&& stream){
    archivoFuente = std::move(stream);
    archivoAbierto = archivoFuente.is_open();
    numLineas = 0;

    if (!archivoAbierto){
    std::cerr << "Error: No se puede abrir el archivo" << std::endl;
    return;
    }
}

// Entrega la siguiente linea del archivo
std::string siguienteLinea(){
    if (!archivoAbierto || !archivoFuente.is_open()){
        std::cerr << "Error: No se puede leer el archivo" << std::endl;
        return std::string(1, FIN_ARCHIVO);
    }
    //Lectura de la siguiente linea
    std::string linea;
    if (std::getline(archivoFuente, linea)){

        numLineas++;
        return linea;
    }

    //Para por si hay un error durante la lectura
    if (archivoFuente.bad()){
        std::cerr << "Error: No se puede leer el archivo" << std::endl;
    }

    //Por el EOF o error
    return std::string(1, FIN_ARCHIVO);
}

//Boleano que indica si es que si ya se acabo el archivo
bool esFinDeArchivo(const std::string& linea){
    return linea.size() == 1 && linea[0] == FIN_ARCHIVO;
}

//Solo nos dice que tantas lineas hemos leido
int lineasLeidas(){
    return numLineas;
}

//Para cerrar el archivo y liberar recursos
void cerraLectura(){
    if (archivoFuente.is_open()){
        archivoFuente.close();
    }
    archivoAbierto = false;
}