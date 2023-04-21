#include <unistd.h>
#include <stdio.h>
#include "function.h"
#include <string.h>
#include <stdbool.h> 

bool principal_menu(int server_socket){
    printf("Menu Principal\n");
    printf("Escoger una accion:\n");
    printf("[0] Mostrar Informacion \n");
    printf("[1] Crear Aldeano\n");
    printf("[2] Subir de Nivel\n");
    printf("[3] Atacar\n");
    printf("[4] Espiar\n");
    printf("[5] Robar\n");
    printf("[6] Pasar\n");
    printf("[7] Rendirse\n");
    char c[2];
    char *c_r = calloc(2, sizeof(char));
    scanf("%s", c);
    for (int i=0; i<2; i++){
        c_r[i]=c[i];
    }
    //char c = getchar();
    //getchar();
    if (c_r[0] == '0'){
        client_send_message(server_socket, 10, "mostar info");
        //mostrar info
    } else if (c_r[0] == '1'){
        char id_crear_aldeano = menu_crear_aldeano();
        if (id_crear_aldeano == '0'){
            client_send_message(server_socket, 11, "minero");
        } else if (id_crear_aldeano == '1'){
            client_send_message(server_socket, 11, "agricultor");
        } else if (id_crear_aldeano == '2'){
            client_send_message(server_socket, 11, "ingeniero");
        } else if (id_crear_aldeano == '3'){
            client_send_message(server_socket, 11, "ataque");
        } else if (id_crear_aldeano == '4'){
            client_send_message(server_socket, 11, "defensa");
        }
        

    } else if (c_r[0] == '2'){
        char id_subir_nivel = menu_subir_nivel();
        printf("id %c\n", id_subir_nivel);
        if (id_subir_nivel == '0'){
            client_send_message(server_socket, 13, "minero");
        } else if (id_subir_nivel == '1'){
            client_send_message(server_socket, 13, "agricultor");
        } else if (id_subir_nivel == '2'){
            client_send_message(server_socket, 13, "ingeniero");
        } else if (id_subir_nivel == '3'){
            client_send_message(server_socket, 13, "ataque");
        } else if (id_subir_nivel == '4'){
            client_send_message(server_socket, 13, "defensa");
        }
    } else if (c_r[0] == '3'){
        char * id_atacar = menu_atacar();
        client_send_message(server_socket, 14, id_atacar);
        //printf("id %c\n", id);
    } else if (c_r[0] == '4'){
        char * id_espiar = menu_espiar();
        //printf("el id es %s\n", id);
        //printf("no falla\n");
        client_send_message(server_socket, 15, id_espiar);
    } else if (c_r[0] == '5'){
        char* id_robar = menu_robar();
        client_send_message(server_socket, 16, id_robar);
    }
    else if (c_r[0] == '6'){
        printf("Fin del turno\n");
        client_send_message(server_socket, 17, "pasar");
        return false;
    }
    else if (c_r[0] == '7'){
        printf("rendirse\n");
        client_send_message(server_socket, 18, "rendirse");
        return false;
    }
    free(c_r);
    return true;
    
}

char menu_crear_aldeano(){
    printf("Menu Crear Aldeano\n");
    printf("Escoger rol a asignar...\n");
    printf("[0] Minero\n");
    printf("[1] Agricultor\n");
    printf("[2] Ingeniero\n");
    printf("[3] Guerrero\n");
    char x[2];
    char *x_r = calloc(2, sizeof(char));
    scanf("%s", x);
    for (int i = 0; i < 2; i++)
    {
        x_r[i] = x[i];
    }  
    //char x = getchar();
    //getchar();
    return x_r[0];
}

char menu_subir_nivel(){
    printf("Menu Subir Nivel\n");
    printf("Escoger rol a subir de nivel...\n");
    printf("[0] Minero\n");
    printf("[1] Agricultor\n");
    printf("[2] Ingeniero\n");
    printf("[3] Ataque\n");
    printf("[4] Defensa\n");
    char x[2];
    char *x_r = calloc(2, sizeof(char));
    scanf("%s", x);
    for (int i = 0; i < 2; i++)
    {
        x_r[i] = x[i];
    }  
    //char x = getchar();
    //getchar();
    return x_r[0];
}

char menu_atacar(){
    printf("Menu atacar\n");
    printf("Escoger jugador a atacar...\n");
    //printf("[0] \n");
    //VARIABLE SEGUN NUMERO DE JUGADORES
    //char x = getchar();
    //getchar();
    char x[2];
    char *x_r = calloc(2, sizeof(char));
    scanf("%s", x);
    for (int i = 0; i < 2; i++)
    {
        x_r[i] = x[i];
    }    
    //printf("ESCANEO OK\n");
    return x_r[0];
}

char * menu_espiar(){
    printf("\nMenu Espiar\n");
    printf("Ingresa ID de jugador a espiar:\n");
    //VARIABLE SEGUN NUMERO DE JUGADORES
    char x[2];
    char *x_r = calloc(2, sizeof(char));
    scanf("%s", x);
    for (int i = 0; i < 2; i++)
    {
        x_r[i] = x[i];
    }    
    //printf("ESCANEO OK\n");
    return x_r[0];
}

char* menu_robar(){
    printf("\nMenu Robar\n");
    printf("Escoger jugador a robar...\n");
    char x[2];
    char *x_r = calloc(2, sizeof(char));
    scanf("%s", x);
    for (int i=0; i<2; i++){
        x_r[i]=x[i];
    }
    printf("\nQuieres robar comida u oro?\n");
    printf("[0] Comida \n");
    printf("[1] Oro \n");
    char y[2];
    char *y_r = calloc(2, sizeof(char));
    scanf("%s", y);
    for (int j=0; j<2; j++){
        y_r[j]=y[j];
    }
    char *elemento_final = calloc(2, sizeof(char));
    elemento_final[0]=x_r[0];
    elemento_final[1]=y_r[0];
    elemento_final[2]=0;
    return elemento_final;
}



//int main (int argc, char *argv[]){
    //principal_menu();
    //crear_aldeano();
    //return 0;
//}