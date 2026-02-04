# Prueba técnica – GPS proximity alert (C / UART / NMEA)

## Objetivo
Desarrollar un programa en C que:
- Obtenga la posición GPS desde un módulo compatible con **Adafruit Ultimate GPS (ID 790)**.
- Procese tramas **NMEA** recibidas por **UART**, puerto serie COM5 y COM6.
- Avise cuando la posición se encuentre dentro de un **radio de 100 metros** respecto a una posición fija.

Dado que no se dispone del hardware físico durante el desarrollo, se ha implementado un **entorno de simulación realista** que reproduce el comportamiento del módulo GPS.


---

## Enfoque general

El sistema se ha dividido en tres partes claramente separadas:

1. **Simulador de GPS (sender)**
   - Simula el comportamiento del módulo Adafruit GPS.
   - Genera tramas NMEA válidas (`GPRMC`).
   - Modifica progresivamente la latitud y longitud para simular movimiento.
   - Envía una trama cada **200 ms** por UART.

2. **Receptor (receiver)**
   - Simula el comportamiento de una Raspberry Pi conectada al GPS.
   - Recibe tramas NMEA por UART.
   - Parseo de datos mediante la librería **libnmea**.
   - Conversión de coordenadas a formato decimal.
   - Cálculo de distancia a un punto fijo.
   - Aviso al entrar en un radio de **100 m**.

3. **Librería de parseo**
   - Se utiliza la librería open-source **libnmea**:
     https://github.com/jacketizer/libnmea
   - Elegida por su simplicidad, portabilidad y uso extendido.

---


## Simulación del GPS

### Coordenadas utilizadas
- **Inicio (Plaza Mayor, Madrid)**  
  `40.415392, -3.707374`
- **Destino (Puerta del Sol, Madrid)**  
  `40.416729, -3.703339`

El simulador:
- Comienza en Plaza Mayor.
- Incrementa latitud y longitud mediante un **paso fijo** cada 200 ms.
- Aproxima una velocidad compatible con un dron urbano.
- Genera tramas `$GPRMC` con **checksum correcto**.

Este enfoque permite probar toda la lógica sin depender de satélites ni hardware físico. 

Se ha utilizado ChatGpt para generar más rápido el Sender.exe.

---

## EJECUCION

Ejecutar en carpeta ./build/receiver.exe y en carpeta src/sender.exe

---

## Problemas y consultas

Durante el desarrollo se han encontrado varios problemas, listados de menos complejos a más complejos:

    - Consulta del cableado RaspBerry - AdaFruit:
        para esto se ha buscado la documentación eléctrica de ambos dispositivos para determinar el pinout de ambos y cablearlo correctamente.
    - Definicion de coms con Puertos serie:
        para esto se ha buscado documentacion en foros y chatgpt.
    - Manejo de cadenas:
        el manejo de cadenas en C es diferente al de Java (lenguaje que mejor manejo). Para solventarlo se ha consultado a ChatGpt y documentacion de C.
    - Aprendizaje de posicionamiento GPS: 
        No contaba con conocimientos de como calcular la posicion de un objeto con respecto a las coordenadas que se envían NMEA por lo que se consultaron fórmulas con distintas IAs. Todas las fórmulas son sugeridas.
    - Manejo de punteros y memorias:
        como se menciona, C es un lenguaje que no manejo y he tenido que consultar varios videos para comprender como usar structs, punteros, etc. Aún así queda mucho por aprender acerca de esto.
    - Inclusión de librerias:
        lo que me ha sido mas dificultoso ha sido incluir la libreria en el proyecto. Hacía mucho tiempo que no incluía una libreria a mano, normalmente, al utilizar gestores de proyectos como Maven en Java, las dependencias se incluyen direntamente desde un xml. Muchos problemas con los archivos MakeFiles, los cuales también se consultaron con distintas IAs.

## PROCESO

    1.- Se busca información de los dispositivos a manejar.
    2.- Se usa rapsberry no por nada en especial, si no porque se ha usado en otros proyectos para pruebas. 
    3.- Se conuslta documentación eléctrica de ambos y se dibuja documentación.
    4.- Se decido cómo hacer un sistema cerrado y funcional para poder probar lo que se codifica. 
    5.- Se llega a la conclusión de utilizar las prácticas seguidas en otras ocasiones(en el caso de comunicaciones TCP/IP ): tener un emisor receptor.
    6.- Se consulta documentación sobre C y uso de puertos Serie.
    7.- Se codifica un Receiver y un Sender básico, los cuales imprimen la cadena que se recibe y se envía respectivamente. Aunque la aplicación está pensada para Linux se codifica para la Api de windows, ya que, va a 
    ser mucho más rápido. Se pensó que, de haber, tiempo se montaría un docker con linux y recodificar todo a linux.
    8.- Se procede a usar la libreria libnmea, se consulta documentación y se trata de incluir en el proyecto.
    9.- Una vez incluida(y sin errores de compilación) se procede a añadir el parse de la trama fija que se envía.
    10.- Se procede a codificar con la IA el sender para ir más rápido, ya que, no es el objeto principal de la prueba.
    11.- Una vez que se observa que se reciben tramas con valores variables en la geoposición se procede a parsear en el receirver y a desarrollar la comprobación de geoposición: fórmulas de cálculo de posición, tratamiento de cadenas.
    12.- Pruebas finales.

## Proyección de horas

    - En total se ha tardado unas 8 horas:
        - 1 de documentación eléctrica 
        - 7 de codificación
    - Las horas estimadas al leer el enunciado de la prueba fueron bastantes más, ya que, son tecnologías **que no manejo**, pero con ayuda de la IA y tutoriales se ha avanzado más rápido.



