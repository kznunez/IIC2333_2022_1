#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>
#include "./os_API.h"

#define BLOCKS 2048
#define BYTES_PER_DIRECTORY_ENTRY 32
#define PAGES_PER_BLOCK 256
#define BYTES_PER_PAGE 4096
#define BYTES_PER_BLOCK 1048576

typedef struct locfile {
    enum {DIRECTORIO, ARCHIVO, INVALIDO} tipo;
    uint32_t bloque;
    size_t tamano;
} LocFile;

// Struct
//struct stat staa = {0};
// Declaracion de funciones 
int len(char* string);
int check_bloque(FILE *file_disco, uint32_t bloque);
int es_archivo(char* nombre);
char *obtener_nombre(char *path);
int comprobar_si_existe(FILE *file, char* nombre);
uint32_t buscar_bloque_disponible(FILE *file_disco);
int search(FILE *file, LocFile *archivo, char *nombre);
int write(FILE *file_disco, FILE *file_salida, LocFile *archivo);
void write_to_disk(FILE *file_disco, FILE *file_entrada, char *nombre, unsigned life);
void search_and_write(FILE *file_disco, char *dest,  uint32_t bloque, char *nombre_dir);
char *obtener_nombre(char *path);
void marcar_bloque_ocupado(FILE *file_disco, uint32_t dir_bloque);
int aumentar_lifmap(FILE *file_disco, uint32_t bloque, unsigned int pagina, unsigned life_max);
size_t rellenar_bloque(FILE *file_disco, FILE *file_entrada, uint32_t bloque, size_t memoria_restante, unsigned life_max);

void llenar_osfile(FILE *file, osFile *archivo, char *buscado);
void crear_archivo(FILE *file, osFile *archivo, char *nombre);
void llenar_tamano(FILE *file, osFile *archivo, char *buscado);
int agregar_puntero(FILE *file, uint32_t bloque_indice, uint32_t bloque_nuevo, size_t tamano);