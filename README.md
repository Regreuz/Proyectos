Hasta el momento existen tres módulos.

1. abrir.cpp: En este solamente el programa abre el archivo e indica si se pudo o no realizar la accion.
2. lectura.cpp: En esta primero se inicializa la lectura, mandará la siguiente línea que solicite el lexer.cpp, cierra cuando se acabó la lectura.
3. lexer.cpp: Esta es la más extensa, en ella se almacenan nuestras estructuras de datos para guardar nuestros Tokens y compararlos

El flujo de manera sencilla sería algo tal que:

1. abrir.cpp. Abre el archivo, no lee nada.
1.1 Inicializa l lectura con inicializarLectura(std::move(archivo)), puesto que se transfiere al stream ya abierto.
   2. Lectura.cpp guarda el ifstream en 'ArchivoFuente'
   3. Cada llamada a la siguiente línea hace un std::getLine()
   4. Avanza el curso del stream hacía adelante, por lo que solo hay una lectura.
