#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"

// Declaración de las funciones para que otros archivos puedan usarlas
void start_webserver(void);
void wifi_init_softap_sta(void); // Asumiendo que tienes una función para iniciar el WiFi

#endif // WIFI_MANAGER_H