#include <iostream>
#include <fstream>

int main(int argc, char *argv[]){
    if (argc < 2){
        std:: cerr << "Error: No se encontro el archivo" << std::endl;
        return 1;    
    }

    std::ifstream archivo(argv[1]);
    if (!archivo.is_open()){
        std:: cerr <<"Error: No se puede abrir el archivo" << std::endl;
        return 1;
    }

    
}