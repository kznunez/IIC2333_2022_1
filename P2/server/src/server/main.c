#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include "comunication.h"
#include "conection.h"
#include "../structs/jugador.h"

int n_jugadores = 0;
int ready = 0;
Jugador* jugadores_array[4];
int sockets_array[4];
int server_socket;
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_t threads[3];


void* common_thread (void *atr){
  printf("id recibido: %d\n", (int)atr);
  int id = (int) atr;
  char *welcome = (char*)malloc(24 * sizeof(char));
  sprintf(welcome, "Bienvenido Cliente %d!!", id);
  server_send_message(sockets_array[id], 1, welcome);
  free(welcome);
  pthread_mutex_unlock(&lock);
  pthread_mutex_lock(&lock2);
  n_jugadores++;
  pthread_mutex_unlock(&lock2);
  int msg_code = server_receive_id(sockets_array[id]);
  char * client_name = server_receive_payload(sockets_array[id]);
  jugadores_array[id] = jugador_init(client_name, id);
  // printf("%s\n", client_name);
  printf("nombre jugador %d: %s\n", jugadores_array[id]->id, jugadores_array[id]->nombre);
  server_send_message(sockets_array[0], 0, client_name);
  free(client_name);

  for (int i = 0; i < 9; i++)
  {
    char* message = "Elija rol de su aldeano:\n[1] Agricultor\n[2] Minero\n[3] Ingeniero\n[4] Guerrero\n";
    server_send_message(sockets_array[id], 2, message);
    int msg_code = server_receive_id(sockets_array[id]);
    char * type_char = server_receive_payload(sockets_array[id]);
    int type = atoi(type_char);
    asignar_aldeano(jugadores_array[id], type);
    free(type_char);
  }

  pthread_mutex_lock(&lock2);
  ready++;
  pthread_mutex_unlock(&lock2);

  pthread_exit(NULL);
}

void* creador_threads(void *atr){
  for (int i = 0; i < 3; i++)
  {
    pthread_mutex_lock(&lock);
    int id = i+1;
    sockets_array[i+1]= get_client(server_socket);
    printf("id es: %d \n", id);
    pthread_create(&threads[i], NULL, common_thread, (void*)id);
  }
}

