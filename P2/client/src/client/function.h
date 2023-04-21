#include "comunication.h"
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h> 

char menu_crear_aldeano();
bool principal_menu(int server_socket);
char menu_subir_nivel();
char * menu_robar();
char * menu_espiar();
char * menu_atacar();