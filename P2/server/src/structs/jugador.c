#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "jugador.h"

Jugador* jugador_init(char* nombre, int id){
  Jugador* jug = malloc(sizeof(Jugador));
  jug->id = id;
  jug->nombre = nombre;
  jug->aldeanos=9; 
  if(id==0){
    jug->es_lider=1;
  }
  else{
    jug->es_lider=0;
  }
  // Inicializan en cero y cuando son asignados los aldeanos 
  // los niveles deben en empezar en 1 y se asignan los nueve
  // aldeanos al cada tipo
  jug->oro = 0;
  jug->comida = 0;
  jug->ciencia = 0;
  jug->n_mineros = 0;
  jug->n_agricultores = 0;
  jug->n_ingenieros = 0;
  jug->n_guerreros = 1;
  jug->nivel_minero = 1;
  jug->nivel_agricultores = 1;
  jug->nivel_ingenieros = 1;
  jug->nivel_ataque = 1;
  jug->nivel_defensa = 1;
  jug->eliminado = false;

  return jug;

}

char* recolectar_recursos(Jugador* jug){
  
  int new_oro = jug->n_mineros*jug->nivel_minero*2;
  int new_comida = jug->n_agricultores*jug->nivel_agricultores*2;
  int new_ciencia = jug->n_ingenieros*jug->nivel_ingenieros;
  // Se recolectan los recursos y se agregan a los atributos
  jug->oro = jug->oro+new_oro;
  jug->comida = jug->comida+new_comida;
  jug->ciencia = jug->ciencia+new_ciencia;
  char* mensaje = calloc(2000, sizeof(char));
  sprintf(mensaje, "vuelve a escoger una opcion\nRECOLECION EXITOSA\nHaz recolectado %i de comida\nHaz recolectado %i de oro\nHaz recolectado %i de ciencia\n", new_comida,new_oro,new_ciencia);
  return mensaje;
}

void asignar_aldeano(Jugador* jug, int tipo){
  /* El tipo se designa por el numero
    1 = minero
    2 = agricultor
    3 = ingeniero
    4 = guerrero
  */ 
  if (tipo == 1)
  {
    jug->n_agricultores++;  
  }
  else if (tipo ==2)
  { 
    jug->n_mineros++;  
  }
  else if (tipo ==3)
  {
    jug->n_ingenieros++;
  }
  else if (tipo ==4)
  {
    jug->n_guerreros++;
  }
  // restar un aldeano de los 9 iniciales
  jug->aldeanos=jug->aldeanos-1;
  
}

bool crear_aldeano(Jugador* jug, int tipo){
  printf("jugador %i creara un tipo %i\n", jug->id, tipo);
  if (tipo == 1)
  {
    if (jug->comida >=10 && jug->oro >=5)
    {
      jug->n_mineros++; 
      jug->comida=jug->comida-10;
      jug->oro=jug->oro-5;
      printf("Se creo un minero\n");
      return true;
    }
    else{
      printf("recursos insuficiente\n");
      return false;
    }
  }
  else if (tipo == 2)
  {
    if (jug->comida >=10)
    {
      jug->n_agricultores++;
      jug->comida=jug->comida-10;
      printf("Se creo un agricultor\n");
      return true;
    }
    else{
      printf("recursos insuficiente\n");
      return false;
    }
  }
  else if (tipo == 3)
  {
    if (jug->comida >=20 && jug->oro >=10)
    {
      jug->n_ingenieros++;
      jug->comida=jug->comida-20;
      jug->oro=jug->oro-10;
      printf("Se creo un ingeniero\n");
      return true;
    }
    else{
      printf("recursos insuficiente\n");
      return false;
    }
  }
  else if (tipo == 4)
  {
    if (jug->comida >=10 && jug->oro >=10)
    {
      jug->n_guerreros ++;
      jug->comida=jug->comida-10;
      jug->oro=jug->oro-10;
      printf("Se creo un guerrero\n");
      return true;
    }
    else{
      printf("recursos insuficiente\n");
      return false;
    }
  }
}




//CUALQUIER DUDA DE ESTAS ME PREGUNTAN (Pachi)

// -----mostrar info -------------------------

void mostrar_informacion(Jugador* jug){
  printf("Informacion...:\n");
  printf("Recursos\n");
  printf("Comida: %i\n", jug->comida);
  printf("Oro: %i\n", jug->oro);
  printf("Ciencia: %i\n", jug->ciencia);

  printf("Aldeanos\n");
  printf("Agricultores: %i - Nivel: %i\n", jug->n_agricultores, jug->nivel_agricultores);
  printf("Mineros: %i- Nivel: %i\n", jug->n_mineros, jug->nivel_minero);
  printf("Guerrers: %i- Nivel: %i\n", jug->n_guerreros, jug->nivel_ataque);
  printf("Ingenieros: %i- Nivel: %i\n", jug->n_ingenieros, jug->nivel_ingenieros);
  
}
//--subir nivel--------------------------

