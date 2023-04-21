# P2-SSOO

## Integrantes 
- Francisco Balmaceda - 18623948
- Josefa Parra - 19643098
- Florencia Pesenti - 16640268
- Victoria Sanchez - 18623913
- Maria Paz Subiabre - 16640535 

## Instrucciones de Ejecución
Para ejecutar el proyecto se debe inicializar en la carpeta server el comando make para compilar el código y luego ejecutar la siguiente línea:
./server -i <ip_address> -p <tcp_port>

Paralelamente, para inicializar a un cliente, se debe ejecutar en otra consola el comando make y luego ejecutar la siguiente línea:
./client -i <ip_address> -p <tcp_port>

Actualmente se utiliza en el código el address: 0.0.0.0 y port: 8080, por lo que habría que escribir dicha dirección y puesto en las líneas anteriores. Cabe destacar que existen dos archivos independientes makefile, uno para el servidor y otro para el cliente.

Luego de tener inicializados a el servido y a los clientes, en consola se debe ingresar los nombres de los jugadores (por parte de los clientes) y se asignan los distintos aldeanos a su correspondiente rol. Cuando se encuentra todo listo el líder (cliente que se conecte primero) puede iniciar el juego presionando enter.

Cada jugador en su respectivo turno tendrá acceso a un menú en el cual se encontrarán las opciones de Mostrar información, Crear Aldeano, Subir de Nivel, Atacar, Espiar, Robar, Pasar y Rendirse. Cada opción se despegará en la consola acompañada de un numero el cual el jugador deberá apretar según la opción deseada, ciertas opciones como Crear Aldeano, Subir Nivel, Espiar y Robar, se despliega un sub menú en donde deben precisar sus decisiones respectivas.

## Paquetes
El tipo de paquete utilizado es de tipo string para todos los casos.

## Decisiones de diseño
En primer lugar, el servidor es quien realiza toda la lógica del juego, mientras que el cliente es quien escoge que decisiones tomar. Se dispone de dos carpetas independientes, client y server, cada una con . A continuación, se explicara cada una de ellas.

### Server
Como se mencionó anteriormente, el servidor es quien se encarga de la lógica del juego, por lo que se incluye el archivo jugador.c en donde se construye la lógica de cada una de las opciones las cuales podría escoger el cliente, junto con los archivos comunication.c y conection.c en donde se maneja las conexiones con el cliente.
Por último, se encuentra el archivo main.c en donde se reciben las opciones escogidas por el cliente, se realiza la lógica según sea el msg code recibido, y se envía la respuesta correspondiente al cliente.

Los msg code  a recibir con su respectiva acción son los siguientes:
- msg_code: 10 -> Mostrar información 
- msg_code: 11 -> Crear Aldeano
- msg_code: 13 -> Subir Nivel 
- msg_code: 14 -> Atacar
- msg_code: 15 -> Espiar
- msg_code: 16 -> Robar
- msg_code: 17 -> Pasar
- msg_code: 18 -> Rendirse

### Cliente
El cliente tiene la única labor de escoger la opción que desea que se ejecute y desplegar la información al jugador. Los archivos que contiene son: function.c en donde se realiza el menú de opciones, es aqui en donde se envía un msg code, según la opción escogida, al servidor, también se encuentran los archivos comunication.c y conection.c en donde se maneja las conexiones con el servidor. Finalmente se encuentra el archivo main.c en donde se recibe los msg code del servidor y, según el msg code recibido, se despliega en consola al cliente la respuesta correspondiente.
Los msg code recibidos con su respectiva asociación son los siguientes:

- msg_code: 1 -> Solicitar nombre
- msg_code: 2 -> Asignar Aldeano
- msg_code: 3 -> Empezar el juego
- msg_code: 4 -> imprimir mensaje servidor
- msg_code: 5 -> comenzar el juego
- msg_code: 11 -> llegan
    - Crear aldeano
    - Subir nivel
    - Atacar
- msg_code: 15 -> Espiar
- msg_code: 16 -> Robar
- msg_code: 19 -> Recoger recursos
- msg_code: 10 -> mostrar informacion

## Principales Funciones del programa
- se conectan los clientes e inicializan los jugadores correctamente
- funciona correctamente cada una de las opciones del menú

## Supuestos
- El recolectar recursos se muestra en todos los turnos menos en el primero que se parte con 100 de cada recurso
- Cuando el jugador necesita el id de otro jugador en atacar/espiar/robar el jugador sabe que va de 0 a n-1, siendo n la cantidad de jugadores.

## FUNCIONAMIENTO (Para facilitar corrección)
- Rendirse solo funciona con dos clientes
- Cunado un jugador gana no se acaba el flujo del juego, solo avisa al ganador que ganó

