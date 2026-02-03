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
        lo que me ha sido mas dificultoso ha sido incluir la libreria en el proyecto. Hacía mucho tiempo que no incluía una libreria a mano, normalmente, al utilizar gestores de proyectos como Maven en Java, las dependencias se incluyen direntamente desde un xml. 




