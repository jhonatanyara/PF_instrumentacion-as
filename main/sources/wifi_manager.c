#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "esp_sntp.h"
#include "driver/gpio.h"
#include "ina226.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>

static const char *TAG = "WIFI_MANAGER";

#define ESP_AP_SSID           CONFIG_ESP_WIFI_AP_SSID
#define ESP_AP_PASS           CONFIG_ESP_WIFI_AP_PASSWORD
#define ESP_AP_MAX_CONN       CONFIG_ESP_MAX_STA_CONN_AP
#define ESP_AP_CHANNEL        CONFIG_ESP_WIFI_AP_CHANNEL

#define ESP_STA_SSID          CONFIG_ESP_WIFI_REMOTE_AP_SSID
#define ESP_STA_PASS          CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD
#define ESP_MAXIMUM_RETRY     CONFIG_ESP_MAXIMUM_STA_RETRY

#define GPIO_STATUS_1         GPIO_NUM_32
#define GPIO_STATUS_2         GPIO_NUM_33
#define GPIO_INPUT_PIN_SEL    ((1ULL<<GPIO_STATUS_1) | (1ULL<<GPIO_STATUS_2))

static int s_retry_num = 0;
static bool s_sntp_initialized = false;
static httpd_handle_t server_handle = NULL;

// Forward declarations
static void initialize_sntp(void);

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Reintentando conectar a la red WiFi...");
        } else {
            ESP_LOGE(TAG, "Fallo al conectar a la red WiFi después de %d intentos", ESP_MAXIMUM_RETRY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "¡Conectado! Dirección IP adquirida en la interfaz STA.");
        s_retry_num = 0;
        if (!s_sntp_initialized) {
            initialize_sntp();
            s_sntp_initialized = true;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(TAG, "Un dispositivo se ha conectado al AP.");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        ESP_LOGI(TAG, "Un dispositivo se ha desconectado del AP.");
    }
}

static void initialize_sntp(void) {
    ESP_LOGI(TAG, "Inicializando SNTP para sincronizar hora...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

static bool __attribute__((unused)) is_time_synced(void) {
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    return (timeinfo.tm_year > (2016 - 1900));
}

static const char *get_current_time_string(char *buffer, size_t len) {
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
        snprintf(buffer, len, "Sin hora");
    } else {
        strftime(buffer, len, "%Y-%m-%d %H:%M:%S", &timeinfo);
    }
    return buffer;
}

static void initialize_gpio_inputs(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = GPIO_INPUT_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "GPIO de estado inicializados: GPIO %d, GPIO %d", GPIO_STATUS_1, GPIO_STATUS_2);
}

void wifi_init_softap_sta(void) {
    ESP_LOGI(TAG, "Inicializando WiFi en modo AP + STA...");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    initialize_gpio_inputs();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = ESP_AP_SSID,
            .ssid_len = strlen(ESP_AP_SSID),
            .channel = ESP_AP_CHANNEL,// Configura el canal del AP con 
            .password = ESP_AP_PASS,
            .max_connection = ESP_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };
    if (strlen(ESP_AP_PASS) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = ESP_STA_SSID,
            .password = ESP_STA_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Inicialización WiFi completada.");
}

static esp_err_t api_status_get_handler(httpd_req_t *req) {
    float voltage = ina226_get_bus_voltage();
    float current = ina226_get_current();
    float power = ina226_get_power();
    double energy_wh = ina226_get_energy_wh();
    int gpio1 = gpio_get_level(GPIO_STATUS_1);
    int gpio2 = gpio_get_level(GPIO_STATUS_2);
    char time_string[32] = {0};
    get_current_time_string(time_string, sizeof(time_string));

    char json_response[320];
    int len = snprintf(json_response, sizeof(json_response),
                       "{\"time\":\"%s\",\"voltage\":%.3f,\"current\":%.3f,\"power\":%.3f,\"energy_wh\":%.4f,\"gpio_1\":%d,\"gpio_2\":%d}",
                       time_string, voltage, current, power, energy_wh, gpio1, gpio2);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json_response, len);
}

static esp_err_t favicon_get_handler(httpd_req_t *req) {
    httpd_resp_set_status(req, "204 No Content");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    const char *html =
        "<!DOCTYPE html><html lang=\"es\"><head><meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>Instrumentación ESP32</title>"
        "<style>body{font-family:Arial,sans-serif;padding:16px;background:#f4f4f4;color:#202124;}"
        "h1{color:#0f4173;} .card{background:#fff;border-radius:10px;padding:18px;margin:12px 0;box-shadow:0 4px 10px rgba(0,0,0,0.08);}"
        "dl{display:grid;grid-template-columns:1fr 1fr;gap:10px;}dt{font-weight:700;}dd{margin:0;color:#333;}"
        "</style></head><body>"
        "<h1>Dashboard de Consumo DC</h1>"
        "<div class=\"card\"><dl>"
        "<dt>Hora</dt><dd id=\"time\">-</dd>"
        "<dt>Voltaje</dt><dd id=\"voltage\">-</dd>"
        "<dt>Corriente</dt><dd id=\"current\">-</dd>"
        "<dt>Potencia</dt><dd id=\"power\">-</dd>"
        "<dt>Energía</dt><dd id=\"energy\">-</dd>"
        "<dt>GPIO 32</dt><dd id=\"gpio1\">-</dd>"
        "<dt>GPIO 33</dt><dd id=\"gpio2\">-</dd>"
        "</dl></div>"
        "<p>El ESP32 actúa como SoftAP + STA y ofrece actualizaciones en tiempo real para medir voltaje, corriente, potencia, energía y estado de GPIO.</p>"
        "<script>async function refresh() {"
        "  try {"
        "    const res = await fetch('/api/status');"
        "    if (!res.ok) return;"
        "    const data = await res.json();"
        "    document.getElementById('time').innerText = data.time;"
        "    document.getElementById('voltage').innerText = data.voltage.toFixed(2) + ' V';"
        "    document.getElementById('current').innerText = data.current.toFixed(3) + ' A';"
        "    document.getElementById('power').innerText = data.power.toFixed(3) + ' W';"
        "    document.getElementById('energy').innerText = data.energy_wh.toFixed(4) + ' Wh';"
        "    document.getElementById('gpio1').innerText = data.gpio_1 ? 'HIGH' : 'LOW';"
        "    document.getElementById('gpio2').innerText = data.gpio_2 ? 'HIGH' : 'LOW';"
        "  } catch (e) { console.error(e); }"
        "}"
        "setInterval(refresh, 2000); refresh();</script></body></html>";

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

static const httpd_uri_t uri_root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t uri_favicon = {
    .uri = "/favicon.ico",
    .method = HTTP_GET,
    .handler = favicon_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t uri_api_status = {
    .uri = "/api/status",
    .method = HTTP_GET,
    .handler = api_status_get_handler,
    .user_ctx = NULL,
};

void start_webserver(void) {
    ESP_LOGI(TAG, "Iniciando servidor web HTTP...");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&server_handle, &config) == ESP_OK) {
        httpd_register_uri_handler(server_handle, &uri_root);
        httpd_register_uri_handler(server_handle, &uri_api_status);
        httpd_register_uri_handler(server_handle, &uri_favicon);
        ESP_LOGI(TAG, "Servidor web HTTP iniciado. Entra a la IP del ESP32 en tu navegador.");
    } else {
        ESP_LOGE(TAG, "Error iniciando el servidor web HTTP.");
    }
}