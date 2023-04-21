#include "./os_API.h"
#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#define BLOCKS 2048
#define PAGES_PER_BLOCK 256
#define BYTES_PER_PAGE 4096
#define BYTES_PER_BLOCK 1048576

// Funcion extra
static char **split_by_sep(char *str, char *sep)
{
    char **new_str = calloc(255, sizeof(char *));
    int index = 0, len;

    char *token = strtok(str, sep);
    while (token != NULL)
    {
        new_str[index] = calloc(4096, sizeof(char));
        strcpy(new_str[index++], token);
        token = strtok(NULL, sep);
    }

    // Remove dangling Windows (\r) and Unix (\n) newlines
    len = strlen(new_str[index - 1]);
    if (len > 1 && new_str[index - 1][len - 2] == '\r')
    new_str[index - 1][len - 2] = '\0';
    else if (len && new_str[index - 1][len - 1] == '\n')
    new_str[index - 1][len - 1] = '\0';
    return new_str;
} 



/* Funciones Generales */
void os_mount(char* diskname, unsigned life)
{
    LIFE = life;
    DISK_NAME = diskname;
}

void os_bitmap(unsigned num)
{
    // Abrimos el archivo .bin
    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL)
        exit(1);
    // Cantidad de bloques libres y ocupados
    int free_blocks = 0;
    int taken_blocks = 0;
    // Lectura pagina
    char* page0 = malloc(sizeof(char) * 4096);   // 4096 Bytes 
    fseek(file, 0, SEEK_SET);                    // Establecemos la posicion donde comienza la lectura 
    fread(page0, sizeof(char), 4096, file);      // Leemos la pagina 0
    // Imprimimos el bitmap completo
    if (num == 0)
    {
        printf("\nObtenemos el bitmap completo.\n");
        for (int i = 0; i < 256; i++)            // Recorremos los primeros 256 chars (256 Bytes = 2048 bits)
        {
            unsigned int byte = page0[i];
            //printf("\n%d\n", byte);
            for(int j = 0; j < 8; j++)           // Recorremos los 8 bits de cada char
            {
                unsigned int bit = byte & 0x080; // AND con 1000 0000, si es 1 el valid bit es 1.
                bit >>= 7;
                //printf("%d", bit);
                if (bit == 0)                    // Bloque desocupado
                {
                    printf("Bloque %i: %d, ", free_blocks + taken_blocks, bit); 
                    free_blocks = free_blocks + 1;
                }
                else                             // Bloque ocupado
                {
                    printf("Bloque %i: %d, ", free_blocks + taken_blocks, bit); 
                    taken_blocks = taken_blocks + 1;
                }
                byte <<= 1;
            }
            printf("\n");
        }
        printf("\nHay %i bloques ocupados y %i bloques libres.\n", taken_blocks, free_blocks);
    }
    // Imprimimos el valor del bitmap para el bloqeu sum
    else
    {
        for (int i = 0; i < 256; i++)            // Recorremos los primeros 256 chars (256 Bytes = 2048 bits)
        {
            unsigned int byte = page0[i];
            for(int j = 0; j < 8; j++)           // Recorremos los 8 bits de cada char
            {
                unsigned int bit = byte & 0x080; // AND con 1000 0000, si es 1 el valid bit es 1.
                bit >>= 7;
                // Imprimimos el bloque num
                if (free_blocks + taken_blocks == num)
                {
                    printf("Bloque %i: %d\n", free_blocks + taken_blocks, bit);
                }
                if (bit == 0)                    // Bloque desocupado
                {
                    free_blocks = free_blocks + 1;
                }
                else                             // Bloque ocupado
                {
                    taken_blocks = taken_blocks + 1;
                }
                byte <<= 1;
            }
        }
        printf("\nHay %i bloques ocupados y %i bloques libres.\n", taken_blocks, free_blocks);
    }
    // Liberamos malloc de la pagina 0
    free(page0);
    // Cerramos el archivo .bin
    fclose(file);
}

void os_lifemap(int lower, int upper)
{
    // Abrimos el archivo .bin
    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL)
        exit(1);
    // Cantidad de bloques rotten y saludables
    int rotten_blocks = 0;
    int healthy_blocks = 0;
    int exist_rotten_page = 0;                                          // Es 0 si no hay una pagina rotten en el bloque y 1 en el caso contrario
    // Lectura paginas
    int block1_init = 1048576;                                          // Inicio bloque 1 = 1048576 Bytes (primera pagina del bloque 1)
    printf("\n");
    // Imprimimos el lifemap completo
    if (lower == -1 && upper == -1)
    {
        for (int n_page = 0; n_page < 512; n_page++)                    // En dos bleques (bloque 1 y bloque 2) tenemos 512 paginas
        {
            char* page = malloc(sizeof(char) * 4096);                   // 4096 Bytes (pagina)
            fseek(file, block1_init + n_page * block1_init, SEEK_SET);  // Establecemos la posicion donde comienza la lectura 
            fread(page, sizeof(char), 4096, file);                      // Leemos la pagina 
            printf("page %i\n", n_page);
            for (int i = 0; i < 1024; i++)                              // Recorremos los 1024 P/E de la pagina (4 Bytes * 1024 = 4096 Bytes)
            {
                // Obtenemos el valor de P/E
                uint32_t P_E = page[i*4] | (page[i*4+1]<<8) | (page[i*4+2]<<16) | (page[i*4+3]<<24);
                if ((P_E & 0xff000000) >> 24 == -1)                     // Pagina rotten
                {
                    exist_rotten_page = 1;
                }
                if (i == 255 || i == 511 || i == 767 || i == 1023)      // Termino bloque
                {
                    if (exist_rotten_page == 1)                         // Hay una pagina roten el en bloque
                    {
                        rotten_blocks = rotten_blocks + 1;
                        exist_rotten_page = 0;
                    }
                    else                                                // No hay una pagina roten el en bloque
                    {
                        healthy_blocks = healthy_blocks + 1;
                    }
                    
                }
                printf("%i ", (P_E & 0xff000000) >> 24);
            }
            free(page);
            printf("\n");
        }
    }
    // Imprimimos el lifemap del rango indicado
    else
    {
        printf("\nRevisamos el P/E desde la página %i hasta la página %i.\n", lower, upper-1);
        int actual_page = 0;
        for (int n_page = 0; n_page < 512; n_page++)                    // En dos bleques (bloque 1 y bloque 2) tenemos 512 paginas
        {
            unsigned char* page = malloc(sizeof(char) * 4096);                   // 4096 Bytes (pagina)
            fseek(file, block1_init + n_page * block1_init, SEEK_SET);  // Establecemos la posicion donde comienza la lectura 
            fread(page, sizeof(char), 4096, file);                      // Leemos la pagina 0
            for (int i = 0; i < 1024; i++)                              // Recorremos los 1024 P/E de la pagina (4 Bytes * 1024 = 4096 Bytes)
            {
                // Obtenemos el valor de P/E
                if (actual_page >= lower && actual_page < upper)        // Si la pagina actual esta dentro del rango pedido
                {
                    uint32_t P_E = page[i*4+3] + (page[i*4+2]<<8) + (page[i*4+1]<<16) + (page[i*3]<<24);
                    if ((P_E & 0xff000000) >> 24 == -1)                 // Pagina rotten
                    {
                        exist_rotten_page = 1;
                    }
                    if (i == 255 || i == 511 || i == 767 || i == 1023)  // Termino bloque
                    {
                        if (actual_page - lower >= 255)
                        {
                            if (exist_rotten_page == 1)                 // Hay una pagina roten el en bloque
                            {
                                rotten_blocks = rotten_blocks + 1;
                                exist_rotten_page = 0;
                            }
                            else                                        // No hay una pagina roten el en bloque
                            {
                                healthy_blocks = healthy_blocks + 1;
                            }
                        }
                    }
                    printf("%i ", (P_E & 0xff000000) >> 24);
                }
                actual_page = actual_page + 1;
            }
            free(page);
        }
        printf("\n");
    }
    if (rotten_blocks == 0 && healthy_blocks == 0)
    {
        printf("\nNo completamos la revisión de un bloque completo.\n");
    }
    else
    {
        printf("\nHay %i bloques rotten y %i bloques saludables.\n", rotten_blocks, healthy_blocks);
    }
    // Cerramos el archivo .bin
    fclose(file);
}


