#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <nmea.h>
#include <nmea/gprmc.h>

// Configuración del puerto serie
static const char *PORT = "\\\\.\\COM6";
static const DWORD BAUD = CBR_9600;

// Posicion de referencia
static const double REF_LAT = 40.416729;
static const double REF_LON = -3.703339;

// Radio en metros de la zona objetivo
static const double RADIUS_M = 100.0;

// Manejo de errores de la api Windows
static void print_win_err(const char *msg) {
    DWORD e = GetLastError();
    fprintf(stderr, "%s (GetLastError=%lu)\n", msg, (unsigned long)e);
}

// Convierte nmea_position a decimal y si es S/W lo hace negativo
static double pos_to_decimal(const nmea_position *p) {
    double dec = (double)p->degrees + (p->minutes / 60.0);
    if (p->cardinal == 'S' || p->cardinal == 'W') dec = -dec;
    return dec;
}

// Convierte grados a radianes
static double deg2rad(double d) { return d * (3.141592653589793 / 180.0); }

// Haversine entre dos puntos (lat1,lon1) y (lat2,lon2) en metros
static double haversine_m(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0;
    double dlat = deg2rad(lat2 - lat1);
    double dlon = deg2rad(lon2 - lon1);

    double a =
        pow(sin(dlat / 2),2) +
        cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
        pow(sin(dlon / 2), 2);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return R * c;
}

int main(void) {
    printf("Receiver libnmea: abriendo %s...\n", PORT);

    // Estructura para el puerto serie
    HANDLE h = CreateFileA(
        PORT,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    // Manejo de errores al abrir el puerto
    if (h == INVALID_HANDLE_VALUE) {
        print_win_err("ERROR: no puedo abrir el puerto");
        printf("Pulsa ENTER para salir...\n");
        getchar();
        return 1;
    }

    // Parametros del puerto serie
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    
    dcb.BaudRate = BAUD;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    // Manejo de errores al obtener estado
    if (!GetCommState(h, &dcb)) {
        print_win_err("ERROR: GetCommState");
        CloseHandle(h);
        return 1;
    }
    // Manejo de errores al establecer estado
    if (!SetCommState(h, &dcb)) {
        print_win_err("ERROR: SetCommState");
        CloseHandle(h);
        return 1;
    }

    // Timeouts (evita bloqueos eternos)
    COMMTIMEOUTS t = {0};
    t.ReadIntervalTimeout = 50;
    t.ReadTotalTimeoutConstant = 50;
    t.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(h, &t);

    printf("OK. Escuchando (9600 8N1).\n");
    printf("Referencia: lat=%.6f lon=%.6f | Radio: %.1f m\n\n", REF_LAT, REF_LON, RADIUS_M);

    char line[1024];
    int idx = 0;

    char buf[256];

    int done = 0;  // <- cuando sea 1, salimos


    //Bucle de recepcion y procesamiento de la trama recibida
    while (!done) {
        DWORD n = 0;

        // Leer datos del puerto serie
        // Si no se pueden leer, mostrar error y salir
        if (!ReadFile(h, buf, (DWORD)sizeof(buf), &n, NULL)) {
            print_win_err("ERROR: ReadFile");
            break;
        }
        if (n == 0) continue;

        // Procesar los datos recibidos byte a byte
        for (DWORD i = 0; i < n && !done; i++) {
            char ch = buf[i];

            if (idx < (int)sizeof(line) - 1) {
                line[idx++] = ch;
            } else {
                // overflow: resetea
                idx = 0;
                continue;
            }

            // Fin de linea
            if (ch == '\n') {
                // terminar cadena
                line[idx] = '\0';

                // dejar linea mutable para nmea_parse
                char mutable_line[1024];
                // Creo copia mutable de la linea
                strncpy(mutable_line, line, sizeof(mutable_line) - 1);
                // definir el ultimo byte como \0 por si acaso
                mutable_line[sizeof(mutable_line) - 1] = '\0';
                
                // parsear trama NMEA 
                nmea_s *msg = nmea_parse(mutable_line, strlen(mutable_line), 1);


                // Si es GPRMC, procesar
                if (msg && msg->type == NMEA_GPRMC) {
                    // Cast a estructura GPRMC
                    nmea_gprmc_s *rmc = (nmea_gprmc_s *)msg;

                    // la estructura nmea_gprmc_s tiene un campo 'valid' que indica si los datos son válidos
                    if (rmc->valid) {

                        // Obtener lat/lon en decimal
                        double lat = pos_to_decimal(&rmc->latitude);
                        double lon = pos_to_decimal(&rmc->longitude);
                        
                        // haversine da la distancia al punto de referencia
                        double d = haversine_m(lat, lon, REF_LAT, REF_LON);
                        // Mostrar lat, lon y distancia
                        printf("lat=%.6f lon=%.6f  dist=%.1f m\n", lat, lon, d);

                        // si estamos dentro del radio, avisar y salir
                        if (d <= RADIUS_M) {
                            printf("\n>>> DENTRO DEL RADIO (<= %.1f m). PARANDO RECEPCION.\n", RADIUS_M);
                            Beep(1200, 200);
                            Beep(1200, 200);
                            done = 1;   
                        }
                    }
                }

                // Liberar memoria del mensaje parseado
                if (msg) nmea_free(msg);
                // reset indice
                idx = 0;
            }
        }
    }

    // Cerrar puerto serie
    CloseHandle(h);
    printf("Puerto cerrado. Fin.\n");
    return 0;
}
