#include "lcd.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "LCD_DRIVER";
static i2c_master_dev_handle_t lcd_handle;
static uint8_t s_lcd_address = 0;

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
#define LCD_RW        0x02
#define LCD_RS        0x01

static esp_err_t lcd_write_raw(uint8_t data) {
    return i2c_master_transmit(lcd_handle, &data, 1, 1000);
}

static esp_err_t lcd_pulse_enable(uint8_t data) {
    esp_err_t err = lcd_write_raw(data | LCD_ENABLE); // Pone ENABLE en Alto
    if (err != ESP_OK) return err;
    
    // Aumentado a 500us para garantizar que el LCD lea el pulso
    esp_rom_delay_us(500); 
    
    err = lcd_write_raw(data & ~LCD_ENABLE); // Pone ENABLE en Bajo
    if (err != ESP_OK) return err;
    
    // Aumentado a 500us para darle tiempo a procesar la instrucción
    esp_rom_delay_us(500); 
    return ESP_OK;
}

static esp_err_t lcd_write_nibble(uint8_t nibble, bool rs) {
    uint8_t data = (nibble << 4) | LCD_BACKLIGHT | (rs ? LCD_RS : 0);
    esp_err_t err = lcd_write_raw(data);
    if (err != ESP_OK) return err;
    return lcd_pulse_enable(data);
}

static esp_err_t lcd_send_byte(uint8_t value, bool rs) {
    esp_err_t err = lcd_write_nibble(value >> 4, rs);
    if (err != ESP_OK) return err;
    return lcd_write_nibble(value & 0x0F, rs);
}

static esp_err_t lcd_command(uint8_t command) {
    return lcd_send_byte(command, false);
}

static esp_err_t lcd_data(uint8_t data) {
    return lcd_send_byte(data, true);
}

static esp_err_t lcd_probe_address(i2c_master_bus_handle_t bus_handle, uint8_t address, i2c_master_dev_handle_t *handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
        .flags.disable_ack_check = 0,
    };
    return i2c_master_bus_add_device(bus_handle, &dev_cfg, handle);
}

esp_err_t lcd_init(i2c_master_bus_handle_t bus_handle) {
    ESP_LOGI(TAG, "Configurando la pantalla LCD en el bus I2C...");

    // Se asume 0x27 por defecto si la macro falla, seguido de 0x3F
    const uint8_t addresses[] = {0x27, 0x3F}; 
    size_t found = 0;

    for (size_t i = 0; i < sizeof(addresses); ++i) {
        esp_err_t err = lcd_probe_address(bus_handle, addresses[i], &lcd_handle);
        if (err == ESP_OK) {
            s_lcd_address = addresses[i];
            found = 1;
            break;
        }
    }

    if (!found) {
        ESP_LOGE(TAG, "Error al conectar la pantalla LCD. No se encontró dirección 0x27 o 0x3F.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Pantalla LCD detectada en dirección 0x%02X", s_lcd_address);

    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Secuencia oficial de inicialización para modo de 4 bits
    lcd_write_nibble(0x03, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_write_nibble(0x03, false);
    vTaskDelay(pdMS_TO_TICKS(2));
    lcd_write_nibble(0x03, false);
    vTaskDelay(pdMS_TO_TICKS(2));
    lcd_write_nibble(0x02, false); // Cambio definitivo a 4 bits
    vTaskDelay(pdMS_TO_TICKS(2));

    lcd_command(0x28); // 4 bits, 2 líneas, fuente 5x8
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_command(0x08); // Apagar display
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_command(0x01); // Limpiar display
    vTaskDelay(pdMS_TO_TICKS(10)); // Limpiar toma más tiempo
    lcd_command(0x06); // Incremento de cursor automático
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_command(0x0C); // Encender display, apagar cursor
    vTaskDelay(pdMS_TO_TICKS(5));

    ESP_LOGI(TAG, "Pantalla LCD inicializada correctamente.");
    return ESP_OK;
}

static esp_err_t lcd_set_cursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offsets[] = {0x00, 0x40};
    if (row > 1) {
        return ESP_ERR_INVALID_ARG;
    }
    return lcd_command(0x80 | (col + row_offsets[row]));
}

void lcd_clear(void) {
    lcd_command(0x01);
    // IMPORTANTE: Limpiar el display requiere al menos 2ms reales. Subimos a 5ms.
    vTaskDelay(pdMS_TO_TICKS(5)); 
}

void lcd_print(const char *text) {
    lcd_clear();
    vTaskDelay(pdMS_TO_TICKS(5)); // Pausa de seguridad antes de inyectar datos

    uint8_t row = 0;
    uint8_t col = 0;

    lcd_set_cursor(0, 0);

    for (const char *c = text; *c != '\0' && row < 2; ++c) {
        if (*c == '\n') {
            row++;
            if (row >= 2) break;
            col = 0;
            lcd_set_cursor(0, row);
            continue;
        }

        if (col >= 16) {
            row++;
            if (row >= 2) break;
            col = 0;
            lcd_set_cursor(0, row);
        }

        char ch = *c;
        // Si no es un caracter ASCII imprimible, se reemplaza por espacio
        if ((uint8_t)ch < 0x20 || (uint8_t)ch > 0x7E) {
            ch = ' '; 
        }

        lcd_data((uint8_t)ch);
        col++;
    }
}