int subir_nivel_minero(Jugador* jug){
  int cantidad_necesaria = jug->nivel_minero*10;
  if (jug->nivel_minero<5 && jug->comida>=cantidad_necesaria && jug->oro>=cantidad_necesaria && jug->ciencia>=cantidad_necesaria){
    jug->comida-=cantidad_necesaria;
    jug->oro-=cantidad_necesaria;
    jug->ciencia-=cantidad_necesaria;
    jug->nivel_minero+=1;
    printf("AUMENTO DE NIVEL: %i -> %i\n", jug->nivel_minero-1, jug->nivel_minero);
    return 0;
  }
  if (jug->nivel_minero==5){
    printf("Estas en el nivel maximo\n");
    return 1;
  }
  else {
    printf("Faltan recursos para poder subir de nivel\n");
    return 2;
  }
}

int subir_nivel_agricultor(Jugador* jug){
  int cantidad_necesaria = jug->nivel_minero*10;
  if (jug->nivel_agricultores<5 && jug->comida>=cantidad_necesaria && jug->oro>=cantidad_necesaria && jug->ciencia>=cantidad_necesaria){
    jug->comida-=cantidad_necesaria;
    jug->oro-=cantidad_necesaria;
    jug->ciencia-=cantidad_necesaria;
    jug->nivel_agricultores+=1;
    printf("AUMENTO DE NIVEL: %i -> %i\n", jug->nivel_agricultores-1, jug->nivel_agricultores);
    return 0;
  }
  if (jug->nivel_agricultores==5){
    printf("Estas en el nivel maximo\n");
    return 1;
  }
  else {
    printf("Faltan recursos para poder subir de nivel\n");
    return 2;
  }
}

int subir_nivel_ingeniero(Jugador* jug){
  int cantidad_necesaria = jug->nivel_ingenieros*10;
  if (jug->nivel_ingenieros<5 && jug->comida>=cantidad_necesaria && jug->oro>=cantidad_necesaria && jug->ciencia>=cantidad_necesaria){
    jug->comida-=cantidad_necesaria;
    jug->oro-=cantidad_necesaria;
    jug->ciencia-=cantidad_necesaria;
    jug->nivel_ingenieros+=1;
    printf("AUMENTO DE NIVEL: %i -> %i\n", jug->nivel_ingenieros-1, jug->nivel_ingenieros);
    return 0;
  }
  if (jug->nivel_ingenieros==5){
    printf("Estas en el nivel maximo\n");
    return 1;
  }
  else {
    printf("Faltan recursos para poder subir de nivel\n");
    return 2;
  }
}

int subir_nivel_ataque(Jugador* jug){
  int cantidad_necesaria = jug->nivel_ataque*10;
  if (jug->nivel_ataque<5 && jug->comida>=cantidad_necesaria && jug->oro>=cantidad_necesaria && jug->ciencia>=cantidad_necesaria){
    jug->comida-=cantidad_necesaria;
    jug->oro-=cantidad_necesaria;
    jug->ciencia-=cantidad_necesaria;
    jug->nivel_ataque+=1;
    printf("AUMENTO DE NIVEL: %i -> %i\n", jug->nivel_ataque-1, jug->nivel_ataque);
    return 0;
  }
  if (jug->nivel_ataque==5){
    printf("Estas en el nivel maximo\n");
    return 1;
  }
  else {
    printf("Faltan recursos para poder subir de nivel\n");
    return 2;
  }
}
int subir_nivel_defensa(Jugador* jug){
  int cantidad_necesaria = jug->nivel_defensa*10;
  if (jug->nivel_defensa<5 && jug->comida>=cantidad_necesaria && jug->oro>=cantidad_necesaria && jug->ciencia>=cantidad_necesaria){
    jug->comida-=cantidad_necesaria;
    jug->oro-=cantidad_necesaria;
    jug->ciencia-=cantidad_necesaria;
    jug->nivel_defensa+=1;
    printf("AUMENTO DE NIVEL: %i -> %i\n", jug->nivel_defensa-1, jug->nivel_defensa);
    return 0;
  }
  if (jug->nivel_defensa==5){
    printf("Estas en el nivel maximo\n");
    return 1;
  }
  else {
    printf("Faltan recursos para poder subir de nivel\n");
    return 2;
  }
}


