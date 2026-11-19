#include <stdio.h>
#include <stdlib.h>

int main(int arg, char *argv[]) {

    if (arg < 2){
        printf("Error");
        return -1;
    }

    FILE *archivo = fopen(argv[1], "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo.\n");
        return -1;
    }

    char linea[256]; //Longitud de maximo 100?
    int num_linea = 0;
    while(fgets(linea, 256, archivo) != NULL){
        num_linea++;
    }

    if (ferror(archivo)){
        perror("Error en la lectura del archivo. \n");
        fclose(archivo);
        return -1;
    }

    fclose(archivo);

    return 0;
}

