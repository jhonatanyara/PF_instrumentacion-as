#include "ina226.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "sdkconfig.h"
#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "INA226";
static i2c_master_dev_handle_t s_ina226_handle = NULL;
static double s_energy_wh = 0.0;
static bool s_ina226_available = false;

#define INA226_ADDRESS CONFIG_INA226_I2C_ADDRESS
#define INA226_REG_CONFIG 0x00
#define INA226_REG_SHUNT_VOLTAGE 0x01
#define INA226_REG_BUS_VOLTAGE 0x02
#define INA226_REG_POWER 0x03
#define INA226_REG_CURRENT 0x04
#define INA226_REG_CALIBRATION 0x05
#define INA226_SHUNT_OHMS 0.1f

static esp_err_t ina226_write_register(uint8_t reg, uint16_t value) {
    uint8_t buffer[3] = {reg, (uint8_t)(value >> 8), (uint8_t)value};
    return i2c_master_transmit(s_ina226_handle, buffer, sizeof(buffer), 1000);
}

static esp_err_t ina226_read_register(uint8_t reg, uint16_t *value) {
    if (!s_ina226_available || s_ina226_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t reg_addr = reg;
    uint8_t buffer[2] = {0};
    esp_err_t err = i2c_master_transmit_receive(s_ina226_handle, &reg_addr, 1, buffer, sizeof(buffer), 1000);
    if (err != ESP_OK) {
        return err;
    }
    *value = (uint16_t)buffer[0] << 8 | buffer[1];
    return ESP_OK;
}

static esp_err_t ina226_read_signed_register(uint8_t reg, int16_t *value) {
    uint16_t raw = 0;
    esp_err_t err = ina226_read_register(reg, &raw);
    if (err != ESP_OK) {
        return err;
    }
    *value = (int16_t)raw;
    return ESP_OK;
}

static void ina226_scan_addresses(i2c_master_bus_handle_t bus_handle) {
    ESP_LOGI(TAG, "Escaneando el bus I2C para direcciones 0x40-0x4F...");
    for (uint16_t addr = 0x40; addr <= 0x4F; ++addr) {
        esp_err_t probe_err = i2c_master_probe(bus_handle, addr, 1000);
        if (probe_err == ESP_OK) {
            ESP_LOGI(TAG, "Dispositivo I2C detectado en 0x%02x", addr);
        }
    }
}

esp_err_t ina226_init(i2c_master_bus_handle_t bus_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = INA226_ADDRESS,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
        .flags.disable_ack_check = 0,
    };

    esp_err_t err = i2c_master_probe(bus_handle, INA226_ADDRESS, 1000);
    if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGW(TAG, "INA226 no responde en la dirección 0x%02x", INA226_ADDRESS);
        ina226_scan_addresses(bus_handle);
        return err;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error al sondear el INA226: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &s_ina226_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error al inicializar el INA226 en la dirección 0x%02x: %s", INA226_ADDRESS, esp_err_to_name(err));
        return err;
    }

    uint16_t config = 0x4127;
    err = ina226_write_register(INA226_REG_CONFIG, config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo escribir la configuración del INA226: %s", esp_err_to_name(err));
        return err;
    }

    uint16_t calibration = 4096;
    err = ina226_write_register(INA226_REG_CALIBRATION, calibration);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo escribir la calibración del INA226: %s", esp_err_to_name(err));
        return err;
    }

    s_energy_wh = 0.0;
    s_ina226_available = true;
    ESP_LOGI(TAG, "INA226 inicializado correctamente.");
    return ESP_OK;
}

float ina226_get_bus_voltage(void) {
    if (!s_ina226_available) {
        return 0.0f;
    }
    uint16_t raw = 0;
    if (ina226_read_register(INA226_REG_BUS_VOLTAGE, &raw) != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer registro BUS_VOLTAGE");
        return 0.0f;
    }
    uint16_t value = raw >> 3;
    float voltage = value * 0.00125f;
    ESP_LOGI(TAG, "[BUS_V] raw=0x%04x (%u) shifted=%u voltage=%.4fV", raw, raw, value, voltage);
    return voltage;
}

float ina226_get_current(void) {
    if (!s_ina226_available) {
        return 0.0f;
    }
    int16_t raw = 0;
    if (ina226_read_signed_register(INA226_REG_SHUNT_VOLTAGE, &raw) != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer registro SHUNT_VOLTAGE");
        return 0.0f;
    }
    float shunt_uv = raw * 2.5f;
    float shunt_mv = shunt_uv / 1000.0f;
    float current = (shunt_mv / 1000.0f) / INA226_SHUNT_OHMS;
    ESP_LOGI(TAG, "[SHUNT_V] raw=0x%04x (%d) shunt_uv=%.1f shunt_mv=%.4f current=%.4fA", raw, raw, shunt_uv, shunt_mv, current);
    return current;
}

float ina226_get_power(void) {
    if (!s_ina226_available) {
        return 0.0f;
    }
    return ina226_get_bus_voltage() * ina226_get_current();
}

double ina226_get_energy_wh(void) {
    return s_energy_wh;
}

void ina226_update_energy(float power_watts, float delta_seconds) {
    if (delta_seconds > 0.0f && s_ina226_available) {
        s_energy_wh += (power_watts * delta_seconds) / 3600.0;
    }
}
