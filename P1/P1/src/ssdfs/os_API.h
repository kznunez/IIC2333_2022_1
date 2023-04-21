#pragma once
#include <stdint.h>
#include <stddef.h>

//Definicion del Struct osFile que representa un archivo.
typedef struct osfile {
    int modo; //0:read 1:write
    size_t tamano;
    size_t cursor;
    uint32_t bloque_ultimo;
    uint32_t pagina_ultimo;
    int utilizado;
    uint32_t bloque_indice;
    uint8_t *buffer;
} osFile;


typedef struct stacknode StackNode;

struct stacknode {
    uint32_t block;
    int idx;
    StackNode* next;
};

extern int LIFE;
extern char* DISK_NAME;


// Declaracion de funciones (Enunciado)
// Funciones Generales
void os_mount(char* diskname, unsigned life);
void os_bitmap(unsigned num);
void os_lifemap(int lower, int upper);
int os_trim(unsigned limit);
void os_tree();

// Funciones de manejo de archivos
int os_exists(char* filename);
osFile* os_open(char* filename, char mode);
int os_read(osFile* file_desc, void* buffer, int nbytes);
int os_write(osFile* file_desc, void* buffer, int nbytes);
int os_close(osFile* file_desc);
int os_rm(char* filename);
int os_mkdir(char* path);
int os_rmdir(char* path);
int os_rmrfdir(char* path);
int os_unload(char* orig, char* dest);
int os_load(char* orig);