int main(int argc, char *argv[]){
  int PORT;
  char* IP;
  if (argc != 5)
  {
    printf("No se pasaron suficientes argumentos\n");
    return 1;
  }else
  {
    for (int i = 1; i < 5; i = i+2)
    {
      if (strcmp(argv[i], "-p") == 0)
      {
        PORT = atoi(argv[i+1]);
      }else if (strcmp(argv[i], "-i") == 0)
      {
        IP = argv[i+1];
      } 
    }    
  }
  printf("se inicializa el servidor\n");
  
  // Se prepara socket de servidor
  server_socket = prepare_socket(IP, PORT);
  sockets_array[0] = get_client(server_socket);
  pthread_mutex_lock(&lock2);
  n_jugadores++;
  pthread_mutex_unlock(&lock2);
  pthread_t creador;
  pthread_create(&creador, NULL, creador_threads, (void*)NULL);

  char *welcome = (char*)malloc(30 * sizeof(char));
  sprintf(welcome, "Bienvenido Cliente lider %d!!", 0);
  server_send_message(sockets_array[0], 1, welcome);
  free(welcome);
  int msg_code = server_receive_id(sockets_array[0]);
  char * client_name = server_receive_payload(sockets_array[0]);
  jugadores_array[0] = jugador_init(client_name, 0);
  // printf("%s\n", client_name);
  printf("nombre jugador %d: %s\n", jugadores_array[0]->id, jugadores_array[0]->nombre);
  free(client_name);
  sockets_array[0] = sockets_array[0];
  server_send_message(sockets_array[0], 4, "Elija las profecsiones de sus 9 primeros aldeanos\n");
  for (int i = 0; i < 9; i++)
  {
    char* message = (char*)malloc(66 * sizeof(char));
    sprintf(message, "Aldeano %d:\n[1] Agricultor\n[2] Minero\n[3] Ingeniero\n[4] Guerrero\n", i);
    server_send_message(sockets_array[0], 2, message);
    free(message);
    int msg_code = server_receive_id(sockets_array[0]);
    char * type_char = server_receive_payload(sockets_array[0]);
    int type = atoi(type_char);
    free(type_char);
    asignar_aldeano(jugadores_array[0], type);
  }
  server_send_message(sockets_array[0], 3, "Atenti");

  pthread_mutex_lock(&lock2);
  ready++;
  pthread_mutex_unlock(&lock2);
  
  while (1)
  {
    int msg_code = server_receive_id(sockets_array[0]);
    char * client_message = server_receive_payload(sockets_array[0]);
    free(client_message);
    if (msg_code == 0)
    {
      if(n_jugadores == ready)
      {
        server_send_message(sockets_array[0], 6, "cortar thread");
        for (int i = 0; i < n_jugadores; i++)
        {
          server_send_message(sockets_array[i], 5, "Comienza el juego");
        }
        pthread_cancel(creador);
        pthread_detach(creador);
        for (int i = 0; i < n_jugadores-1; i++)
        {
          pthread_detach(threads[i]);
        }
        
        break;
      }else
      {
        server_send_message(sockets_array[0], 3, "Faltan jugadores por estar listos.");
      }
    }
  }
  
  char* game_begin = "inicio el juego";
  char* response2 = recolectar_recursos(jugadores_array[0]);
  server_send_message(sockets_array[0], 19, response2);
  free(response2);
  server_send_message(sockets_array[0], 1, game_begin);
  
  int my_attention = 0;
  while (1)
  {
    // Se obtiene el paquete del cliente 1
    int msg_code = server_receive_id(sockets_array[my_attention]);

    if (msg_code == 1) //El cliente me envió un mensaje a mi (servidor)
    {
      //printf("entre a code 1\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      free(client_message);

    }
    else if (msg_code == 2){ //El cliente le envía un mensaje al otro cliente
      printf("entre a code 2\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("Servidor traspasando desde %d a %d el mensaje: %s\n", my_attention+1, ((my_attention+1)%4)+1, client_message);

      // Mi atención cambia al otro socket
      my_attention = (my_attention + 1) % n_jugadores;
      server_send_message(sockets_array[my_attention], 2, client_message);
      free(client_message);
    }
    else if (msg_code == 10) //El cliente me envió un mensaje a mi (servidor) para MOSTRAR INFO
    {
      printf("entre a code 10 \n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: %s\n", my_attention+1, client_message);
      int comida = jugadores_array[my_attention]->comida;
      int oro = jugadores_array[my_attention]->oro;
      int ciencia = jugadores_array[my_attention]->ciencia;
      int n_agricultores = jugadores_array[my_attention]->n_agricultores;
      int nivel_agricultores = jugadores_array[my_attention]->nivel_agricultores;
      int n_mineros = jugadores_array[my_attention]->n_mineros;
      int nivel_minero = jugadores_array[my_attention]->nivel_minero;
      int n_guerreros = jugadores_array[my_attention]->n_guerreros;
      int nivel_ataque = jugadores_array[my_attention]->nivel_ataque;
      int n_ingenieros = jugadores_array[my_attention]->n_ingenieros;
      int nivel_ingenieros = jugadores_array[my_attention]->nivel_ingenieros;
      char* server_message = calloc(2000,sizeof(char));
      sprintf(server_message, "MOSTRANDO INFORMACION\n\nINFORMACION RECURSOS\n- Comida: %i\n- Oro: %i\n- Ciencia: %i\nINFORMACION ALDEANOS\n- Agricultores: %i - Nivel: %i\n- Mineros: %i - Nivel: %i\n- Guerreros: %i - Nivel: %i\n- Ingenieros: %i - Nivel: %i\n", comida, oro, ciencia, n_agricultores, nivel_agricultores, n_mineros, nivel_minero, n_guerreros, nivel_ataque, n_ingenieros, nivel_ingenieros);
      //sprintf(server_message, "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\n", comida, oro, ciencia, n_agricultores, nivel_agricultores, n_mineros, nivel_minero, n_guerreros, nivel_guerreros, n_ingenieros, nivel_ingenieros);
      server_send_message(sockets_array[my_attention], 10, server_message);
      free(server_message);
      free(client_message);
  
    }
    else if (msg_code == 11) //El cliente me envió un mensaje a mi (servidor) para CREAR ALDEANO
    {
      printf("entre a code 11\n");
      char * client_message1 = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d creara un %s\n", my_attention+1, client_message1);
      if (strcmp(client_message1,"minero") == 0){
        bool answer = crear_aldeano(jugadores_array[my_attention], 1);
        if (answer){
          char * client_message = "Minero creado\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
        else{
          char * client_message = "recursos insuficientes\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
      } else if (strcmp(client_message1,"agricultor") == 0){
        bool answer = crear_aldeano(jugadores_array[my_attention], 2);
        if (answer){
          char * client_message = "Agricultor creado\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
        else{
          char * client_message = "recursos insuficientes\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
      } else if (strcmp(client_message1,"ingeniero") == 0){
        bool answer = crear_aldeano(jugadores_array[my_attention], 3);
        if (answer){
          char * client_message = "Ingeniero creado\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
        else{
          char * client_message = "recursos insuficientes\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
      } else if (strcmp(client_message1,"guerrero") == 0){
        printf("jugador %i\n", jugadores_array[my_attention+1]->id);
        bool answer = crear_aldeano(jugadores_array[my_attention], 4);
        if (answer){
          char * client_message = "Guerrero creado\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
        else{
          char * client_message = "recursos insuficientes\n";
          server_send_message(sockets_array[my_attention], 11, client_message);
        }
      }      
      free(client_message1);
    }
    else if (msg_code == 13) //El cliente me envió un mensaje a mi (servidor) para SUBIR NIVEL
    {
      printf("entre a code 13 \n");
      printf("recursos comida %i, ciencia%i, oro %i\n", jugadores_array[my_attention]->comida, jugadores_array[my_attention]->ciencia, jugadores_array[my_attention]->oro);
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d quiere subir niver a %s\n", my_attention+1, client_message);
      if (strcmp(client_message,"minero") == 0){
        int n = subir_nivel(jugadores_array[my_attention],1);
        if (n == 0){
          server_send_message(sockets_array[my_attention], 11, "se aumento el nivel minero");
        }
        else if (n == 1){
          server_send_message(sockets_array[my_attention], 11, "nivel maximo, no se puede aumentar mas nivel minero");
        }
        else if (n== 2){
          server_send_message(sockets_array[my_attention], 11, "recursos insuficientes para aumentar nivel minero");
        }        
      }
      else if (strcmp(client_message,"agricultor") == 0){
        int n = subir_nivel(jugadores_array[my_attention],2);
        if (n == 0){
          server_send_message(sockets_array[my_attention], 11, "se aumento el nivel agricultores");
        }
        else if (n == 1){
          server_send_message(sockets_array[my_attention], 11, "nivel maximo, no se puede aumentar mas nivel agricultores");
        }
        else if (n== 2){
          server_send_message(sockets_array[my_attention], 11, "recursos insuficientes para aumentar nivel agricultores");
        }
      }
      else if (strcmp(client_message,"ingeniero") == 0){
        int n = subir_nivel(jugadores_array[my_attention],3);
        if (n == 0){
          server_send_message(sockets_array[my_attention], 11, "se aumento el nivel ingeniero");
        }
        else if (n == 1){
          server_send_message(sockets_array[my_attention], 11, "nivel maximo, no se puede aumentar mas nivel ingenieros");
        }
        else if (n== 2){
          server_send_message(sockets_array[my_attention], 11, "recursos insuficientes para aumentar nivel ingenieros");
        }
      }
      else if (strcmp(client_message,"ataque") == 0){
        int n = subir_nivel(jugadores_array[my_attention],4);
        if (n == 0){
          server_send_message(sockets_array[my_attention], 11, "se aumento el nivel ataque");
        }
        else if (n == 1){
          server_send_message(sockets_array[my_attention], 11, "nivel maximo, no se puede aumentar mas nivel ataque");
        }
        else if (n== 2){
          server_send_message(sockets_array[my_attention], 11, "recursos insuficientes para aumentar nivel ataque");
        }
      }
      else if (strcmp(client_message,"defensa") == 0){
        int n = subir_nivel(jugadores_array[my_attention],5);
        if (n == 0){
          server_send_message(sockets_array[my_attention], 11, "se aumento el nivel defensa");
        }
        else if (n == 1){
          server_send_message(sockets_array[my_attention], 11, "nivel maximo, no se puede aumentar mas nivel defensa");
        }
        else if (n== 2){
          server_send_message(sockets_array[my_attention], 11, "recursos insuficientes para aumentar nivel defensa");
        }
      }
      // Le enviamos la respuesta
      free(client_message);
    }
    if (msg_code == 14) //El cliente me envió un mensaje a mi (servidor) ATACAR
    {
      printf("entre a code 14\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: atacar a jugador con id %s\n", my_attention+1, client_message);
      int client_message_int = atoi(client_message);
      free(client_message);
      //printf("el jugador a atacar es el con id: %i\n");
      if (jugadores_array[client_message_int]->eliminado==false){
        int response = atacar(jugadores_array[my_attention], jugadores_array[client_message_int]);
        if (response==1){
          server_send_message(sockets_array[my_attention], 14, "\nATAQUE EXITOSO\n");
          server_send_message(sockets_array[client_message_int], 14, "\nHAZ SIDO ELIMINADO\n");
          printf("El jugador %i ha eliminado al jugador %i\n", jugadores_array[my_attention]->id, jugadores_array[client_message_int]->id);
        }
        else {
          server_send_message(sockets_array[my_attention], 14, "\nATAQUE FALLIDO\n");
        }
      }
      else {
        server_send_message(sockets_array[my_attention], 14, "\nESTE JUGADOR YA HA SIDO ELIMINADO: NO SE PUEDE ATACAR\n");
      }
      free(client_message);
      // Le enviamos la respuesta
      //char * response = "funcion por hacer";
      //server_send_message(sockets_array[my_attention], 14, response);
    }
    if (msg_code == 15) //El cliente me envió un mensaje a mi (servidor) ESPIAR
    {
      printf("entre a code 15\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: espiar a jugador con id %s\n", my_attention+1, client_message);
      int client_message_int = atoi(client_message);
      free(client_message);
      if (jugadores_array[client_message_int]->eliminado==false){
        char * response = espiar(jugadores_array[my_attention], jugadores_array[client_message_int]);
        free(client_message);
        server_send_message(sockets_array[my_attention], 15, response);
        free(response);
      }
      else {
        server_send_message(sockets_array[my_attention], 15, "\nESTE JUGADOR YA HA SIDO ELIMINADO: NO SE PUEDE ESPIAR\n");
      }
    } 
    if (msg_code == 16) //El cliente me envió un mensaje a mi (servidor) robar
    {
      printf("entre a code 16\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: %s\n", my_attention+1, client_message);
      char* id_robar = calloc(1, sizeof(char));
      id_robar[0]=client_message[0];
      char* recurso_robar = calloc(1, sizeof(char));
      recurso_robar[0] = client_message[1];
      int id_robar_int = atoi(id_robar);
      int recurso_robar_int = atoi(recurso_robar);
      free(id_robar);
      free(recurso_robar);
      free(client_message);

      if (jugadores_array[id_robar_int]->eliminado==false){
        char * response = robar(jugadores_array[my_attention], jugadores_array[id_robar_int], recurso_robar_int);
        free(client_message);
        server_send_message(sockets_array[my_attention], 16, response);
        free(response);
      }
      else {
        server_send_message(sockets_array[my_attention], 16, "\nESTE JUGADOR YA HA SIDO ELIMINADO: NO SE PUEDE ROBAR\n");      
      }

    } if (msg_code == 17) //El cliente me envió un mensaje a mi (servidor) pasar
    {
      printf("entre a code 17\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: %s\n", my_attention+1, client_message);
      // Le enviamos la respuesta
      char * response = "Jugador paso turno";
      server_send_message(sockets_array[my_attention], 11, response);
      free(client_message);
      // Mi atención cambia al otro socket

      int jugadores_vivos = 0;
      for (int i = 0; i < n_jugadores; i++)
      {
        if (!jugadores_array[(my_attention + i) % n_jugadores]->eliminado) {
          jugadores_vivos++;
        }
      }
      if (jugadores_vivos == 1) {
        for (int i = 0; i < n_jugadores; i++)
        {
          if (!jugadores_array[(my_attention + i) % n_jugadores]->eliminado) {
          my_attention = (my_attention + i) % n_jugadores;
          char* response2 = recolectar_recursos(jugadores_array[my_attention]);
          server_send_message(sockets_array[my_attention], 19, response2);
          free(response2);
          //printf("ganaste\n");
          printf("my attention %i\n", my_attention);
          server_send_message(sockets_array[my_attention], 11, "\nHAZ GANADO\n");
          for (int i = 0; i < n_jugadores; i++)
          {
            server_send_message(sockets_array[i], 21, "Fin juego");
            free(jugadores_array[i]);
          }
          // mandar mensaje que gano
          return 0;
          }
        }
      }
      else {
        int i = 1;
        while (1)
        {
          if (!jugadores_array[(my_attention + i) % n_jugadores]->eliminado) {
            my_attention = (my_attention + i) % n_jugadores;
            char* response2 = recolectar_recursos(jugadores_array[my_attention]);
            server_send_message(sockets_array[my_attention], 19, response2);
            free(response2);
            i = 1;
            break;
          }
          else {
            i++;
          }
        }
      }
      //my_attention = (my_attention + 1) % n_jugadores;
      
    } 
    if (msg_code == 18) //El cliente me envió un mensaje a mi (servidor) rendirse
    {
      printf("entre a code 18\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: %s\n", my_attention, client_message);
      free(client_message);
      // Le enviamos la respuesta
      rendirse(jugadores_array[my_attention]);
      char * response = "Jugador se rindio\n";
      server_send_message(sockets_array[my_attention], 20, response);
      // Mi atención cambia al otro socket
      my_attention = (my_attention + 1) % n_jugadores;
      
    }
    if (msg_code == 19){
      printf("entre a code 19\n");
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: %s\n", my_attention+1, client_message);
      char* response = recolectar_recursos(jugadores_array[my_attention]);
      free(client_message);
      server_send_message(sockets_array[my_attention], 17, response);
      free(response);
    // free(response);
    }

    printf("------------------\n");
    server_send_message(sockets_array[my_attention], 1, "vuelve a escoger una opcion");
  }
  return 0;
}
