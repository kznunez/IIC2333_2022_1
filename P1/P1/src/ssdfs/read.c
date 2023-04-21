#include "./read.h"

#include "string.h"


/**
 * es_archivo(char*): Recibe un string luego retorna para identificar si es archivo o una carpeta(directorio).
 * retorna 1 en caso de que sea archivo 0 en cualquier otro caso
 */
int es_archivo(char* nombre){
    int i=0;
    for(char j = nombre[i]; j != '\0'; i++){
        j = nombre[i];
        if((j == '.' && nombre[i+1] != '.') && (j == '.' && nombre[i+1] != '/')){
            //printf("Es archivo\n");
            return 1;
        }
        
    }
    return 0;
}
/**
 * search(FILE *, LocFile, char*): Busca dentro del disco el nombre del archivo o carpeta y actualiza el LocFile según corresponda.
 * no retorna nada
 */

int comprobar_si_existe(FILE *file, char* buscado){
    uint32_t bloque = 3;
    for(int i=0; i< 256; i++){
        int posicion = (bloque * PAGES_PER_BLOCK + i) * BYTES_PER_PAGE; // Asignamos posición inicial
        unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
        fseek(file, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
        fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
        char nombre[28];
        for(int j = 0; j < BYTES_PER_PAGE; j+=BYTES_PER_DIRECTORY_ENTRY){
            unsigned int byte = page[j];
            unsigned char letra;
            if(byte == 3){
                int cont = 5;
                while ((letra = page[j+cont]) != 0 && cont < 32){
                    nombre[cont-5] = letra;
                    cont++;
                };
                nombre[cont-5] = '\0';
                if(!strcmp(nombre, buscado)){
                    free(page);
                    return 1;
                }
                
            }
        }
        free(page);
    }
    return 0;
}
int search(FILE *file, LocFile *archivo, char *buscado){ //Buscará en el bloque indicado, el archivo o directorio buscado. retornará 1 si lo encuentra 0 si no
    uint32_t bloque = archivo->bloque;
    for(int i=0; i< 256; i++){
        int posicion = (bloque * PAGES_PER_BLOCK + i) * BYTES_PER_PAGE; // Asignamos posición inicial
        unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
        fseek(file, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
        fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
        char nombre[28];
        for(int j = 0; j < BYTES_PER_PAGE; j+=BYTES_PER_DIRECTORY_ENTRY){
            unsigned int byte = page[j];
            unsigned char letra;
            if(byte == 3 || byte == 1){
                int cont = 5;
                while ((letra = page[j+cont]) != 0 && cont < 32){
                    nombre[cont-5] = letra;
                    cont++;
                };
                nombre[cont-5] = '\0';
                if(!strcmp(nombre, buscado)){
                    uint32_t dir_bloque = page[j+1]+(page[j+2]<<8)+(page[j+3]<<16)+(page[j+4]<<24);
                    archivo->bloque = dir_bloque;
                    if(byte == 3){
                        archivo->tipo = ARCHIVO;
                    }else if(byte == 1){
                        archivo->tipo = DIRECTORIO;
                    }else{
                        archivo->tipo = INVALIDO;
                    }
                    free(page);
                    return 1;
                }
                
            }
        }
        free(page);
    }
    return 0;
}

int write(FILE *file_disco, FILE *file_salida, LocFile *archivo){
    uint32_t bloque = archivo->bloque;
    unsigned char* page0 = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes para la pagina 0
    unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    unsigned char* page1 = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 

    int posicion = (bloque * PAGES_PER_BLOCK) * BYTES_PER_PAGE; // Asignamos posición inicial

    fseek(file_disco, 0, SEEK_SET);            // Establecemos la posicion en 0 para revisar el bitmap
    fread(page0, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina
    fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura
    fread(page, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina
    archivo->tamano = 0;
    for(int i= 0; i< 8; i++){
        archivo->tamano += (page[i]<<(i*8));
    }
    long long int contador = 0;
    for(int j = 8; j < BYTES_PER_PAGE; j+=4){
        uint32_t dir_bloque = page[j]+(page[j+1]<<8)+(page[j+2]<<16)+(page[j+3]<<24);
        uint32_t bloque_ocupado = (page0[dir_bloque >> 3] & (1 << (7 - (dir_bloque & 7))));
        if(bloque_ocupado && dir_bloque){
            if(!check_bloque(file_disco, dir_bloque)){
                printf("Una de las paginas está rotten, no se copiará mas el archivo\n");
                free(page1);
                free(page0);
                free(page);
                return 0;
            }
            for(int i = 0; i < 256; i++){
                int posicion = ((dir_bloque*PAGES_PER_BLOCK)+i) * BYTES_PER_PAGE; // Asignamos posición inicial
                fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura
                fread(page1, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina

                for(int z=0; z < BYTES_PER_PAGE; z++){
                    fwrite(&page1[z], sizeof(char), 1, file_salida);
                    contador++;
                    if(contador == archivo->tamano){
                        break;
                    }
                }
                if(contador == archivo->tamano){
                    break;
                }
            }
        }else{
            break;
        }
    }
    free(page1);
    free(page0);
    free(page);
    return 1;

}

void search_and_write(FILE *file_disco, char *dest, uint32_t bloque, char *nombre_dir){
    char destino[100];
    strcpy(destino, dest);
    if(dest[2] != '\0'){
        strcat(destino, "/");
    }
    strcat(destino, nombre_dir);
    
    mkdir(destino, 0777);
    
    //unsigned int bloque = archivo->bloque;
    char nombre[27];

    for(int i=0; i< PAGES_PER_BLOCK; i++){ // Recorremos cada pagina
        int posicion = (bloque * PAGES_PER_BLOCK + i) * BYTES_PER_PAGE; // Asignamos posición inicial
        unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
        fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
        fread(page, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina
        for(int j = 0; j < BYTES_PER_PAGE; j+=BYTES_PER_DIRECTORY_ENTRY){
            unsigned int byte = page[j];
            unsigned char letra;
            if(byte == 3 || byte == 1){
                int cont = 5;
                while ((letra = page[j+cont]) != 0 && cont < 32){
                    nombre[cont-5] = letra;
                    cont++;
                };
                nombre[cont-5] = '\0';
                uint32_t dir_bloque = page[j+1]+(page[j+2]<<8)+(page[j+3]<<16)+(page[j+4]<<24);
                if(byte == 1){
                    search_and_write(file_disco, destino, dir_bloque, nombre);
                }else{
                    char destino_nuevo[100];
                    strcpy(destino_nuevo, destino);
                    strcat(destino_nuevo, "/");
                    strcat(destino_nuevo, nombre);
                    FILE *file_salida;
                    file_salida = fopen(destino_nuevo, "wb");
                    LocFile *carpetarchivo = calloc(1, sizeof(LocFile));
                    carpetarchivo->bloque = dir_bloque;
                    write(file_disco, file_salida, carpetarchivo);
                    free(carpetarchivo);
                    fclose(file_salida);
                }
                
            }
        }
        free(page);
    }
    return;
}

void write_to_disk(FILE *file_disco, FILE *file_entrada, char *nombre, unsigned life_max){
    // Registrar tamaño
    fseek(file_entrada, 0L, SEEK_END);
    size_t file_size = ftell(file_entrada);

    //cargar pagina 0 para revisar bit disponibles
    unsigned char* page0 = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes para la pagina 0
    fseek(file_disco, 0, SEEK_SET);            // Establecemos la posicion en 0 para revisar el bitmap
    fread(page0, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina

    //Recorrer desde el bloque 4
    for(uint32_t dir_bloque = 4; dir_bloque < 2048; dir_bloque++){
        uint32_t bloque_ocupado = (page0[dir_bloque >> 3] & (1 << (7 - (dir_bloque & 7)))); //Obtener el valor del bitmap
        if(!bloque_ocupado){ //Si no está ocupado
            int pagina_lif = (dir_bloque * 1024)/BYTES_PER_PAGE;
            uint32_t bloque_lif = (pagina_lif/256) + 1;
            pagina_lif %= 256;
            int inicio_lif = (dir_bloque * 1024)%BYTES_PER_PAGE;
            int posicion = ((bloque_lif)*256 + pagina_lif) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque

            fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion para revisar el lifemap
            unsigned char* lif_page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
            fread(lif_page, sizeof(char), BYTES_PER_PAGE, file_disco);
            int roto = 0;
            for(int lp = inicio_lif; lp < inicio_lif+1024; lp+=4){
                int life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
                if(life < 0){
                    roto = 1;
                    break; // Si está roto entonces dejar de revisar;
                }
                
            }
            
            free(lif_page);
            if(roto){
                continue; // Si está rota alguna pagina buscar en el siguiente bloque
            }
            int encontrado = 0;
            for(int i_dir=0; i_dir< 256; i_dir++){ // Recorrer bloque 3(Directorio base)
                posicion = (3*256 + i_dir) * BYTES_PER_PAGE;
                unsigned char* pag_direct = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
                fseek(file_disco, posicion , SEEK_SET);            // Establecemos la posicion
                fread(pag_direct, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina
                int pos_write=404;
                for(int j = 0; j < BYTES_PER_PAGE; j+=32){ // Recorrer cada pagina
                    unsigned int byte = pag_direct[j];
                    if(byte == 0){
                        
                        pos_write = j;

                        unsigned int bit_valides = 3; 
                        unsigned int dir1 = (unsigned)dir_bloque & 0xff;
                        unsigned int dir2 = ((unsigned)dir_bloque & 0xff00) >> 8;
                        unsigned int dir3 = ((unsigned)dir_bloque & 0xff0000) >> 16;
                        unsigned int dir4 = ((unsigned)dir_bloque & 0xff000000) >> 24;

                        pag_direct[pos_write++] = bit_valides; //Registrar existencia de un archivo 
                        pag_direct[pos_write++] = dir1; // Registrar puntero del bloque
                        pag_direct[pos_write++] = dir2;
                        pag_direct[pos_write++] = dir3;
                        pag_direct[pos_write++] = dir4;
                        for(int pos=0; pos<len(nombre); pos++){
                            pag_direct[pos_write++] = nombre[pos]; // Registrar nombre del bloque
                        }

                        fseek(file_disco, posicion , SEEK_SET);            // Establecemos la posicion de vuelta
                        fwrite(pag_direct, sizeof(char), BYTES_PER_PAGE, file_disco);      // Volvemos a llenar la pagina
                        
                        
                        encontrado = 1;
                        break;
                    }
                }
                free(pag_direct);
                
                if(encontrado){
                    marcar_bloque_ocupado(file_disco, dir_bloque);
                    break;
                }
            }
            if(encontrado){
                unsigned char* pag_direct = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
                int posicion_indice = ((dir_bloque)*256 + 0) * BYTES_PER_PAGE; // Dirigirse al bloque indice del archivo

                fseek(file_disco, posicion_indice, SEEK_SET);
                fread(pag_direct, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina
                unsigned int tamano[8];
                for(int z=0; z<8; z++){
                    tamano[z] = (file_size & (0xff << z*8)) >> z*8;
                    pag_direct[z] = tamano[z];
                }

                

                

                uint32_t bloque_a_ocupar;
                fseek(file_entrada, 0, SEEK_SET); // Definimos en posicion 0 al disco a copiar
                size_t memoria_restante = file_size;
                
                for(int i = 0; i < 1012; i++){
                    if(memoria_restante == 0){
                        fseek(file_disco, posicion_indice, SEEK_SET);
                        fwrite(pag_direct, sizeof(char), BYTES_PER_PAGE, file_disco);      // Volvemos a llenar la pagina
                        free(pag_direct);
                        break;
                    }
                    bloque_a_ocupar = buscar_bloque_disponible(file_disco);
                    if(bloque_a_ocupar){
                        marcar_bloque_ocupado(file_disco, bloque_a_ocupar);
                        memoria_restante = rellenar_bloque(file_disco, file_entrada, bloque_a_ocupar, memoria_restante, life_max);
                        unsigned int dir1 = (unsigned)bloque_a_ocupar & 0xff;
                        unsigned int dir2 = ((unsigned)bloque_a_ocupar & 0xff00) >> 8;
                        unsigned int dir3 = ((unsigned)bloque_a_ocupar & 0xff0000) >> 16;
                        unsigned int dir4 = ((unsigned)bloque_a_ocupar & 0xff000000) >> 24;
                        int pos_puntero = (4*i)+8;
                        pag_direct[pos_puntero++] = dir1;
                        pag_direct[pos_puntero++] = dir2;
                        pag_direct[pos_puntero++] = dir3;
                        pag_direct[pos_puntero++] = dir4;

                    
                    }else{
                        fseek(file_disco, posicion_indice, SEEK_SET);
                        fwrite(pag_direct, sizeof(char), BYTES_PER_PAGE, file_disco);      // Volvemos a llenar la pagina
                        free(pag_direct);
                        printf("No queda bloques disponibles\n");
                        break;
                    }
                }
                if(memoria_restante != 0){
                    printf("Ups, lo lamento\n");
                }
                break;

            }
        }
    }
    

    
    free(page0);
    return;

}

size_t rellenar_bloque(FILE *file_disco, FILE *file_entrada, uint32_t bloque, size_t memoria_restante, unsigned life_max){ 
    int posicion = ((bloque)*256 + 0) * BYTES_PER_PAGE; // Partimos el bloque
    fseek(file_disco, posicion, SEEK_SET);
    unsigned char* page = calloc(BYTES_PER_PAGE,sizeof(char)); //Copiamos datos de tamaño de una pagina, para insertar al disco
    
    int cantidad = 4096;
    size_t mimemoria = memoria_restante;
    for(int i=0; i< 256; i++){ //Rellenamos las paginas
        fread(page, sizeof(char), BYTES_PER_PAGE, file_entrada);
        if(mimemoria < cantidad){//-----------------------------------------------------------------------
            aumentar_lifmap(file_disco, bloque, i, life_max);
            unsigned int ultima_pos = ftell(file_disco);
            unsigned char* page_disco = calloc(BYTES_PER_PAGE,sizeof(char)); //Copiamos datos de tamaño de una pagina, para insertar al disco
            fread(page_disco, sizeof(char), BYTES_PER_PAGE, file_disco);
            int rec = 0;
            for(int mim=0; i<mimemoria; i++){
                page_disco[rec++] = page[mim];
            }
            for(int mim=0; mim<(4096-mimemoria); mim++){
                page_disco[rec++] = 0;
            }
            
            fseek(file_disco, ultima_pos, SEEK_SET);
            fwrite(page_disco, sizeof(char), BYTES_PER_PAGE, file_disco);
            free(page_disco);
            free(page);
            return 0;
        }
        aumentar_lifmap(file_disco, bloque, i, life_max);
        fwrite(page, sizeof(char), BYTES_PER_PAGE, file_disco);
        mimemoria -= BYTES_PER_PAGE;
    }
    free(page);
    return mimemoria;
}

uint32_t buscar_bloque_disponible(FILE *file_disco){
    unsigned char* page0 = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes para la pagina 0
    fseek(file_disco, 0, SEEK_SET);            // Establecemos la posicion en 0 para revisar el bitmap
    fread(page0, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina
    
    for(uint32_t dir_bloque = 4; dir_bloque < 2048; dir_bloque++){
        int bloque_ocupado = (page0[dir_bloque >> 3] & (1 << (7 - (dir_bloque & 7))));
        if(!bloque_ocupado){ // Si bloque no está marcado como ocupado
            int pagina_lif = (dir_bloque * 1024)/BYTES_PER_PAGE;
            uint32_t bloque_lif = (pagina_lif/256) + 1;
            pagina_lif %= 256;
            int inicio_lif = (dir_bloque * 1024)%BYTES_PER_PAGE;
            int posicion = ((bloque_lif)*256 + pagina_lif) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque

            fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion para revisar el lifemap
            unsigned char* lif_page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
            fread(lif_page, sizeof(char), BYTES_PER_PAGE, file_disco);
            int roto = 0;
            for(int lp = inicio_lif; lp < inicio_lif+1024; lp+=4){
                int life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
                if(life < 0){
                    roto = 1;
                    break; // Si está roto entonces dejar de revisar;
                }
                
            }
            
            free(lif_page);
            if(roto){
                continue; // Si está rota alguna pagina buscar en el siguiente bloque
            }
            free(page0);
            return dir_bloque;
        }

    }
    free(page0);
    return 0;
}

//Funcion que revisa los life de las paginas de cierto bloque
int check_bloque(FILE *file_disco, uint32_t bloque){
    int ultima_pos = ftell(file_disco);

    int pagina_lif = (bloque * 1024)/BYTES_PER_PAGE;
    uint32_t bloque_lif = (pagina_lif/256) + 1;
    pagina_lif %= 256;
    int inicio_lif = (bloque * 1024)%BYTES_PER_PAGE;
    int posicion = ((bloque_lif)*256 + pagina_lif) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque

    fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion para revisar el lifemap
    unsigned char* lif_page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    fread(lif_page, sizeof(char), BYTES_PER_PAGE, file_disco);
    for(int lp = inicio_lif; lp < inicio_lif+1024; lp+=4){
        int life = lif_page[lp] | (lif_page[lp+1]<<8) | (lif_page[lp+2]<<16) | (lif_page[lp+3]<<24);
        if(life < 0){
            free(lif_page);
            fseek(file_disco, ultima_pos, SEEK_SET);
            return 0;
        }
        
    }
    
    free(lif_page);
    fseek(file_disco, ultima_pos, SEEK_SET);
    return 1;
}

int aumentar_lifmap(FILE *file_disco, uint32_t bloque, unsigned int pagina, unsigned life_max){
    unsigned int ultima_pos = ftell(file_disco);
    int pagina_lif = (bloque * 1024)/BYTES_PER_PAGE;
    uint32_t bloque_lif = (pagina_lif/256) + 1;
    pagina_lif %= 256;
    int inicio_lif = (bloque * 1024)%BYTES_PER_PAGE;
    int posicion = ((bloque_lif)*256 + pagina_lif) * BYTES_PER_PAGE; // Asignamos posición donde se ubica el bloque
    fseek(file_disco, posicion, SEEK_SET);            // Establecemos la posicion para revisar el lifemap
    unsigned char* lif_page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    fread(lif_page, sizeof(char), BYTES_PER_PAGE, file_disco);
    inicio_lif += 4*pagina;
    long long int life = lif_page[inicio_lif] | (lif_page[inicio_lif+1]<<8) | (lif_page[inicio_lif+2]<<16) | (lif_page[inicio_lif+3]<<24);
    life ++;
    if(life >= life_max){
        printf("Mas que lifmax\n");
        for(int menos=0; menos<4; menos++){
            lif_page[inicio_lif++] = -1;
        }
        fseek(file_disco, posicion, SEEK_SET);
        fwrite(lif_page, sizeof(char), BYTES_PER_PAGE, file_disco);
        fseek(file_disco, ultima_pos, SEEK_SET);
        return 0;
    }

    lif_page[inicio_lif]   = life & (unsigned)0xff;
    lif_page[inicio_lif+1] = (life & ((unsigned)0xff00)) >> (8);
    lif_page[inicio_lif+2] = (life & ((unsigned)0xff0000)) >> (16);
    lif_page[inicio_lif+3] = (life & ((unsigned)0xff000000)) >> (24);
    fseek(file_disco,posicion , SEEK_SET);
    fwrite(lif_page, sizeof(char), BYTES_PER_PAGE, file_disco);
    free(lif_page);
    fseek(file_disco, ultima_pos, SEEK_SET);
    return 1;

}

void marcar_bloque_ocupado(FILE *file_disco, uint32_t dir_bloque){
    unsigned int ultima_pos = ftell(file_disco);
    unsigned char* page0 = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes para la pagina 0
    fseek(file_disco, 0, SEEK_SET);            // Establecemos la posicion en 0 para revisar el bitmap
    fread(page0, sizeof(char), BYTES_PER_PAGE, file_disco);      // Leemos la pagina

    uint32_t despues = page0[dir_bloque >> 3] | (1 << (7 - (dir_bloque & 7)));
    page0[dir_bloque >> 3] = despues;
    fseek(file_disco, 0, SEEK_SET);
    fwrite(page0, sizeof(char), BYTES_PER_PAGE, file_disco);

    free(page0);
    fseek(file_disco, ultima_pos, SEEK_SET);
}

char *obtener_nombre(char *path){
    char *string = calloc(500, sizeof(char));
    strcpy(string, path);
    char * token = strtok(string, "/");
    char *nombre = calloc(28, sizeof(char));
    while( token != NULL ) {
      strcpy(nombre,token);
      token = strtok(NULL, "/");
    }
    free(string);
    return nombre;
}

int len(char* string){
    int j = 0;
    while(string[j] != '\0'){
        j++;
    }
    return j;
};

void llenar_osfile(FILE *file, osFile *archivo, char *buscado) {
    uint32_t bloque = archivo->bloque_indice;
    for(int i=0; i< 256; i++){ //Recorrer las paginas del bloque
        int posicion = (bloque * PAGES_PER_BLOCK + i) * BYTES_PER_PAGE; // Asignamos posición inicial en el bloque
        unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
        fseek(file, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
        fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
        char nombre[28];
        for(int j = 0; j < BYTES_PER_PAGE; j+=BYTES_PER_DIRECTORY_ENTRY){
            unsigned int byte = page[j];
            unsigned char letra;
            if(byte == 3 || byte == 1){
                int cont = 5;
                while ((letra = page[j+cont]) != 0 && cont < 32){
                    nombre[cont-5] = letra;
                    cont++;
                };
                nombre[cont-5] = '\0';
                if(!strcmp(nombre, buscado)){
                    uint32_t dir_bloque = page[j+1]+(page[j+2]<<8)+(page[j+3]<<16)+(page[j+4]<<24);
                    archivo->bloque_indice = dir_bloque;
                    free(page);
                    return;
                }
                
            }
        }
        free(page);
    }
    return;
}

void crear_archivo(FILE *file, osFile *archivo, char *nombre){
    uint32_t bloque = archivo->bloque_indice;
    
    for(int i=0; i< 256; i++){ //Recorrer las paginas del bloque
        int posicion = (bloque * PAGES_PER_BLOCK + i) * BYTES_PER_PAGE; // Asignamos posición inicial en el bloque directorio
        unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
        fseek(file, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
        fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
        for(int j = 0; j < BYTES_PER_PAGE; j+=BYTES_PER_DIRECTORY_ENTRY){
            unsigned int byte = page[j];
            //unsigned char letra;
            if(byte == 0){
                uint32_t bloque_disponible = buscar_bloque_disponible(file);
                archivo->bloque_indice = bloque_disponible;
                marcar_bloque_ocupado(file, bloque_disponible);
                unsigned int bit_valides = 3; 
                unsigned int dir1 = (unsigned)bloque_disponible & 0xff;
                unsigned int dir2 = ((unsigned)bloque_disponible & 0xff00) >> 8;
                unsigned int dir3 = ((unsigned)bloque_disponible & 0xff0000) >> 16;
                unsigned int dir4 = ((unsigned)bloque_disponible & 0xff000000) >> 24;

                page[j++] = bit_valides; //Registrar existencia de un archivo 
                page[j++] = dir1; // Registrar puntero del bloque
                page[j++] = dir2;
                page[j++] = dir3;
                page[j++] = dir4;
                for(int pos=0; pos<len(nombre); pos++){
                    page[j++] = nombre[pos]; // Registrar nombre del bloque
                }
                fseek(file, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
                fwrite(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
                free(page);
                return;
            }
        }
        free(page);
    }
    printf("No queda entradas disponibles\n");
    return;
};

void llenar_tamano(FILE *file, osFile *archivo, char *buscado){
    uint32_t bloque = archivo->bloque_indice;
    int posicion = (bloque * PAGES_PER_BLOCK) * BYTES_PER_PAGE; // Asignamos posición inicial en el bloque
    unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    fseek(file, posicion, SEEK_SET);            // Establecemos la posicion donde comienza la lectura 
    fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
    archivo->tamano = 0;
    for(int i= 0; i< 8; i++){
        archivo->tamano += (page[i]<<(i*8));
    }
}

int agregar_puntero(FILE *file, uint32_t bloque_indice, uint32_t bloque_nuevo, size_t tamano){
    unsigned int ultima_pos = ftell(file);
    int posicion = (bloque_indice * PAGES_PER_BLOCK) * BYTES_PER_PAGE; // Asignamos posición inicial en el bloque
    unsigned char* page = malloc(sizeof(char) * BYTES_PER_PAGE);   // 4096 Bytes 
    fseek(file, posicion, SEEK_SET);
    fread(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina
    int cant_punteros = tamano/(BYTES_PER_PAGE * PAGES_PER_BLOCK);
    if(cant_punteros == 1012){
        printf("Se ha alcanzado la maxima capacidad\n");
        return 0;
    }
    int pos_nuevo_puntero = 8 + cant_punteros*4;

    unsigned int dir1 = (unsigned)bloque_nuevo & 0xff;
    unsigned int dir2 = ((unsigned)bloque_nuevo & 0xff00) >> 8;
    unsigned int dir3 = ((unsigned)bloque_nuevo & 0xff0000) >> 16;
    unsigned int dir4 = ((unsigned)bloque_nuevo & 0xff000000) >> 24;

    page[pos_nuevo_puntero++] = dir1;
    page[pos_nuevo_puntero++] = dir2;
    page[pos_nuevo_puntero++] = dir3;
    page[pos_nuevo_puntero++] = dir4;
    fseek(file, posicion, SEEK_SET);
    fwrite(page, sizeof(char), BYTES_PER_PAGE, file);      // Leemos la pagina

    fseek(file, ultima_pos, SEEK_SET);
    free(page);
}