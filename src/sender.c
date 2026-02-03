#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Configuración del puerto serie
static const char *PORT = "\\\\.\\COM5";
static const int PERIOD_MS = 200;

// Definimos punto de inicio y destino
static const double START_LAT  = 40.415392;
static const double START_LON  = -3.707374;
static const double TARGET_LAT = 40.416729;
static const double TARGET_LON = -3.703339;

// Paso fijo por tick (200 ms). Ajusta si quieres más lento/rápido.
// 1e-5 ~ 1.1m (lat). 1.5e-5 ~ 1.7m por tick ~ 8.5m/s aprox.
static const double STEP_DEG = 0.000015;

// Valor absoluto
static double dabs(double x) { return (x < 0) ? -x : x; }

// decimal -> NMEA ddmm.mmmm (lat) + hemisferio
static void to_nmea_lat(double lat, char *out, char *hemi) {
    *hemi = (lat >= 0) ? 'N' : 'S';
    double a = dabs(lat);
    int deg = (int)a;
    double minutes = (a - (double)deg) * 60.0;

    int mm = (int)minutes;
    double frac = (minutes - (double)mm) * 10000.0;
    int f = (int)(frac + 0.5); // redondeo

    if (f >= 10000) { f -= 10000; mm += 1; }
    if (mm >= 60) { mm -= 60; deg += 1; }

    sprintf(out, "%02d%02d.%04d", deg, mm, f);
}

// decimal -> NMEA dddmm.mmmm (lon) + hemisferio
static void to_nmea_lon(double lon, char *out, char *hemi) {
    *hemi = (lon >= 0) ? 'E' : 'W';
    double a = dabs(lon);
    int deg = (int)a;
    double minutes = (a - (double)deg) * 60.0;

    int mm = (int)minutes;
    double frac = (minutes - (double)mm) * 10000.0;
    int f = (int)(frac + 0.5);

    if (f >= 10000) { f -= 10000; mm += 1; }
    if (mm >= 60) { mm -= 60; deg += 1; }

    sprintf(out, "%03d%02d.%04d", deg, mm, f);
}

// XOR del cuerpo (sin '$' ni '*')
static uint8_t nmea_checksum(const char *s) {
    uint8_t cs = 0;
    while (*s) cs ^= (uint8_t)(*s++);
    return cs;
}

int main(void) {
    HANDLE hSerial = CreateFileA(PORT, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error abriendo %s\n", PORT);
        return 1;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hSerial, &dcb);
    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hSerial, &dcb);

    double lat = START_LAT;
    double lon = START_LON;

    printf("Sender simple en %s | tick %dms | step %.6f deg\n", PORT, PERIOD_MS, STEP_DEG);

    while (1) {
        // Mover lat hacia TARGET_LAT
        if (lat < TARGET_LAT) lat += STEP_DEG;
        else if (lat > TARGET_LAT) lat -= STEP_DEG;

        // Mover lon hacia TARGET_LON
        if (lon < TARGET_LON) lon += STEP_DEG;
        else if (lon > TARGET_LON) lon -= STEP_DEG;


        // A NMEA
        char lat_nmea[16], lon_nmea[16];
        char lat_hemi, lon_hemi;
        to_nmea_lat(lat, lat_nmea, &lat_hemi);
        to_nmea_lon(lon, lon_nmea, &lon_hemi);

        // RMC (campos fijos, solo cambia lat/lon)
        char body[200];
        sprintf(body, "GPRMC,123519,A,%s,%c,%s,%c,0.0,0.0,230394,,,A",
                lat_nmea, lat_hemi, lon_nmea, lon_hemi);

        uint8_t cs = nmea_checksum(body);

        char sentence[240];
        sprintf(sentence, "$%s*%02X\r\n", body, cs);

        DWORD written;
        WriteFile(hSerial, sentence, (DWORD)strlen(sentence), &written, NULL);

        printf("%s", sentence);

        Sleep(PERIOD_MS);
    }

    CloseHandle(hSerial);
    return 0;
}