int subir_nivel(Jugador* jug, int tipo){
  if (tipo == 1){ //nivel minero
    int n = subir_nivel_minero(jug);
    return n;
  } else if (tipo == 2){
    int n = subir_nivel_agricultor(jug);
    return n;
  } else if (tipo == 3){
    int n = subir_nivel_ingeniero(jug);
    return n;
  } else if (tipo == 4){
    int n = subir_nivel_ataque(jug);
    return n;
  } else if (tipo == 5){
    int n = subir_nivel_defensa(jug);
    return n;
  }
}
//---------curr ataca a other ---------
int atacar(Jugador* curr, Jugador* other){
  //curr->fuerza = curr->nivel_guerreros * curr->nivel_ataque;
  //other->nivel_defensa = (other->nivel_guerreros * other->nivel_ataque) * 2;
  int fuerza_atacante = curr->n_guerreros*curr->nivel_ataque;
  printf("cantidad guerreros defensor: %i, nivel defensa defensor: %i\n", other->n_guerreros, other->nivel_defensa);
  int fuerza_defensor = (other->n_guerreros*other->nivel_defensa)*2;
  printf("fuerza atacante : %i, fuerza defensor: %i\n", fuerza_atacante, fuerza_defensor);
  if (fuerza_atacante>fuerza_defensor){
  //if (curr > other){
    curr->comida += other->comida;
    other->comida = 0;
    curr->oro += other->oro;
    other->oro = 0;
    curr->ciencia += other->ciencia;
    other->ciencia = 0;
    other->eliminado=true;
    return 1;
    //ACA SE DEBERIA ELIMINAR EL JUGADOR

  } else{
    curr->n_guerreros = curr->n_guerreros/2;
    return 0;

  }


}
//-----------curr espia a other---------------
char* espiar(Jugador* curr, Jugador* other){
  
  if (curr->oro>=30){
    curr->oro -= 30;
    char* mensaje = calloc(2000,sizeof(char));
    sprintf(mensaje, "ESPIANDO A %s\n- Cantidad guerreros: %i\n- Nivel defenza: %i\n- Nivel ataque: %i\n",other->nombre, other->n_guerreros, other->nivel_defensa, other->nivel_ataque);
    //printf("entro funcion espiar4 y el nombre es %s\n", other->nombre);
    //printf("Espiando....\n");
    //printf("cantidad guerreros: %i", other->n_guerreros);
    //printf("nivel defenza: %i", other->nivel_defensa);
    //printf("nivel ataque: %i", other->nivel_ataque);
    return mensaje;

  } else{
    char* mensaje_else = calloc(500,sizeof(char));
    //char* mensaje_else;
    sprintf(mensaje_else,"No hay suficientes recursos\n");
    return mensaje_else;
  }
  


}
//--------------------------

//---------curr roba a other -------------------
//recurso_robar es un int que representa si eligio robar comida o oro
//recurso_robar=0 es comida y recurso_robar=1 es oro
char* robar(Jugador* curr, Jugador* other, int recurso_robar){
  if (curr->ciencia>=10){
    curr->ciencia-=10;
    if (recurso_robar==0){
      int comida_robada = (other->comida)*(0.1);
      other->comida-=comida_robada;
      curr->comida+=comida_robada;
      char* mensaje = calloc(2000, sizeof(char));
      sprintf(mensaje, "ROBO EXITOSO\nLe haz robado %i de comida al jugador con id %i\n", comida_robada, other->id);
      return mensaje;
    }
    if (recurso_robar==1){
      int oro_robado = (other->oro)*(0.1);
      other->oro-=oro_robado;
      curr->oro+=oro_robado;
      char* mensaje_segundo_if =calloc(2000, sizeof(char));
      sprintf(mensaje_segundo_if,"ROBO EXITOSO\nLe haz robado %i de oro al jugador con id %i\n", oro_robado, other->id);
      return mensaje_segundo_if;
    }
  }
  else {
    char* mensaje_else = calloc(500, sizeof(char));
    sprintf(mensaje_else,"ERROR: No tienes sufiecientes recursos para realizar esta accion\n");
    return mensaje_else;
  }
}

//Aqui se deberia pasar al siguiente turno pero aun no se como hacerlo
//Yo creo que se deberia manejar en flujo principal
void pasar(Jugador* jug){
  printf("Se ha termiando el turno actual\n");
}

void rendirse(Jugador* jug){
  printf("Jugador con id %i se ha rendido\n", jug->id);
  jug->oro = 0;
  jug->comida = 0;
  jug->ciencia = 0;
  jug->n_mineros = 0;
  jug->n_agricultores = 0;
  jug->n_ingenieros = 0;
  jug->n_guerreros = 0;
  jug->eliminado = true;
  //Aqui hay que eliminar al jugador de los jugadores activos

}

//int main(int argc, char const *argv[])
//{
  //printf("\n:)\n");
  //return 0;
//}
