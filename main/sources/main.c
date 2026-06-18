#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "sdkconfig.h"

// Librerías de periféricos (¡Nuevas APIs!)
#include "driver/i2c_master.h"
#include "driver/gpio.h"

// Nuestras librerías personalizadas
#include "wifi_manager.h"
#include "lcd.h"
#include "ina226.h"


static const char *TAG = "MAIN";

// el esp rx y tx van conectados a los pines GPIO1 (TX) y GPIO3 (RX) respectivamente, 
//pero no los usaremos para nada en este proyecto, así que no los configuramos como UART, 
//sino que los dejamos libres para otras funciones si es necesario.
// los sda y scl del bus I2C los configuramos en el menuconfig, típicamente GPIO21 para SDA 
// y GPIO22 para SCL, pero puedes elegir otros pines si lo deseas.
// Definiciones de pines para el bus I2C

//ip del esp:
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA_IO
#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL_IO

// Variable global para manejar el Bus I2C
i2c_master_bus_handle_t bus_handle;

// Inicialización del Bus I2C
static esp_err_t i2c_master_init(void) {
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1, // El sistema elige un puerto libre automáticamente
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_LOGI(TAG, "Inicializando Bus I2C Maestro...");
    return i2c_new_master_bus(&i2c_bus_config, &bus_handle);
}

static void display_task(void *arg) {
    int64_t last_time_us = esp_timer_get_time();
    const TickType_t delay = pdMS_TO_TICKS(2000);
    char lcd_text[64];

    ESP_LOGI(TAG, "[DISPLAY_TASK] Iniciado");
    
    while (1) {
        ESP_LOGI(TAG, "[DISPLAY_TASK] Leyendo sensores...");
        float voltage = ina226_get_bus_voltage();
        float current = ina226_get_current();
        float power = ina226_get_power();
        int64_t now_us = esp_timer_get_time();
        float delta_seconds = (now_us - last_time_us) / 1000000.0f;
        last_time_us = now_us;

        ina226_update_energy(power, delta_seconds);
        double energy_wh = ina226_get_energy_wh();

        char line1[17];
        char line2[17];

        snprintf(line1, sizeof(line1), "V=%4.2f I=%4.2f", voltage, current);
        snprintf(line2, sizeof(line2), "P=%4.2f E=%4.0f", power, energy_wh);

        snprintf(lcd_text, sizeof(lcd_text), "%s\n%s", line1, line2);
        lcd_print(lcd_text);

        vTaskDelay(delay);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=== Iniciando Sistema ===");

    // 1. Inicializar NVS (Requisito obligatorio para usar WiFi en el ESP32)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Inicializar el Bus I2C
    ESP_ERROR_CHECK(i2c_master_init());

    // 3. Inicializar la Pantalla LCD
    ESP_ERROR_CHECK(lcd_init(bus_handle));

    // 4. Inicializar el sensor INA226 (no detener el arranque si no está presente)
    ret = ina226_init(bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "INA226 no disponible o error de I2C: %s. Continuando sin mediciones de energía.", esp_err_to_name(ret));
    }

    // 5. Inicializar WiFi y Servidor Web
    wifi_init_softap_sta();
    start_webserver();

    // 6. Iniciar la tarea de actualización de display y datos
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}