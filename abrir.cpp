#include <iostream>
#include <fstream>
#include <string>

void inicializarLectura(std::ifstream&& stream);
std::string siguienteLinea();
void cerrarLectura();

int main(int argc, char *argv[]){

    // Fallo en la apertura y busqueda de archivo
    if (argc < 2){
        std:: cerr << "Error: No se encontro el archivo" << std::endl;
        return -1;    
    }

    std::ifstream archivo;
    archivo.open(argv[1], std::ios::in);

    //Fallo en no se pudo abrir el archivo
    if (!archivo.is_open()){
        std:: cerr <<"Error: No se puede abrir el archivo" << argv[1] << std::endl;
        return -1;
    }

    std::cout << "Archivo" << argv[1] << "abierto correctamente" << std::endl;

    inicializarLectura(std::move(archivo));

    cerrarLectura();
    
    return 0;
}