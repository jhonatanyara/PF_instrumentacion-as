#ifndef LCD_H
#define LCD_H

#include "driver/i2c_master.h"
#include "esp_err.h"

// Pasamos el "Bus" como parámetro para que el LCD se conecte a él
esp_err_t lcd_init(i2c_master_bus_handle_t bus_handle);

// Funciones de ejemplo que podrías necesitar
void lcd_clear(void);
void lcd_print(const char *text);

#endif // LCD_H