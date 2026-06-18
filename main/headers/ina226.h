#ifndef INA226_H
#define INA226_H

#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t ina226_init(i2c_master_bus_handle_t bus_handle);
float ina226_get_bus_voltage(void);
float ina226_get_current(void);
float ina226_get_power(void);
double ina226_get_energy_wh(void);
void ina226_update_energy(float power_watts, float delta_seconds);

#endif // INA226_H