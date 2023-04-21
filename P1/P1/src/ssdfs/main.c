#include <stdio.h>
#include "os_API.h"
#include <stdlib.h>

int LIFE;
char* DISK_NAME;
int main(int argc, char *argv[])
{
  printf("Hello P1!\n");

  os_mount(argv[1], 5000);                 // se monta el disco 
  os_bitmap(0);                            // Se imprime todo el bitmap
  os_lifemap(0, 256);                      // Entrega el lifemap de las paginas 0 a 255
  os_tree();                               // Entrega el arbol de directorios
  char* filename = "~/ssoo/test.jpg";      // Dirextorio existente
  //char* filename = "~/ssoo/facebook.png";  // Directorio inexistente
  //char* filename = "~/facebook.png";       // Dirextorio existente
  //char* filename = "~/facebook.jpg";       // Directorio inexistente
  os_exists(filename);                     // ¿Existe o no el directorio?
  //os_unload("~/dir1", "./");
  //os_trim(1);
  //os_load("./dir1");
  //os_unload("~/amogus2.mp4", "./amogus3.mp4");

}