// ------------------------------ os_trim -------------------------------
uint32_t find_free_block(uint32_t first, uint8_t* bitmap, uint8_t* lif_page, FILE* disk, unsigned limit) {
    // si el primer candidato es 0, sabemos que no hay disponibles y nos ahorramos revisar todo de nuevo
    if (first == 0) {
        return first;
    }
    int32_t life;
    for (; first < BLOCKS; first++) {
        if ((bitmap[first >> 3] & (1 << (7 - (first & 7)))) == 0) {
        // bloque está libre; hay que revisar las páginas para que no estén en peligro
            fseek(disk, BYTES_PER_BLOCK * 1 + (first >> 2) * BYTES_PER_PAGE, 0);
            fread(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
            int fb = (first & 3) * PAGES_PER_BLOCK << 2;
            for (; fb < ((first & 3) + 1) * PAGES_PER_BLOCK << 2; fb += 4) {
                life = lif_page[fb] | (lif_page[fb+1]<<8) | (lif_page[fb+2]<<16) | (lif_page[fb+3]<<24);
                if ((life == -1) || (LIFE - life - 1 <= limit)) { // -1 por si hubiera que reubicar algo y reescribir este bloque
                    // si está rotten o está en el límite, el bloque no nos sirve
                    break;
                }
            }
            if (fb >= ((first & 3) + 1) * PAGES_PER_BLOCK << 2) { // revisamos el bloque completo sin problemas: es utilizable
                return first;
            }
        }
    }
    return 0;
}

void relocate_block(uint32_t block, uint32_t free, uint8_t* lif_page, FILE* disk) {
    uint8_t page[BYTES_PER_PAGE];
    int32_t life;
    //printf("%d > %d\n", block, free);
    // page by page, one by one
    for (int dp = 0; dp < BYTES_PER_BLOCK; dp += BYTES_PER_PAGE) { // dp:dir page
        fseek(disk, BYTES_PER_BLOCK * block + dp, 0);
        fread(page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
        fseek(disk, BYTES_PER_BLOCK * free + dp, 0);
        fwrite(page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
    }
    // escribir el lifemap
    fseek(disk, BYTES_PER_BLOCK * 1 + (free >> 2) * BYTES_PER_PAGE, 0);
    fread(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
    for (int lp = (free & 3) * PAGES_PER_BLOCK << 2; 
            lp < ((free & 3) + 1) * PAGES_PER_BLOCK << 2; lp += 4) {
        life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
        life++;
        lif_page[lp] = life & 0xff;
        lif_page[lp+1] = (life & 0xff00) >> 8;
        lif_page[lp+2] = (life & 0xff0000) >> 16;
        lif_page[lp+3] = (life & 0xff000000) >> 24;
    }
    fwrite(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
}

int os_trim(unsigned limit) {
    FILE *disk = fopen(DISK_NAME, "r+b");
    uint8_t* bitmap = calloc(BYTES_PER_PAGE, sizeof(uint8_t));
    fread(bitmap, sizeof(uint8_t), BYTES_PER_PAGE, disk);
    uint32_t first_free = 4;
    uint8_t* lif_page = calloc(BYTES_PER_PAGE, sizeof(uint8_t));
    uint8_t dir_page[BYTES_PER_PAGE]; // no es necesario que todas las páginas usen malloc
    uint8_t idx_page[BYTES_PER_PAGE];
    StackNode* pendingDirs = malloc(sizeof(StackNode));
    pendingDirs -> next = NULL;
    StackNode* aux;
    StackNode* dataPointers;
    int flags = 0;  // flags |= flag para subirla ; flags &= ~flag para bajarla
    enum flag{write_bitmap=1, write_page=2, relocate=4, rel_file=8, file_rotten=16};
    uint32_t block_pointer, data_pointer;
    int32_t life;
    size_t size;
    int relocates = 0;

    uint32_t block = 3;
    do {
        // iterar las paginas del directorio
        for (int p = 0; p < PAGES_PER_BLOCK; p++) {
            fseek(disk, BYTES_PER_BLOCK * block + p * BYTES_PER_PAGE, 0);
            fread(dir_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
            for (int e = 0; e < BYTES_PER_PAGE; e+=32) {
                // invalid
                if (dir_page[e + 0] == 0b00) {
                    continue;
                // directory or file; el procedimiento es similar
                } else if (dir_page[e + 0] & 1) {
                    // un shift left es lo mismo que multiplicar por 2; n << 8 = n * 2^8 = n * 256
                    block_pointer = dir_page[e + 1];
                    block_pointer |= (dir_page[e + 2] << 8);
                    block_pointer |= (dir_page[e + 3] << 16);
                    // como es puntero, el último hay que pasarlo a unsigned o podría haber problemas si es >= 128
                    block_pointer |= ((unsigned)dir_page[e + 4] << 24);
                    //printf("%hhu %hhu %hhu %hhu ", dir_page[e + 1], dir_page[e + 2], dir_page[e + 3], dir_page[e + 4]);
                    //printf("%u %u %u \n", block_pointer, block_pointer >> 2, block_pointer & 3);

                    // encontrar el bloque en el lifemap: numero de 1era pagina del bloque por 4
                    // esto se divide en 4096 para obtener la página del lifemap que lo contiene.
                    // entonces: multiplicar por 256 para obtener el número de la 1er página del
                    // bloque; multiplicar por 4 para ubicar el byte dentro del lifemap; dividir
                    // en 4096 para obtener el 1er byte de la página del lifemap que lo contiene
                    // 256 * 4 / 4096 = 1024/4096 = 1/4 -> hay que dividir por 4 !!! (o SHR 2 veces)
                    fseek(disk, BYTES_PER_BLOCK * 1 + (block_pointer >> 2) * BYTES_PER_PAGE, 0);
                    fread(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                    // una página del lifemap contiene información sobre 4 bloques. El bloque que
                    // nos interesa lo indican los últimos 4 bits del número de bloque.
                    // avanzamos de a 4, el límite está al final de la sección correspondiente a este bloque
                    flags &= ~relocate; // anular la flag para reubicar la entrada
                    for (int lp = (block_pointer & 3) * PAGES_PER_BLOCK << 2; // lp:lifemap page
                         lp < ((block_pointer & 3) + 1) * PAGES_PER_BLOCK << 2; lp += 4) {
                        life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
                        if (life == -1) {
                            flags &= ~relocate; // si está rotten, no tiene sentido reubicarla
                            break;
                        } else if (LIFE - life <= limit) {
                            flags |= relocate; // subir la flag relocate
                        }
                    }
                    if (flags & relocate) { // hay que reubicar el bloque
                        first_free = find_free_block(first_free, bitmap, lif_page, disk, limit);
                        if (first_free > 3) {
                            relocate_block(block_pointer, first_free, lif_page, disk);
                            // escribir el bitmap
                            bitmap[first_free >> 3] |= 1 << (7 - (first_free & 7));
                            bitmap[block_pointer >> 3] &= ~(1 << (7 - (block_pointer & 7)));
                            flags |= write_bitmap; // subir esta flag indica que el bitmap cambió y hay que escribirlo a disco

                            // escribir el directorio
                            dir_page[e+1] = first_free & 0xff;
                            dir_page[e+2] = (first_free & 0xff00) >> 8;
                            dir_page[e+3] = (first_free & 0xff0000) >> 16;
                            dir_page[e+4] = (first_free & 0xff000000) >> 24;
                            flags |= write_page; // subir la flag para escribir la pagina del dir
                            
                            relocates++;
                            block_pointer = first_free;
                            first_free++; // no es estrictamente necesario, pero nos ahorra trabajo en find_free
                        }
                        if (first_free == 0) { // si no encontramos ningún bloque
                            // ~abort~ mejor dicho, imprimimos lo pedido si no se puede reubicar
                        }
                    }

                    if (dir_page[e + 0] & 2) { // file only
                        fseek(disk, BYTES_PER_BLOCK * block_pointer, 0);
                        fread(idx_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                        // los primeros tres pasan automáticamente a int
                        size = idx_page[0];
                        size |= (idx_page[1] << 8);
                        size |= (idx_page[2] << 16);
                        // el cuarto elemento hay que pasarlo a unsigned, si no podría haber serios problemas con complemento a 2
                        size |= ((unsigned)idx_page[3] << 24);
                        // el resto hay que pasarlo a size_t
                        size |= ((size_t)idx_page[4] << 32);
                        size |= ((size_t)idx_page[5] << 40);
                        size |= ((size_t)idx_page[6] << 48);
                        size |= ((size_t)idx_page[7] << 56);
                        //flags &= ~rel_file; // anular la flag de reubicar algún bloque de datos
                        //flags &= ~file_rotten; // anular la flag de archivo rotten
                        flags &= ~(rel_file | file_rotten);
                        // en este stack guardaremos los bloques de datos que habrá que reubicar
                        dataPointers = malloc(sizeof(StackNode));
                        dataPointers -> next = NULL;
                        dataPointers -> block = 0;
                        // avanzamos de a 8 partiendo en 8; (dp - 8)/4 es la cantidad de bloques recorridos, AKA la cantidad de MiB
                        for (int dp = 8; ((dp - 8) >> 2) * BYTES_PER_BLOCK < size; dp += 4) { // dp:data page
                            // el último debe pasarse a unsigned como siempre
                            data_pointer = idx_page[dp] | (idx_page[dp+1]<<8) | (idx_page[dp+2]<<16) | ((unsigned)idx_page[dp+3]<<24);
                            // cargar la life page correspondiente al bloque de datos
                            fseek(disk, BYTES_PER_BLOCK * 1 + (data_pointer >> 2) * BYTES_PER_PAGE, 0);
                            fread(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                            // revisamos la vitalidad de sus páginas
                            for (int lp = (data_pointer & 3) * PAGES_PER_BLOCK << 2; // lp:lifemap page
                                 lp < ((data_pointer & 3) + 1) * PAGES_PER_BLOCK << 2; lp += 4) {
                                life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
                                if (life == -1) {
                                    flags |= file_rotten; // si está rotten, no tiene sentido reubicar
                                    break;
                                } else if (LIFE - life <= limit) {
                                    // se setea siempre lo mismo, pero seguimos loopeando por si el bloque está rotten
                                    dataPointers -> block = data_pointer;
                                    dataPointers -> idx = dp;
                                    // hay que guardar el índice del puntero para más adelante
                                }
                            }
                            if (flags & file_rotten) { // si está rotten no nos interesa qué pasa con los otros bloques
                                break;
                            } else if (dataPointers -> block > 0) { // si fue cambiado es un bloque a revisar, se guarda en el stack
                                aux = malloc(sizeof(StackNode));
                                aux -> block = 0;
                                aux -> next = dataPointers;
                                dataPointers = aux;
                            }
                        }
                        while (dataPointers -> next != NULL) {
                            // eliminamos el último elemento porque no nos sirve
                            aux = dataPointers;
                            dataPointers = dataPointers -> next;
                            free(aux);
                            if ((flags & file_rotten) == 0) { // solo trabajamos si el archivo no está roto
                                first_free = find_free_block(first_free, bitmap, lif_page, disk, limit);
                                if (first_free > 3) {
                                    relocate_block(dataPointers -> block, first_free, lif_page, disk);
                                    bitmap[first_free >> 3] |= 1 << (7 - (first_free & 7));
                                    bitmap[dataPointers -> block >> 3] &= ~(1 << (7 - (dataPointers -> block & 7)));
                                    flags |= write_bitmap;
                                    // escribimos los punteros que cambiamos
                                    idx_page[dataPointers -> idx] = first_free & 0xff;
                                    idx_page[dataPointers -> idx + 1] = (first_free & 0xff00) >> 8;
                                    idx_page[dataPointers -> idx + 2] = (first_free & 0xff0000) >> 16;
                                    idx_page[dataPointers -> idx + 3] = (first_free & 0xff000000) >> 24;
                                    flags |= rel_file; // subir la flag que dice que reubicamos algún bloque de datos para escribir el índice a disco
                                    
                                    relocates++;
                                    first_free++;
                                }
                            }
                        }
                        // en teoría se podría setear el primero solo una vez al principio y liberarlo solo al final pero :V
                        free(dataPointers);
                        if (flags & rel_file) { // si reubicamos algún bloque de datos escribimos el índice a disco
                            fseek(disk, BYTES_PER_BLOCK * block_pointer, 0);
                            fwrite(idx_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                            // hay que subir el contador P/E
                            fseek(disk, BYTES_PER_BLOCK * 1 + (block_pointer >> 2) * BYTES_PER_PAGE, 0);
                            fread(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                            int lp = ((block_pointer & 3) * PAGES_PER_BLOCK) << 2; // el índice del primer byte en la página del lifemap que está en memoria
                            life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
                            life++; // no hay que preocuparse que este bloque no esté rotten o en el límite [buscamos un bloque que no diera problemas]
                            // guardar el valor P/E como arreglo de 4 bytes en la life page
                            lif_page[lp] = life & 0xff;
                            lif_page[lp+1] = (life & 0xff00) >> 8;
                            lif_page[lp+2] = (life & 0xff0000) >> 16;
                            lif_page[lp+3] = (life & 0xff000000) >> 24;
                            // escribirla tal cual
                            fwrite(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                        }
                    } else if ((dir_page[e + 0] & 2) == 0) { // directory only
                        // hay que subir nuestro Stack en un nivel ;; este directorio será revisado después
                        pendingDirs -> block = block_pointer;
                        aux = malloc(sizeof(StackNode));
                        aux -> next = pendingDirs;
                        pendingDirs = aux;
                    }
                }
            }
            // si reubicamos algún bloque apuntado por este directorio, entonces cambiamos los punteros y hay que escribirlo al disco
            if (flags & write_page) {
                fseek(disk, BYTES_PER_BLOCK * block + p * BYTES_PER_PAGE, 0);
                fwrite(dir_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                if (block > 3) { // si este no es el directorio base hay que subir el contador P/E
                    fseek(disk, BYTES_PER_BLOCK * 1 + (block >> 2) * BYTES_PER_PAGE, 0);
                    fread(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                    int lp = ((block & 3) * PAGES_PER_BLOCK + p) << 2;
                    life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
                    life++;
                    lif_page[lp] = life & 0xff;
                    lif_page[lp+1] = (life & 0xff00) >> 8;
                    lif_page[lp+2] = (life & 0xff0000) >> 16;
                    lif_page[lp+3] = (life & 0xff000000) >> 24;
                    fwrite(lif_page, sizeof(uint8_t), BYTES_PER_PAGE, disk);
                }
                // bajamos la flag correspondiente para que quede libre para el próximo directorio del Stack
                flags &= ~write_page;
            }
        }
        // hacemos pop del siguiente bloque de directorio que revisamos
        aux = pendingDirs;
        pendingDirs = pendingDirs -> next;
        free(aux);
        if (pendingDirs == NULL) {
            // si no quedan bloques de directorio lo situamos en 0 para indicar que terminamos
            block = 0;
        } else {
            block = pendingDirs -> block;
        }
    } while (block >= 3); // si es menor que 3 no puede ser un directorio (bitmap o lifemap) -> terminamos

    if (flags & write_bitmap) { // en caso que haya que escribir el bitmap
        fseek(disk, 0, 0);
        fwrite(bitmap, sizeof(uint8_t), BYTES_PER_PAGE, disk);
    }

    // liberamos todos los allocs y retornamos la cantidad de reubicaciones
    free(lif_page);
    free(bitmap);
    fclose(disk);
    return relocates;
}
// ------------------------------ end os_trim -------------------------------

/* Función recursiva utilizada para imprimir el arbol de directorios os_tree*/
void print_dir(char* dir, uint32_t indice, int sangria)
{
    // Abrimos el archivo .bin
    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL)
        exit(1);
    // Lectura paginas
    int posicion = indice*4096*256;
    for (int n_page = 0; n_page < 256; n_page++)                    // En un bleques (bloque 3) tenemos 256 paginas
    {
        if (n_page != 0)
        {
            posicion = posicion - (n_page-1)*4096 + n_page*4096;
        }
        unsigned char* page = malloc(sizeof(char) * 4096);          // 4096 Bytes (pagina)
        fseek(file, posicion, SEEK_SET);                            // Establecemos la posicion donde comienza la lectura 
        fread(page, sizeof(char), 4096, file);                      // Leemos la pagina 
        for (int i = 0; i < 128; i++)                               // Recorremos los 128 directorios de la pagina (32 Bytes * 128 = 4096 Bytes)
        {
            unsigned int byte1 = page[i*32];                        // Tomamos el primer Byte del directorio
            int entry[2] = {0, 0};                                  // Arreglo de los bits que indican la validez de la entrada
            for (int j = 0; j < 8; j++)                             // Obtenemos los bits de entrada
            {
                unsigned int bit = byte1 & 0x080; 
                bit >>= 7;
                if (bit == 1)
                {
                    if (j == 6)
                    {
                        entry[0] = 1;
                    }
                    else if (j == 7)
                    {
                        entry[1] = 1;
                    }
                }
                byte1 <<= 1;
            }
            if (entry[0] == 0 && entry[1] == 1)                     // La entrada es un directorio
            {
                uint32_t indice = page[i*32 + 1] + (page[i*32 + 2]<<8) + (page[i*32 + 3]<<16) + (page[i*32 + 4]<<24); 
                int n_chars = 0;
                char name[27];                                      // Nombre directorio 
                for (int string = 0; string < 27; string++)         // Obtenemos el nombre del directorio
                {
                    uint8_t c = page[i*32 + 5 + string];
                    name[string] = c;
                    if (c != 0)
                    {
                        n_chars++;
                    }
                }
                for (int san = 0; san < sangria + 1; san++)
                {
                    printf("   ");
                }
                printf("%s/%s\n",dir,name);
                print_dir(name, indice, sangria + 1);               // Al se un directorio, debemos de buscar lo que esta en él
                break;
            }            
            else if (entry[0] == 1 && entry[1] == 1)                // La entrada es un archivo
            {
                int n_chars = 0;
                char name[27];                                      // Nombre archivo 
                for (int string = 0; string < 27; string++)         // Obtenemos el nombre del archivo
                {
                    uint8_t c = page[i*32 + 5 + string];
                    name[string] = c;
                    if (c != 0)
                    {
                        n_chars++;
                    }
                }
                for (int san = 0; san < sangria + 1; san++)
                {
                    printf("   ");
                }
                printf("%s/%s\n", dir,name);
            }                
        } 
        // Liberamos la página leida        
        free(page);
    }
    // Cerramos el archivo .bin
    fclose(file);
}

void os_tree()
{
    printf("\nTREE:\n");
    // Lectura paginas
    int block_init = 3;                                      // Inicio bloque 3 
    printf("\n");
    print_dir("~", block_init, 0);                           // Función recurisiva que imprime el arbol
    printf("\n");
    
}

/* Funciones de manejo de archivos */

int os_exists(char* filename){
    FILE *file;
    file = fopen(DISK_NAME, "rb"); // Abrir el disco
    if (file == NULL)
        exit(1);
    LocFile *archivo = malloc(sizeof(LocFile));
    char *copia_nombre = calloc(256, sizeof(char)); // Memoria para guardar el path y así poder manipular
    strcpy(copia_nombre, filename);
    char * token = strtok(copia_nombre, "/");
    while( token != NULL ) {
        if(!strcmp(token, "~")){ //Si es está en el directorio base
            archivo->bloque = 3;
        }else{
            if(!check_bloque(file, archivo->bloque)){
                printf("No se puede continuar leyendo ya que está rotten\n");
                free(copia_nombre);
                free(archivo);
                fclose(file);
                return 0;
            } else if(!search(file, archivo, token)){
                return 0;
            }
        }
        token = strtok(NULL, "/");
    }
    free(copia_nombre);
    free(archivo);
    return 1;
}


osFile* os_open(char* filename, char mode) {
    FILE *file;
    file = fopen(DISK_NAME, "rb+"); // Abrir el disco
    if (file == NULL)
        exit(1);

    osFile *archivo = malloc(sizeof(osFile));
    archivo->cursor = 0;
    if(mode == 'r') {
        char *copia_nombre = calloc(256, sizeof(char)); // Memoria para guardar el path y así poder manipular
        strcpy(copia_nombre, filename);
        char * token = strtok(copia_nombre, "/");
        char *nombre = calloc(28, sizeof(char)); //Memoria para el nombre del archivo/carpeta
        while( token != NULL ) {
            if(!strcmp(token, "~")){ //Si es está en el directorio base
                archivo->bloque_indice = 3;
            }else{
                llenar_osfile(file, archivo, token);
                if(!check_bloque(file, archivo->bloque_indice)){
                    printf("No se puede continuar leyendo ya que está rotten\n");
                    free(nombre);
                    free(copia_nombre);
                    free(archivo);
                    fclose(file);
                    return NULL;
                }
            }
            strcpy(nombre,token);
            token = strtok(NULL, "/");
        }
        free(copia_nombre);
        llenar_tamano(file,archivo,nombre);
        free(nombre);
        archivo->modo = 0;
    } else if(mode == 'w'){
        archivo->modo = 1;
        archivo->tamano = 0;
        archivo->buffer = NULL;
        archivo->bloque_ultimo = 0;
        archivo->pagina_ultimo = 0;
        archivo->utilizado = 0;
        char *copia_nombre = calloc(256, sizeof(char)); // Memoria para guardar el path y así poder manipular
        strcpy(copia_nombre, filename);
        char * token = strtok(copia_nombre, "/");
        char *nombre = calloc(28, sizeof(char)); //Memoria para el nombre del archivo/carpeta
        while( token != NULL ) {
            if(!strcmp(token, "~")){ //Si es está en el directorio base
                archivo->bloque_indice = 3;
            }else{
                if(!es_archivo(token)){
                    llenar_osfile(file, archivo, token);
                    if(!check_bloque(file, archivo->bloque_indice)){
                        printf("No se puede continuar ya que está rotten una de las paginas\n");
                        free(nombre);
                        free(copia_nombre);
                        free(archivo);
                        fclose(file);
                        return NULL;
                    }
                    
                }else if(comprobar_si_existe(file, token)){
                    printf("OYE! ya existe ese archivo!\n");
                    free(nombre);
                    free(copia_nombre);
                    free(archivo);
                    fclose(file);
                    return NULL;
                }else{
                    crear_archivo(file, archivo, token);
                    printf("Se ha creado archivo vacio en el bloque %d\n", archivo->bloque_indice);
                }
                
            }
            strcpy(nombre,token);
            token = strtok(NULL, "/");
        }
        free(copia_nombre);
        llenar_tamano(file,archivo,nombre);
        free(nombre);
        fclose(file);
    }
    // printf("La carpeta indice se encuentra en el bloque %d\n", archivo->bloque_indice);
    // printf("El tamaño2 es de: %zu Bytes\n", archivo->tamano);
    
    
    
    return archivo;
    
    
}

int os_read(osFile* file_desc, void* buffer, int nbytes) {
    if(file_desc->modo != 0){
        printf("Por favor abrir en modo lectura\n");
        return 0;
    }
    FILE *file;
    file = fopen(DISK_NAME, "rb+"); // Abrir el disco
    if (file == NULL)
        exit(1);
    int cont = 0;
    int cursor, bloque_indice=file_desc->bloque_indice;
    unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    unsigned char* bytes_return = calloc(nbytes, sizeof(unsigned char));

    unsigned char* page_indice = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    int posicion = ((bloque_indice)*256) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque
    fseek(file, posicion, SEEK_SET);
    fread(page_indice, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
    int byte_inicial = 8;
    while(cont != nbytes){
        cursor = file_desc->cursor;
        
        int bloque = cursor/(BYTES_PER_PAGE*PAGES_PER_BLOCK);
        bloque = byte_inicial + bloque*4;
        uint32_t dir_bloque = page_indice[bloque]+(page_indice[bloque+1]<<8)+(page_indice[bloque+2]<<16)+(page_indice[bloque+3]<<24);
        if(!check_bloque(file, bloque)){
            printf("No se puede continuar leyendo, se encontró bloque con pagina rotten");
            free(page);
            memcpy(buffer, bytes_return, nbytes);
            free(bytes_return);
            return cont;
        }
        int pagina = (cursor%(BYTES_PER_PAGE*PAGES_PER_BLOCK))/BYTES_PER_PAGE;

        int posicion = ((dir_bloque)*256 + pagina) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque
        fseek(file, posicion, SEEK_SET);
        fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
        int partida = (cursor%(BYTES_PER_PAGE*PAGES_PER_BLOCK))%BYTES_PER_PAGE;
        for(int i= partida; i<BYTES_PER_PAGE; i++){
            if(file_desc->cursor + 1 >= file_desc->tamano){
                printf("Se llegó al fin del archivo\n");
                free(page);
                memcpy(buffer, bytes_return, nbytes);
                free(bytes_return);
                return cont;
            }
            bytes_return[cont++] = page[i];
            file_desc->cursor++;
            if(cont == nbytes){
                free(page);
                memcpy(buffer, bytes_return, nbytes);
                free(bytes_return);
                return cont;
            }
        }
    }
    free(page);
    return cont;
    

    


}

int os_write(osFile* file_desc, void* buffer, int nbytes) {
    if(file_desc->modo != 1){
        printf("Por favor abrir en modo escritura\n");
        return 0;
    }
    FILE *file_disco;
    file_disco = fopen(DISK_NAME, "rb+"); // Abrir el disco
    if (file_disco == NULL)
        exit(1);
    unsigned char bytes_return[nbytes];
    memcpy(bytes_return, buffer, nbytes);
    int cantidad_escrita = 0;
    for(int i=0; i<nbytes; i++){
        if(file_desc->buffer == NULL){
            file_desc->buffer = calloc(BYTES_PER_PAGE, sizeof(unsigned char*));
            if(file_desc->bloque_ultimo == 0 || file_desc->pagina_ultimo == PAGES_PER_BLOCK){ // Caso en que parte, definir un nuevo bloque disponible
                uint32_t bloque_disponible = buscar_bloque_disponible(file_disco);
                if(!bloque_disponible){
                    free(file_desc->buffer);
                    printf("No queda bloque disponible\n");
                    return cantidad_escrita;
                }else if(!agregar_puntero(file_disco, file_desc->bloque_indice, bloque_disponible, file_desc->tamano)){
                    free(file_desc->buffer);
                    return cantidad_escrita;
                }
                file_desc->bloque_ultimo = bloque_disponible;
                file_desc->pagina_ultimo = 0;
            }else{
                file_desc->pagina_ultimo++;
            }
        }
        file_desc->buffer[(file_desc->utilizado)++] = bytes_return[i];
        cantidad_escrita++;
        file_desc->tamano++;
        if(file_desc->utilizado == BYTES_PER_PAGE){
            file_desc->utilizado = 0;
            int posicion = ((file_desc->bloque_ultimo)*256 + file_desc->pagina_ultimo) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque
            fseek(file_disco, posicion, SEEK_SET);
            printf("Posicion: %d", posicion);
            fwrite(file_desc->buffer, sizeof(char), BYTES_PER_PAGE, file_disco); //Escribimos lo que hay en el buffer
            free(file_desc->buffer);
        }
    }
    return cantidad_escrita;

}

int os_close(osFile* file_desc) {
    FILE *file_disco;
    file_disco = fopen(DISK_NAME, "rb+"); // Abrir el disco
    if (file_disco == NULL)
        exit(1);
    if(file_desc->buffer != NULL){
        int posicion = ((file_desc->bloque_ultimo)*256 + file_desc->pagina_ultimo) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque
        fseek(file_disco, posicion, SEEK_SET);
        fwrite(file_desc->buffer, sizeof(char), BYTES_PER_PAGE, file_disco); //Escribimos lo que hay en el buffer

        int posicion2 = ((file_desc->bloque_indice)*256) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque
        unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
        fseek(file_disco, posicion2, SEEK_SET);
        fread(page, sizeof(char), BYTES_PER_PAGE, file_disco); //Escribimos lo que hay en el buffer
        unsigned int tamano[8];
        for(int z=0; z<8; z++){
            tamano[z] = (file_desc->tamano & (0xff << z*8)) >> z*8;
            page[z] = tamano[z];
        }
        fseek(file_disco, posicion2, SEEK_SET);
        fwrite(page, sizeof(char), BYTES_PER_PAGE, file_disco); //Escribimos lo que hay en el buffer
        free(file_desc->buffer);
    }
    free(file_desc);
}

int os_rm(char* filename)
{
    /* 
    Elimina el archivo "char* filename" el cual es un PATH 
    El bloque Índice del archivo debe ser borrado (sus bits en 0) [Aumenta en 1 sus P/E asociados]
    Se debe actualizar el puntero de la entrada en la pagina del bloque Directorio que lo contiene [Aumenta su P/E]
    */
    // PATH de la forma: " ~/folder1/folder2/file.txt "
    // Paso1: Encontrar en el directorio la entrada al archivo (Puntero al bloque Índice)
    /* Realizar split segun "/" */
    char **split_filename = split_by_sep(filename, "/");
    
    /* Abrimos el archivo .bin */
    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL) {exit(1);}
    
    int file_index;
    int puntero_bloque_anterior = 3;
    int puntero_bloque = 3;
    /* Iterar en el directorio hasta llegar el ultimo elemento de "split_filename"*/
    int posicion = ((0*1024 + puntero_bloque)*256 + 0) * 4096; // Asignamos posición inicial (Bloque 3)

    int i_path = 1;
    while (i_path > -1)
    {
        for (int i_page = 0; i_page < 256; i_page++)
        {
            posicion += i_page*4096;
            char* page = malloc(sizeof(char) * 4096);  
            fseek(file, posicion, SEEK_SET); 
            fread(page, sizeof(char), 4096, file);

            for (int i_byte = 0; i_byte < 4096; i_byte+=32)     // Recorremos cada 32 bytes (tamano entrada)
            {
                /* Obtenemos el nombre de la entrada */
                unsigned char letra;
                char nombre[28];
                int cont = 5;
                while ((letra = page[i_byte+cont]) != 0 && cont < 32)
                {
                    nombre[cont-5] = letra;
                    cont++;
                }
                nombre[cont-5] = '\0';

                /* Verificar validez y nombre de la entrada */
                if (page[i_byte] == 1 && split_filename[i_path] == nombre)          // Byte de validez == a directorio
                {
                    /* Encontramos un Directorio. Pasar al siguiente i_path */
                    i_path += 1;
                    puntero_bloque_anterior = puntero_bloque;
                    puntero_bloque = page[i_byte+1]+(page[i_byte+2]<<8)+(page[i_byte+3]<<16)+(page[i_byte+4]<<24);
                    posicion = (puntero_bloque * 256 + 0) * 4096;
                    break;
                }    
                else if (page[i_byte] == 3 && split_filename[i_path] == nombre)     // Byte de validez == a archivo
                {
                    /* Encontramos el archivo "filename". Borrar Bloque Indice y entrada en Directorio */
                    i_path = -1; // Indicamos el fin del Bucle while
                    
                    puntero_bloque_anterior = puntero_bloque;
                    puntero_bloque = page[i_byte+1]+(page[i_byte+2]<<8)+(page[i_byte+3]<<16)+(page[i_byte+4]<<24); // Puntero al Bloque indice del archivo
                    file_index = (puntero_bloque * 256 + 0) * 4096;     // Posicion de Bloque Indice
                    
                    char* page_indice = malloc(sizeof(char) * 4096);    // Abrimos la pagina 0 del Bloque indice del archivo
                    fseek(file, file_index, SEEK_SET); 
                    fread(page_indice, sizeof(char), 4096, file);
                    
                    char* page_0 = malloc(sizeof(char) * 4096);         // Abrimos la pagina 0 del Bloque Bitmap
                    fseek(file, 0, SEEK_SET); 
                    fread(page_0, sizeof(char), 4096, file);

                    // Marcar desocupados los bloque de datos del archivo en el Bitmap
                    for (int j = 8; j < 4056; j+=4)
                    {
                        int puntero_dato = page_indice[j]+(page_indice[j+1]<<8)+(page_indice[j+2]<<16)+(page_indice[j+3]<<24);
                        int despues = page_0[puntero_dato >> 3] & ~(1 << (7 - (puntero_dato & 7)));
                        page_0[puntero_dato >> 3] = despues;
                        fseek(file, 0, SEEK_SET);
                        fwrite(page_0, sizeof(char), 4096, file);
                    }

                    // Marcar desocupado el bloque Indice en el Bitmap

                    // Borramos el contenido del Bloque Indice
                    char *page_vacia = calloc(4096, sizeof(char));
                    fseek(file, file_index, SEEK_SET); 
                    fwrite(page_vacia, sizeof(char), 4096, file);
                    aumentar_lifmap(file, puntero_bloque, 0, LIFE);

                    free(page_indice);
                    free(page_0);
                    free(page_vacia);

                    // Borramos la entrada en el directorio
                    for (int j_byte = 0; j_byte < 32; j_byte++)
                    {
                        page[i_byte + j_byte] = 0;
                    }
                    fseek(file, posicion, SEEK_SET); 
                    fwrite(page, sizeof(char), 4096, file);
                    aumentar_lifmap(file, puntero_bloque_anterior, i_page, LIFE);
                    break; 
                }
            }
            free(page);
            if (i_path == -1){break;}   // Rompemos el ciclo for cuando encontramos el archivo   
        }
    }
 
    fclose(file);
    return 0;
}

int os_mkdir(char* path)
{
    /* 
    Crea un directorio con el nombre "char* path"
    Incrementar en 1 el P/E de las páginas que sea necesario actualizar para crear las referencias a este directorio
    */
    char **split_path = split_by_sep(path, "/");
    int len_path = sizeof(split_path)/sizeof(char*);
    
    char *nombre_predirectorio = split_path[len_path-2];
    char *nombre_directorio = split_path[len_path-1];
    
    /* Abrimos el archivo .bin */
    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL) {exit(1);}

    int puntero_bloque_anterior = 3;
    int puntero_bloque = 3;

    int posicion = ((0*1024 + puntero_bloque)*256 + 0) * 4096;   // Posicion del primer bloque directorio "ROOT""
    int i_actual = 0;
    int i_path = 0;
    while (i_path > -1)
    {
        for (int i_page = 0; i_page < 256; i_page++)
        {
            posicion += i_page*4096;
            char* page = malloc(sizeof(char) * 4096);  
            fseek(file, posicion, SEEK_SET); 
            fread(page, sizeof(char), 4096, file);

            for (int i_byte = 0; i_byte < 4096; i_byte+=32)
            {
                /* Obtenemos el nombre de la entrada */
                unsigned char letra;
                char nombre[28];
                int cont = 5;
                while ((letra = page[i_byte+cont]) != 0 && cont < 32)
                {
                    nombre[cont-5] = letra;
                    cont++;
                }
                nombre[cont-5] = '\0';

                if (page[i_byte] == 1 && split_path[i_path] == nombre)      // Byte de validez == a directorio
                {
                    if (split_path[i_path] == nombre_predirectorio)         // Se encuentra el penultimo DIR, donde se creara el nuevo DIR
                    {
                        i_path = -1;    // Indicamos el fin del Bucle While()

                        /* Marcar ocupado un nuevo bloque Directorio en el Bitmap */
                        //void marcar_bloque_ocupado(FILE *file_disco, uint32_t dir_bloque){
                        char* page0 = malloc(sizeof(char) * 4096);  
                        fseek(file, 0, SEEK_SET); 
                        fread(page0, sizeof(char), 4096, file);
                        int contador = 0; 
                        for (int j = 1; j < 256; j++)           // Iterar sobre Bytes
                        {
                            unsigned int byte = page0[j];
                            for (int k = 0; k < 8; k++)         // Iterar sobre Bytes
                            {
                                unsigned int bit = byte & 0x080;
                                bit >>= 7;
                                contador++;
                                if (bit == 0)                    // Bloque desocupado
                                { 
                                    marcar_bloque_ocupado(file, contador);
                                    break;
                                }
                                byte <<= 1;
                            }
                        }
                        free(page0);

                        /* Crear la entrada en el ultimo bloque encontrado */
                        puntero_bloque_anterior = puntero_bloque;
                        puntero_bloque = page[i_byte+1]+(page[i_byte+2]<<8)+(page[i_byte+3]<<16)+(page[i_byte+4]<<24);
                        posicion = (puntero_bloque * 256 + 0) * 4096;

                        char* page_predireccion = malloc(sizeof(char) * 4096);
                        fseek(file, posicion, SEEK_SET); 
                        fread(page_predireccion, sizeof(char), 4096, file);
                        
                        char entrada[32];
                        
                        for (int j = 0; j < 4096; j+=32)
                        {
                            if (page_predireccion[i_byte] == 0)
                            {
                                // Escribir datos de la entrada
                                page_predireccion[i_byte] = 1;

                            }
                        }
                        fseek(file, posicion+i_byte, SEEK_SET); 
                        fread(page_predireccion, sizeof(char), 4096, file);
                        aumentar_lifmap(file, puntero_bloque_anterior, i_page, LIFE);
                        free(page_predireccion);
                    }
                    else 
                    {
                        i_path++;
                        puntero_bloque_anterior = puntero_bloque;
                        puntero_bloque = page[i_byte+1]+(page[i_byte+2]<<8)+(page[i_byte+3]<<16)+(page[i_byte+4]<<24);
                        posicion = (puntero_bloque * 256 + 0) * 4096;
                    }
                }  
            }            
            free(page);
            if (i_path == -1){break;}   // Rompemos el ciclo for cuando encontramos el archivo   
        }
    }
    
    /* Comprobar que no existe el ultimo directorio soliitado antes de crearlo?? */
    
    fclose(file);
    return 0;
}

int os_rmdir(char* path)
{
    /*
    Elimina un directorio vacio con el nombre indicado.
    Esto incrementara en 1 el contador P/E de las paginas que sea necesario actualizar para borrar 
    las referencias a este directorio.
    */
    /* Separar el path */
    char **split_path = split_by_sep(path, "/");
    int len_path = sizeof(split_path)/sizeof(char*);

    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL) {exit(1);}

    int puntero_bloque_anterior = 3;
    int puntero_bloque = 3;

    /* Buscamos el ultimo bloque directorio */
    int ultima_posicion = (puntero_bloque * 256) * 4096;
    int posicion = (puntero_bloque * 256) * 4096;           // Posicion del primer Bloque Directorio "Root"
    
    int i_path = 1;
    while (i_path > -1)
    {
        for (int i_page = 0; i_page < 256; i_page++)
        {
            posicion += i_page*4096;
            char* page = malloc(sizeof(char) * 4096);  
            fseek(file, posicion, SEEK_SET); 
            fread(page, sizeof(char), 4096, file);

            for (int i_byte = 0; i_byte < 4096; i_byte+=32)   // Recorremos cada 32 bytes (tamano entrada)
            {
                /* Obtener el nombre de la entrada */
                unsigned char letra;
                char nombre[28];
                int cont = 5;
                while ((letra = page[i_byte+cont]) != 0 && cont < 32)
                {
                    nombre[cont-5] = letra;
                    cont++;
                }
                nombre[cont-5] = '\0';

                /* Verificar validez y nombre de la entrada */
                if (page[i_byte] == 1 && split_path[i_path] == nombre)          // Byte de validez == a directorio
                {
                    /* Actualizar posicion */
                    ultima_posicion = posicion;
                    puntero_bloque = page[i_byte+1]+(page[i_byte+2]<<8)+(page[i_byte+3]<<16)+(page[i_byte+4]<<24);
                    posicion = (puntero_bloque * 256 + 0) * 4096;

                    /* Comprobar si i_path es la ultima iteracion del Bucle while() */
                    if (split_path[i_path] == split_path[len_path-1])
                    {
                        i_path = -1;        // Indicamos el fin del Bucle while

                        /* Borrar el bloque Directorio */
                        // char* page_indice = malloc(sizeof(char) * 4096);    // Abrimos la pagina 0 del Bloque indice del archivo
                        // fseek(file, posicion, SEEK_SET); 
                        // fread(page_indice, sizeof(char), 4096, file);

                        char* page_0 = malloc(sizeof(char) * 4096);         // Abrimos la pagina 0 del Bloque Bitmap
                        fseek(file, 0, SEEK_SET); 
                        fread(page_0, sizeof(char), 4096, file);
                        
                        // Marcar desocupado en Bitmap
                        

                        // Borrar entrada en el bloque Directorio anterior (ultima_posicion)
                        for (int j = 0; j < 32; j++) { page[i_byte + j] = 0; }
                        fseek(file, ultima_posicion, SEEK_SET); 
                        fwrite(page, sizeof(char), 4096, file);
                        aumentar_lifmap(file, puntero_bloque_anterior, i_page, LIFE);
                    }
                    else { i_path++; }      // Pasar al siguiente i_path

                    break;
                }
            }
            free(page);
            if (i_path == -1){break;}   // Rompemos el ciclo for cuando encontramos el archivo   
        }
    }

    // Paso3: Comprobar que el bloque Directorio se encuentre vacio
    // for (int i_page = 0; i_page < 256; i_page++)
    // {
    //     posicion += i_page*4096;
    //     char* page = malloc(sizeof(char) * 4096);  
    //     fseek(file, posicion, SEEK_SET); 
    //     fread(page, sizeof(char), 4096, file);
    //     for (int i_byte = 0; i_byte < 4096; i_byte++)
    //     {
    //         if (i_byte % 32 == 0)
    //         {
    //             unsigned int byte = page[i_byte];
    //             int mibits[2];
    //             for (int j = 6; j < 8; j++)
    //             {
    //                 unsigned int bit = byte & 0x080; 
    //                 bit >>= 7;
    //                 mibits[j] = bit;
    //                 byte <<= 1;
    //             }
    //             sprintf("%d%d", mibits[0], mibits[1]);
    //             if ((mibits[0] != 0) && (mibits[1] != 0))
    //             {
    //                 printf("Error: El directorio no se encuentra vacio. No se puede borrar\n");
    //                 fclose(file);
    //                 return 1;
    //             }
    //         }
    //     }
    // }

    fclose(file);
    return 0;
}

int os_rmrfdir(char* path)
{
    /*
    Elimina un directorio con el nombre indicado, todos sus archivos y subdirectorios correspondientes.
    Esto incrementa en 1 el contador P/E de las paginas que sea necesario actualizar.
    */
   /* Conseguir la direccion/puntero del directorio a borrar */
    char **split_path = split_by_sep(path, "/");
    int len_path = sizeof(split_path)/sizeof(char*);

    FILE *file;
    file = fopen(DISK_NAME, "rb");
    if (file == NULL) {exit(1);}

    int puntero_bloque = 3;

    int ultima_posicion = (puntero_bloque * 256) * 4096;
    int actual_posicion = (puntero_bloque * 256) * 4096;
    
    int i_path = 1;
    while (i_path > -1)
    {
        for (int i_page = 0; i_page < 256; i_page++)
        {
            actual_posicion += i_page*4096;
            char* page = malloc(sizeof(char) * 4096);  
            fseek(file, actual_posicion, SEEK_SET); 
            fread(page, sizeof(char), 4096, file);

            for (int i_byte = 0; i_byte < 4096; i_byte+=32)
            {
                /* Obtenemos el nombre de la entrada */
                unsigned char letra;
                char nombre[28];
                int cont = 5;
                while ((letra = page[i_byte+cont]) != 0 && cont < 32)
                {
                    nombre[cont-5] = letra;
                    cont++;
                }
                nombre[cont-5] = '\0';

                /* Verificar validez y nombre de la entrada */
                if (page[i_byte] == 1 && split_path[i_path] == nombre)          // Byte de validez == a directorio
                {
                    /* Actualizar posiciones */
                    ultima_posicion = actual_posicion;
                    puntero_bloque = page[i_byte+1]+(page[i_byte+2]<<8)+(page[i_byte+3]<<16)+(page[i_byte+4]<<24);
                    actual_posicion = (puntero_bloque * 256 + 0) * 4096;
                    
                    /* Comprobar si i_path es la ultima iteracion del Bucle while() */
                    if (split_path[i_path] == split_path[len_path-1]) {i_path = -1;}  // Indicamos el fin del Bucle while
                    else {i_path++;}                                                    // Pasar al siguiente i_path
                    
                    break;
                }
            }    
            free(page);
            if (i_path == -1){break;}   // Rompemos el ciclo for cuando encontramos el archivo   
        }
    }

    for (int i_page = 0; i_page < 256; i_page++)
    {
        actual_posicion += i_page*4096;
        char* page = malloc(sizeof(char) * 4096);  
        fseek(file, actual_posicion, SEEK_SET); 
        fread(page, sizeof(char), 4096, file);

        for (int i_byte = 0; i_byte < 4096; i_byte+=32)
        {
            /* Obtenemos el nombre de la entrada */
                unsigned char letra;
                char nombre[28];
                int cont = 5;
                while ((letra = page[i_byte+cont]) != 0 && cont < 32)
                {
                    nombre[cont-5] = letra;
                    cont++;
                }
                nombre[cont-5] = '\0';

                /* Concatenar el nombre con la cadena de PATH */
                char* aux_path[4096] = path;
                strcat(aux_path, "/");
                strcat(aux_path, nombre);

                /* Verificar validez y nombre de la entrada */
                if (page[i_byte] == 1) { os_rmrfdir(aux_path); }    // Byte de validez == a directorio
                else if (page[i_byte] == 3) { os_rm(aux_path); }    // Byte de validez == a archivo
        }
    }

    /* Llamar os_rmdir(path) */
    os_rmdir(path);
    
    fclose(file);
    return 0;
}

//Supuesto, orig siempre parte con ~/ y dest es una ruta relativa
//Ej:  os_unload("~/dir1", "./");

int os_unload(char* orig, char* dest) {
    FILE *file;
    file = fopen(DISK_NAME, "rb"); // Abrir el disco
    if (file == NULL)
        printf("jal");
        exit(1);

    LocFile *carpetarchivo = calloc(1, sizeof(LocFile)); // Petición de memoria para guardar datos del archivo

    
    char *string = calloc(500, sizeof(char)); // Memeoria para guardar la dirección de origen y así poder manipular
    strcpy(string, orig);
    char * token = strtok(string, "/");
    char *nombre = calloc(28, sizeof(char)); //Memoria para el nombre del archivo/carpeta
    while( token != NULL ) {
      if(!strcmp(token, "~")){ //Si es está en el directorio base
          carpetarchivo->bloque = 3; 
          carpetarchivo->tipo = DIRECTORIO;
      }else{
          search(file, carpetarchivo, token);
          if(!check_bloque(file, carpetarchivo->bloque)){
              printf("No se puede continuar leyendo ya que está rotten\n");
              free(nombre);
              free(string);
              free(carpetarchivo);
              fclose(file);
              return -1;
          }
      }
      strcpy(nombre,token);
      token = strtok(NULL, "/");
   }
    free(string);

    if(carpetarchivo->tipo){
        FILE *file_salida;
        file_salida = fopen(dest, "wb");
        write(file, file_salida, carpetarchivo);
        fclose(file_salida);
        free(carpetarchivo);
    }else{   
        search_and_write(file, dest, carpetarchivo->bloque, nombre);
        free(carpetarchivo);
    }

    free(nombre);
    fclose(file);
}

int os_load(char* orig){
    int tipo = es_archivo(orig); // 0: Directorio, 1:Archivo
    FILE *file;
    file = fopen(DISK_NAME, "rb+");
    if (file == NULL){
        return -1;
    }
    if(tipo){
        FILE *file_entrada;
        file_entrada = fopen(orig, "rb");
        if (file_entrada == NULL){
            printf("No se pudo abrir el archivo de origen\n");
            return -1;
        }
        char *nombre = obtener_nombre(orig);
        if(!comprobar_si_existe(file, nombre)){
            printf("Ya existe el archivo, no se va a copiar!\n");
        }else{
            write_to_disk(file, file_entrada, nombre, LIFE);
        }
        

        //Liberamos memoria y cerramos archivos
        free(nombre);
        fclose(file_entrada);
        
    }else{ // https://poesiabinaria.net/2011/09/listar-archivos-dentro-de-un-directorio-o-carpeta-en-c/
        DIR *dir;
        FILE *file_entrada;
        struct dirent *ent;
        int lenght;
        char direccion[200]; // Copiamos origen a este char para poder manipular
        char direct[200]; // Perteneciente a cada archivo si es que se trata de carpeta

        dir = opendir(orig);
        if (dir == NULL){
            printf("No se puede abrir el directorio\n");
            return -1;
        }
        
        strcpy(direccion, orig);

        //En caso de que el directorio no termine con '/' ponerlo al final
        lenght = len(direccion);
        if(direccion[lenght-1] != '/'){
            direccion[lenght] = '/';
            direccion[lenght+1] = '\0';
        }
        
        //Recorrer cada archivo de la carpeta
        while ((ent = readdir (dir)) != NULL){
            strcpy(direct, direccion);
            if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) ){
                /* Una vez tenemos el archivo, lo pasamos a una función para procesarlo. */
                tipo = es_archivo(ent->d_name); //Solo si es archivo omitiendo todas las carpetas
                if(tipo){
                    if(!comprobar_si_existe(file, ent->d_name)){
                        strcat(direct, ent->d_name);
                        file_entrada = fopen(direct, "rb");
                        write_to_disk(file, file_entrada, ent->d_name, LIFE);
                        fclose(file_entrada);
                    }else{
                        printf("El archivo '%s' ya existía, no se copiará\n", ent->d_name);
                    }
                    
                }
            }
        }
        closedir(dir);

    }
    fclose(file);
    return 0;
}



