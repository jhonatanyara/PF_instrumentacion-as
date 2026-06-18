# Sistema de Instrumentación DC con ESP32

Monitor de energía en tiempo real con interfaz web, pantalla LCD y conectividad WiFi dual (SoftAP + STA).

## 📋 Descripción General

Este proyecto implementa un analizador de potencia en tiempo real basado en ESP32 que:
- Mide voltaje, corriente, potencia y energía acumulada
- Proporciona interfaz web mediante servidor HTTP
- Muestra datos en tiempo real en pantalla LCD
- Funciona como punto de acceso WiFi (SoftAP) y cliente WiFi (STA) simultáneamente
- Monitorea dos entradas GPIO para estado de dispositivos

### Especificaciones Técnicas

| Componente | Especificación |
|-----------|-----------------|
| **Microcontrolador** | ESP32 (Xtensa dual-core) |
| **RAM** | 520 KB SRAM |
| **Flash** | 4 MB |
| **Frecuencia** | 160 MHz (configurable) |
| **Voltaje de entrada** | 3.3V DC |
| **Sensor de energía** | INA226 (I2C) |
| **Display** | LCD 16x2 con módulo I2C PCF8574 |
| **WiFi** | 802.11 b/g/n |
| **API REST** | HTTP en puerto 80 |

---

## 🔌 Diagrama de Conexiones

### Conexiones Principales

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32                              │
│  (NodeMCU-32S / ESP32 DevKit)                              │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────┐
│  Bus I2C (Compartido)               │
│  Velocidad: 100 kHz - 400 kHz       │
└─────────────────────────────────────┘
        │                    │
        │ SDA (GPIO 21)      │ SCL (GPIO 22)
        │                    │
    ┌───▼────────┐      ┌────────────┐
    │   INA226   │      │  LCD 16x2  │
    │  (Sensor)  │      │ PCF8574    │
    │            │      │ (Display)  │
    └────────────┘      └────────────┘
    Addr: 0x40          Addr: 0x27
```

### Pines GPIO Utilizados

| GPIO | Función | Tipo | Notas | Estado |
|------|---------|------|-------|--------|
| 1 | TX (UART0) | - | No utilizado | Disponible |
| 3 | RX (UART0) | - | No utilizado | Disponible |
| 21 | I2C SDA | Bidireccional | Sensor + Display | **EN USO** |
| 22 | I2C SCL | Salida | Sensor + Display | **EN USO** |
| 32 | Estado 1 | Entrada | Pull-up interno | **EN USO** |
| 33 | Estado 2 | Entrada | Pull-up interno | **EN USO** |

---

## 🔧 Pinout Detallado del ESP32 (NodeMCU-32S)

### Placa ESP32 - Vista Superior

```
┌─────────────────────────────────────────────┐
│          ESP32 DEVKIT / NodeMCU-32S         │
│                                             │
│ GND  D35 D34 D39 D36 D4  D2  D15 D13 D12   │
│ 3V3  D25 D26 D27 D14 D12 GND RST D9 D10   │
│ EN   D32 D33 D25 D26 D27 D14 D12 D11 D8   │
│ SVN  D35 D34 D39 D36 D4  D2  D15 D13 RXD0 │
│                                             │
│              CENTRO DEL CHIP                │
│                                             │
│ TXD0 D9  D10 D11 D6  D7  D8  D5  D3  D1   │
│ D0   D16 D17 D5  D18 D19 D20 D21 D22 D23   │
│ GND  3V3 GND D4  D2  D15 D13 D12 GND D35   │
│                                             │
└─────────────────────────────────────────────┘
```

### Pines del ESP32 para este Proyecto

#### **Grupo I2C (Bus de Comunicación)**
```
         ESP32                 INA226              LCD PCF8574
         ─────                 ──────              ────────────
         
GPIO21 (SDA) ────┬─────────────────────────────┬─────────────
               Pull-up                         Pull-up
              10kΩ (interno)                   10kΩ (interno)
                   │ GND                            │ GND
                   
GPIO22 (SCL) ────┬─────────────────────────────┬─────────────
               Pull-up                         Pull-up
              10kΩ (interno)                   10kΩ (interno)
                   │ GND                            │ GND
                   
    3V3 ─────────────────────────────────────────────────
                   │ VCC                            │ VCC
    
    GND ─────────────────────────────────────────────────
                   │ GND                            │ GND
```

#### **Grupo GPIO de Entrada**
```
         ESP32                  Dispositivo 1         Dispositivo 2
         ─────                  ─────────────         ─────────────
         
GPIO32 ──────┐
             ├── Pull-up interno 100kΩ ──► Señal 1 (Presencia/Estado)
       GND ──┤
       
GPIO33 ──────┐
             ├── Pull-up interno 100kΩ ──► Señal 2 (Presencia/Estado)
       GND ──┤
```

---

## 📍 Especificación de Pines ESP32

### Especificaciones Eléctricas

| Parámetro | Valor |
|-----------|-------|
| **Voltaje de E/S** | 3.3V ±10% |
| **Corriente máxima por pin** | 40 mA |
| **Corriente máxima total (todos los pines)** | 1.2 A |
| **Pull-up/Pull-down interno** | ~45kΩ (configurable) |
| **Capacidad de protección** | ESD ±4kV |

### Configuración Detallada por Función

#### **1. Bus I2C (GPIO 21 - SDA, GPIO 22 - SCL)**

```
Configuración:
├─ Modo: Open-Drain (I2C Standard)
├─ Velocidad: 100 kHz - 400 kHz (configurado: 100 kHz)
├─ Dispositivos conectados: 2
│  ├─ INA226 (Sensor) → Dirección 0x40
│  └─ PCF8574 (Display) → Dirección 0x27
└─ Impedancia: Pull-up software a 3.3V

Requerimientos de cableado:
├─ Tipo de cable: Twisted pair (cable trenzado)
├─ Largo máximo: 1 metro (para 100 kHz)
├─ AWG recomendado: 22 (0.33 mm²)
└─ Impedancia: 100-120Ω
```

#### **2. GPIO 32 - Estado 1 (Entrada Digital)**

```
Configuración:
├─ Modo: INPUT
├─ Pull-up: HABILITADO (interno)
├─ Pull-down: DESHABILITADO
├─ Voltaje de lógica: 3.3V
├─ Voltaje alto (1): 2.4V - 3.3V
└─ Voltaje bajo (0): 0V - 0.8V

Rango de transición: 0.8V - 2.4V

Uso recomendado:
├─ Detectar presencia de dispositivo
├─ Monitorear estado de interruptor
├─ Lectura de sensor digital
└─ Señal de control externo
```

#### **3. GPIO 33 - Estado 2 (Entrada Digital)**

```
Configuración:
├─ Modo: INPUT
├─ Pull-up: HABILITADO (interno)
├─ Pull-down: DESHABILITADO
├─ Voltaje de lógica: 3.3V
├─ Voltaje alto (1): 2.4V - 3.3V
└─ Voltaje bajo (0): 0V - 0.8V

Rango de transición: 0.8V - 2.4V

Uso recomendado:
├─ Detectar presencia de dispositivo
├─ Monitorear estado de interruptor
├─ Lectura de sensor digital
└─ Señal de control externo
```

---

## 🎯 Esquema de Conexión Paso a Paso

### Paso 1: Conexión del Bus I2C

```
┌──────────────────────────────────────────────────────────────┐
│                     ESP32 (NodeMCU-32S)                      │
│                                                              │
│  GPIO 21 (SDA)  ─────────────┐                              │
│  GPIO 22 (SCL)  ─────────────┤                              │
│  GND            ─────────────┤                              │
│  3V3            ─────────────┘                              │
└────────────┬──────────────────────────┬─────────────────────┘
             │                          │
        ┌────▼──────────┐         ┌─────▼──────────┐
        │   INA226      │         │ LCD 16x2       │
        │   Sensor      │         │ + PCF8574      │
        ├───────────────┤         ├────────────────┤
        │ SDA ──── 21   │         │ SDA ──── 21    │
        │ SCL ──── 22   │         │ SCL ──── 22    │
        │ GND ──── GND  │         │ GND ──── GND   │
        │ VCC ──── 3V3  │         │ VCC ──── 3V3   │
        │ 0x40 (Addr)   │         │ 0x27 (Addr)    │
        └───────────────┘         └────────────────┘
```

**Conexión I2C con Resistencias Pull-up (si no están integradas):**

```
    3V3
    ║
    ╠═════ [10kΩ] ─── GPIO21 (SDA)
    ║
    ╠═════ [10kΩ] ─── GPIO22 (SCL)
    ║
   ===
   ···
    ║
   GND
```

### Paso 2: Conexión de GPIO de Entrada

```
┌──────────────────────────────────────────────────────────────┐
│                     ESP32 (NodeMCU-32S)                      │
│                                                              │
│  GPIO 32 ──────────────┐  Entrada de lógica 3.3V            │
│  GPIO 33 ──────────────┤  Pull-up interno 100kΩ             │
│  GND    ──────────────┘                                      │
└────────────┬────────────────┬────────────────┬──────────────┘
             │                │                │
        ┌────▼────┐      ┌────▼────┐    ┌─────▼────┐
        │Sensor 1  │      │Sensor 2  │    │ Control  │
        │(Contacto)│      │(Contacto)│    │ Manual   │
        └──────────┘      └──────────┘    └──────────┘
```

**Configuración de Entrada:**

```
Normal (sin usar):                    Con dispositivo externo:

       ESP32                                  ESP32
       GPIO32                                GPIO32
         │                                     │
        === GND (Pull-up interno)      3V3 ────┬──── [5kΩ]
         │                             │       │
        GND                            └───+───┘
                                           │
                                        [SENSOR/
                                         SWITCH]
                                           │
                                          GND
```

---

## 📊 Tabla Resumen de Conexiones

| Componente | Pin ESP32 | Conexión | Voltaje | Corriente |
|-----------|-----------|----------|---------|-----------|
| **INA226 SDA** | GPIO 21 | Directo | 3.3V | <5mA |
| **INA226 SCL** | GPIO 22 | Directo | 3.3V | <5mA |
| **LCD SDA** | GPIO 21 | Compartido I2C | 3.3V | <5mA |
| **LCD SCL** | GPIO 22 | Compartido I2C | 3.3V | <5mA |
| **Estado 1** | GPIO 32 | Con pull-up | 3.3V | <1mA |
| **Estado 2** | GPIO 33 | Con pull-up | 3.3V | <1mA |
| **GND** | GND | Común | 0V | Variable |
| **3V3** | 3V3 | Alimentación | 3.3V | 200-500mA |

---

## ⚠️ Consideraciones Importantes

### Protección de Entrada

Para los pines GPIO 32 y GPIO 33, si conectan voltajes >3.3V:

```
Entrada externa (0-5V) ──┬──── [1kΩ] ──── GPIO
                        │
                       [1N4148]
                        │
                        GND (Protección contra sobretensión)
```

### Longitud de Cables Recomendada

| Función | Largo Máximo | Recomendado |
|---------|--------------|-------------|
| I2C (100 kHz) | 3 metros | 1 metro |
| I2C (400 kHz) | 1 metro | 0.5 metros |
| GPIO entrada | 5 metros | 1-2 metros |

### Blindaje de Cables

Para sistemas con ruido electromagnético:
- Usar cable **trenzado** para I2C
- Usar cable **apantallado** para GPIO de entrada
- Tierra del blindaje → GND del ESP32

---

## 🔋 Conexión a Fuente de Alimentación

### Configuración del Circuito

```
┌─────────────────────────────────────────────────────────────┐
│                  FUENTE DC (0-32V recomendado)              │
│                                                             │
│  (+) ─────────────┬─────────────────┐                      │
│                   │                 │                      │
│              ┌────▼──────┐     ┌────▼──────┐               │
│              │   INA226   │     │  Carga    │               │
│              │  (Shunt)   │     │  (Medida) │               │
│              └────┬──────┘     └────┬──────┘               │
│                   │                 │                      │
│  (-) ─────────────┴─────────────────┘                      │
│
│  [Conexión al ESP32]
│  VBus+ ──┐
│  GND  ───┼──► GND del ESP32
│          │
│          └──► A/D (opcional, si se mide voltaje analógico)
└─────────────────────────────────────────────────────────────┘
```

### Especificaciones del Sensor INA226

El INA226 es un monitor de potencia de precisión con interfaz I2C:

| Parámetro | Rango |
|-----------|-------|
| **Voltaje de entrada (VBUS)** | 0V a 36V |
| **Corriente (mediante shunt)** | ±20A (típico) |
| **Resolución de voltaje** | 1.25 mV por LSB |
| **Resolución de corriente** | Configurable (depende del shunt) |
| **Resolución de potencia** | 25 mW por LSB |
| **Dirección I2C** | 0x40 (configurable por pines A0-A3) |

### Conexiones del INA226

```
INA226 Pinout:
┌─────────────────┐
│   Sensor INA226 │
│                 │
│ IN+ ────────────┤ (a línea positiva de carga)
│ IN- ────────────┤ (a línea negativa/común)
│ GND ────────────┤ (a GND del ESP32)
│ VCC ────────────┤ (a 3.3V del ESP32)
│ SCL ────────────┤ (a GPIO 22 del ESP32)
│ SDA ────────────┤ (a GPIO 21 del ESP32)
│ A0, A1, A2, A3  │ (No conectados = Addr 0x40)
└─────────────────┘
```

### Conexiones del Display LCD

```
LCD 16x2 con módulo I2C PCF8574:
┌──────────────────────┐
│  LCD + PCF8574       │
│                      │
│ GND ────────────────┤ (a GND del ESP32)
│ VCC ────────────────┤ (a 5V o 3.3V del ESP32)*
│ SDA ────────────────┤ (a GPIO 21 del ESP32)
│ SCL ────────────────┤ (a GPIO 22 del ESP32)
└──────────────────────┘

* Nota: Algunos módulos PCF8574 funcionan con 3.3V
  Si no funciona, alimentar a 5V.
```

---

## 📡 Protocolos de Comunicación

### 1. I2C (Inter-Integrated Circuit)

**Configuración:**
- **Velocidad:** 100-400 kHz (configurable en `Kconfig.projbuild`)
- **Pines:** GPIO 21 (SDA), GPIO 22 (SCL)
- **Pull-ups internos:** Habilitados automáticamente

**Dispositivos en el bus:**
1. **INA226** (Dirección: 0x40)
   - Lee mediciones de voltaje, corriente y potencia
   - Actualización cada 2 segundos

2. **PCF8574 + LCD** (Dirección: 0x27)
   - Control de display LCD 16x2
   - Actualización cada 2 segundos

### 2. WiFi 802.11

El ESP32 funciona en **modo dual** (SoftAP + STA):

#### Modo Punto de Acceso (SoftAP)
```
SSID: POCO (configurable)
Contraseña: 123456789 (configurable)
Seguridad: WPA2
Canal: 1 (configurable)
Max. conexiones: 4
IP: 192.168.4.1
Puerta de enlace: 192.168.4.1
```

#### Modo Cliente (STA)
```
Red remota: POCO (configurable)
Contraseña: 123456789 (configurable)
DHCP: Habilitado
IP: Asignada por el router
Sincronización NTP: pool.ntp.org
```

### 3. HTTP REST API

**Servidor web:**
- **Puerto:** 80
- **Contenido:** Estático (HTML + JavaScript) + API JSON

#### Endpoints

##### GET `/`
Sirve la página web de monitoreo

**Respuesta:** HTML + JavaScript
```html
<!DOCTYPE html>
<html>
<head>
  <title>Instrumentación ESP32</title>
</head>
<body>
  <h1>Dashboard de Consumo DC</h1>
  <!-- Tabla de datos -->
</body>
</html>
```

##### GET `/api/status`
Retorna datos actuales en JSON

**Respuesta:**
```json
{
  "time": "2024-06-11 14:30:45",
  "voltage": 12.45,
  "current": 2.850,
  "power": 35.480,
  "energy_wh": 1234.5678,
  "gpio_1": 1,
  "gpio_2": 0
}
```

**Descripción de campos:**
| Campo | Unidad | Descripción |
|-------|--------|-------------|
| `time` | - | Hora sincronizada via NTP |
| `voltage` | V | Voltaje en la carga |
| `current` | A | Corriente en la carga |
| `power` | W | Potencia instantánea |
| `energy_wh` | Wh | Energía acumulada |
| `gpio_1` | - | Estado de GPIO 32 (1=HIGH, 0=LOW) |
| `gpio_2` | - | Estado de GPIO 33 (1=HIGH, 0=LOW) |

##### GET `/favicon.ico`
Retorna 204 No Content

---

## 🌐 Sistema WiFi Dual (SoftAP + STA)

### Descripción General del Sistema WiFi

El ESP32 implementa un sistema WiFi híbrido que funciona simultáneamente como:
- **Punto de Acceso (AP/SoftAP):** Permite que otros dispositivos se conecten directamente al ESP32
- **Cliente (STA):** Se conecta a una red WiFi existente para acceso remoto y sincronización de hora

Esta configuración proporciona dos formas de acceder al sistema:

```
                    ┌─────────────────────────────┐
                    │         Router WiFi         │
                    │    (Red Principal)          │
                    └────────────┬────────────────┘
                                 │
                    ┌────────────▼────────────────┐
                    │       ESP32 (STA)           │
                    │   Conectado al Router       │
                    │  IP: 192.168.1.XXX          │
                    │                             │
                    │  ┌─────────────────────┐    │
                    │  │  ESP32 (SoftAP)     │    │
                    │  │  Red local          │    │
                    │  │  SSID: POCO         │    │
                    │  │  IP: 192.168.4.1    │    │
                    │  └────────┬────────────┘    │
                    │           │                 │
                    └───────────┼─────────────────┘
                                │
                ┌───────────────┼───────────────┐
                │               │               │
         ┌──────▼────────┐  ┌──▼──────────┐  ┌─▼────────────┐
         │  Móvil 1      │  │ Móvil 2     │  │ Computadora  │
         │  Conectado    │  │ Conectado   │  │ Conectado    │
         │  al SoftAP    │  │ al SoftAP   │  │ al SoftAP    │
         └───────────────┘  └─────────────┘  └──────────────┘
```

### Arquitectura WiFi

#### Interfaz SoftAP (Punto de Acceso)

**Función:** Proporciona red local independiente al ESP32

| Parámetro | Valor | Notas |
|-----------|-------|-------|
| **SSID** | POCO (configurable) | Nombre visible de la red |
| **Contraseña** | 123456789 (configurable) | Seguridad WPA2-PSK |
| **IP del ESP32** | 192.168.4.1 | Gateway y DNS |
| **Máscara de red** | 255.255.255.0 | /24 |
| **Rango DHCP** | 192.168.4.2 - 192.168.4.255 | Asignadas dinámicamente |
| **Máximo de conexiones** | 4 (configurable) | Clientes simultáneos |
| **Canal WiFi** | 1 (configurable) | 2.4 GHz |
| **Protocolo** | 802.11 b/g/n | 20 MHz ancho de banda |

**Ventajas:**
- ✅ No requiere router externo
- ✅ Conexión inmediata sin configuración
- ✅ Ideal para monitoreo local
- ✅ Funciona en cualquier ubicación

**Desventajas:**
- ❌ Rango limitado (~50-100m en línea de vista)
- ❌ Velocidad limitada (hasta 54 Mbps teórico)

#### Interfaz STA (Cliente WiFi)

**Función:** Conecta el ESP32 a una red existente

| Parámetro | Valor | Notas |
|-----------|-------|-------|
| **Red Remota** | otherapssid (configurable) | SSID del router |
| **Contraseña** | otherappassword (configurable) | De la red remota |
| **Seguridad** | WPA2-PSK (mínimo) | Autenticación |
| **IP del ESP32** | Asignada por DHCP | De la red remota |
| **Sincronización NTP** | pool.ntp.org | Servidores de hora públicos |
| **Reintentos** | 5 (configurable) | Intentos de conexión |
| **Timeout** | ~10 segundos | Espera por conexión |

**Ventajas:**
- ✅ Acceso remoto desde cualquier dispositivo en la red
- ✅ Sincronización de hora automática via NTP
- ✅ Mejor rendimiento y estabilidad
- ✅ Rango extendido (limitado por el router)

**Desventajas:**
- ❌ Requiere un router WiFi disponible
- ❌ Depende de conectividad del router
- ❌ Sincronización de hora solo con conexión

### Flujo de Inicialización WiFi

```
┌─────────────────────┐
│  esp_netif_init()   │ ◄─── Inicializa capa de red
└────────┬────────────┘
         │
┌────────▼──────────────────┐
│  esp_event_loop_create()  │ ◄─── Crea manejador de eventos
└────────┬───────────────────┘
         │
┌────────▼──────────────────────────┐
│  esp_netif_create_default_wifi_ap()  │ ◄─── Crea interfaz AP
└────────┬───────────────────────────┘
         │
┌────────▼──────────────────────────┐
│  esp_netif_create_default_wifi_sta() │ ◄─── Crea interfaz STA
└────────┬────────────────────────────┘
         │
┌────────▼──────────────────┐
│   esp_wifi_init()         │ ◄─── Inicializa WiFi
└────────┬───────────────────┘
         │
┌────────▼──────────────────────────┐
│  esp_event_handler_register()      │ ◄─── Registra manejadores
└────────┬───────────────────────────┘
         │
┌────────▼────────────────────────┐
│  esp_wifi_set_mode()             │ ◄─── WIFI_MODE_APSTA
└────────┬─────────────────────────┘
         │
┌────────▼──────────────────────────┐
│  esp_wifi_set_config(AP)          │ ◄─── Configura AP
└────────┬───────────────────────────┘
         │
┌────────▼──────────────────────────┐
│  esp_wifi_set_config(STA)         │ ◄─── Configura STA
└────────┬───────────────────────────┘
         │
┌────────▼──────────────────┐
│   esp_wifi_start()        │ ◄─── Inicia WiFi
└────────┬───────────────────┘
         │
┌────────▼──────────────────────────────────┐
│  ✓ SoftAP activo inmediatamente           │
│  ✓ STA intenta conectar a red remota      │
│  ✓ Servidor HTTP inicia en puerto 80      │
└───────────────────────────────────────────┘
```

### Manejadores de Eventos WiFi

El sistema implementa manejadores de eventos que reaccionan a cambios en el estado WiFi:

#### 1. **WIFI_EVENT_STA_START**
```
Evento: El modo STA se ha iniciado
Acción: Inicia conexión a red remota (esp_wifi_connect)
Reintentos: Configurados en menuconfig
```

#### 2. **WIFI_EVENT_STA_DISCONNECTED**
```
Evento: Se perdió conexión con red remota
Acción: Reintenta conexión automáticamente
Máximo de intentos: 5 (configurable)
Intervalo: ~2 segundos entre intentos
```

#### 3. **IP_EVENT_STA_GOT_IP**
```
Evento: Se asignó IP en la interfaz STA
Acción: Inicia sincronización NTP para obtener hora
Log: Muestra IP asignada
Siguiente: Se pueden hacer consultas remotas
```

#### 4. **WIFI_EVENT_AP_STACONNECTED**
```
Evento: Un cliente se conectó al SoftAP
Acción: Log de conexión
Efecto: Aumenta contador de clientes conectados
```

#### 5. **WIFI_EVENT_AP_STADISCONNECTED**
```
Evento: Un cliente se desconectó del SoftAP
Acción: Log de desconexión
Efecto: Disminuye contador de clientes
```

### Sincronización de Hora con NTP

Cuando el ESP32 se conecta exitosamente a la red remota (STA), automáticamente sincroniza la hora del sistema usando servidores NTP (Network Time Protocol):

**Configuración:**
```
Servidor NTP principal: pool.ntp.org
Modo de operación: SNTP_OPMODE_POLL (sondeo periódico)
Frecuencia: Cada 3600 segundos (1 hora)
Zona horaria: UTC (configurable en SDK)
```

**Beneficios:**
- Hora precisa en registros y datos
- Facilita análisis de datos temporales
- Esencial para sistemas distribuidos
- Timestamp exacto en API REST

**Flujo:**
```
STA Conectada → NTP Init → Pool.ntp.org → Hora Sincronizada → 
→ API REST devuelve hora actual con timestamp
```

### Servidor HTTP Web

**Especificaciones:**
- **Puerto:** 80 (HTTP estándar)
- **Tipo:** Servidor embebido Apache
- **Máximo de sockets:** 2 (simultáneos)
- **Stack size:** Dinámico
- **Autenticación:** No requiere (red local)

**Características:**
```
✓ Servidor web integrado en ESP32
✓ Sirve archivos estáticos (HTML/CSS/JS)
✓ API REST JSON
✓ Respuestas optimizadas para dispositivos móviles
✓ Actualización automática cada 2 segundos
✓ Bajo consumo de recursos
```

### Formas de Acceder al Sistema

#### 1. Conexión Directa al SoftAP (Recomendado para Pruebas)

```
Paso 1: Abre configuración WiFi del dispositivo
Paso 2: Busca red "POCO" (o nombre configurado)
Paso 3: Conecta con contraseña "123456789"
Paso 4: Abre navegador
Paso 5: Escribe en la URL: http://192.168.4.1
Paso 6: ¡Listo! Dashboard visible
```

**Ventajas:**
- Sin dependencias de router
- Conexión instantánea
- Pruebas rápidas

#### 2. Conexión vía Red Remota (STA)

```
Paso 1: Configura SSID y contraseña del router en menuconfig
Paso 2: Flash del código al ESP32
Paso 3: Observa en monitor serial la IP asignada (ej: 192.168.1.50)
Paso 4: En cualquier dispositivo conectado al mismo router
Paso 5: Abre navegador
Paso 6: Escribe: http://192.168.1.50 (o la IP asignada)
Paso 7: ¡Acceso remoto activo!
```

**Ventajas:**
- Acceso desde cualquier dispositivo en la red
- Mejor estabilidad
- Sincronización de hora automática
- Ideal para instalación permanente

#### 3. Acceso Simultáneo Híbrido

```
┌─────────────────────────────────────────────────┐
│          AMBAS CONEXIONES ACTIVAS               │
│                                                 │
│  Dispositivo A ─ WiFi directo SoftAP           │
│                  192.168.4.1                   │
│                                                 │
│  Dispositivo B ─ Red del router (STA)          │
│                  192.168.1.50                  │
│                  (IP asignada)                  │
│                                                 │
│  Ambos ven exactamente los mismos datos        │
│  Actualizados en tiempo real simultáneamente   │
└─────────────────────────────────────────────────┘
```

### Seguridad WiFi

**Configuración de Seguridad Actual:**

| Elemento | Implementación | Nivel |
|----------|-----------------|-------|
| **Autenticación AP** | WPA2-PSK | ⭐⭐⭐ Bueno |
| **Encriptación** | AES-128 | ⭐⭐⭐ Bueno |
| **Contraseña AP** | 123456789 (cambiar) | ⚠️ Débil |
| **Acceso HTTP** | Sin HTTPS | ⚠️ No encriptado |
| **Autenticación STA** | WPA2-PSK | ⭐⭐⭐ Bueno |
| **Aislamiento de clientes** | No implementado | ⚠️ Acceso completo |

**Recomendaciones de Seguridad:**

1. **Cambiar contraseña del AP**
   ```bash
   idf.py menuconfig
   # Example Configuration → WiFi AP Configuration
   # Cambiar "WiFi AP Password" a algo más seguro
   ```

2. **Usar HTTPS en producción**
   ```
   Requiere certificados SSL/TLS
   Mayor consumo de recursos
   Necesario para datos sensibles
   ```

3. **Implementar autenticación en API**
   ```
   Token Bearer o API Key
   Validación de origen (CORS)
   Rate limiting
   ```

4. **Red Remota Segura**
   - Usar WPA3 si el router lo soporta
   - Contraseña fuerte para red remota
   - No compartir credenciales

### Monitoreo de Conexión WiFi

**En la consola serial verás:**

```
I (1234) WIFI_MANAGER: Inicializando WiFi en modo AP + STA...
I (2345) WIFI_MANAGER: GPIO de estado inicializados: GPIO 32, GPIO 33
I (3456) WIFI_MANAGER: Inicialización WiFi completada.
I (4567) WIFI_MANAGER: ¡Conectado! Dirección IP adquirida en la interfaz STA.
I (5678) WIFI_MANAGER: Inicializando SNTP para sincronizar hora...
I (6789) WIFI_MANAGER: Iniciando servidor web HTTP...
I (7890) WIFI_MANAGER: Servidor web iniciado en puerto 80
I (8901) WIFI_MANAGER: Un dispositivo se ha conectado al AP.
```

### Diagnóstico de Conectividad

Para verificar estado de WiFi en tiempo real:

```bash
# Ver logs en tiempo real
idf.py monitor

# Filtrar solo logs WiFi
idf.py monitor | grep -i wifi

# Capturar salida completa
idf.py monitor > output.log
```

**Señales de funcionamiento correcto:**
- ✅ "Inicialización WiFi completada"
- ✅ "Conectado! Dirección IP adquirida" (si STA está configurado)
- ✅ "Servidor web iniciado en puerto 80"
- ✅ Hora sincronizada en respuestas JSON

**Señales de problemas:**
- ❌ "Fallo al conectar" repetidamente
- ❌ "SNTP no respondiendo"
- ❌ No hay asignación de IP
- ❌ Timeout en conexión HTTP

---

## ⚙️ Configuración del Software

### Configuración mediante `menuconfig`

```bash
idf.py menuconfig
```

Menú de configuración disponible en `Example Configuration`:

#### 1. **SoftAP Configuration**
```
- WiFi AP SSID: Nombre de la red (default: "POCO")
- WiFi AP Password: Contraseña (default: "123456789")
- WiFi AP Channel: Canal 1-14 (default: 1)
- Maximal STA connections: Máximo de clientes (default: 4)
```

#### 2. **STA Configuration**
```
- WiFi Remote AP SSID: Red remota a conectarse (default: "otherapssid")
- WiFi Remote AP Password: Contraseña (default: "otherappassword")
- Maximum retry: Reintentos de conexión (default: 5)
- WiFi Scan auth mode threshold: Seguridad mínima (default: WPA2 PSK)
```

#### 3. **Sensor y Display I2C**
```
- I2C SDA pin: GPIO para SDA (default: 21)
- I2C SCL pin: GPIO para SCL (default: 22)
- Dirección I2C del módulo LCD: 0x27 (configurable)
```

---

## 🚀 Instalación y Compilación

### Requisitos Previos

- ESP-IDF v6.0.1 instalado
- Herramientas de compilación:
  - `cmake` 3.20+
  - `ninja` 1.12+
  - `xtensa-esp32-elf` toolchain
- Python 3.8+

### Pasos de Instalación

1. **Clonar/Descargar el proyecto:**
```bash
cd /tu/ruta/softap_sta
```

2. **Configurar el entorno ESP-IDF:**
```bash
$env:IDF_PATH = 'C:\esp\v6.0.1\esp-idf'
```

3. **Limpiar build anterior (si existe):**
```bash
idf.py fullclean
```

4. **Configurar el proyecto:**
```bash
idf.py menuconfig
```
   - Ajusta SSID, contraseñas y pines según tu necesidad
   - Guarda y sal (Q)

5. **Compilar:**
```bash
idf.py build
```

6. **Flashear en el ESP32:**
```bash
idf.py flash
```

7. **Monitorear salida:**
```bash
idf.py monitor
```

---

## 📊 Datos de Salida

### Pantalla LCD
```
V: 12.45V I: 2.85A
P: 35.48W E:1234.57Wh
```

### Salida Serial (GPIO 1/3)
```
I (1234) WIFI_MANAGER: Inicializando WiFi en modo AP + STA...
I (5678) WIFI_MANAGER: Inicialización WiFi completada.
I (6789) WIFI_MANAGER: Inicializando SNTP para sincronizar hora...
I (8901) WIFI_MANAGER: Iniciando servidor web HTTP...
```

### Interfaz Web
- Acceso local: `http://192.168.4.1` (SoftAP)
- Acceso remoto: `http://<IP_ESP32_STA>` (mediante red local)
- Dashboard interactivo con actualización cada 2 segundos
- Soporte responsivo para dispositivos móviles

---

## 🔧 Solución de Problemas

### El ESP32 no se conecta a WiFi
1. Verifica SSID y contraseña en `menuconfig`
2. Asegúrate que el router está disponible
3. Revisa los logs: `idf.py monitor`
4. Aumenta `ESP_MAXIMUM_STA_RETRY` si es necesario

### El sensor INA226 no responde
1. Verifica conexión I2C (SDA/SCL)
2. Confirma dirección I2C: `0x40` (sin los pines A0-A3 conectados)
3. Revisa voltaje en VCC del sensor (debe ser 3.3V)
4. El sistema continúa funcionando sin el sensor

### La pantalla LCD no muestra datos
1. Verifica conexión I2C y alimentación
2. Confirma dirección: `0x27`
3. Prueba alimentación a 5V si está disponible
4. Revisa contraste del LCD (ajusta potenciómetro si existe)

### No puedo acceder a la página web
1. Asegúrate de estar conectado a la red SSID del ESP32
2. Intenta con `192.168.4.1` en el navegador
3. Verifica que el puerto 80 no esté bloqueado
4. Reinicia el ESP32 si es necesario

---

## 📈 Arquitectura del Software

```
main()
├── nvs_flash_init()
├── i2c_master_init()
├── lcd_init()
├── ina226_init()
├── wifi_init_softap_sta()
│   ├── esp_netif_init()
│   ├── esp_event_loop_create_default()
│   ├── wifi_event_handler() [Callback]
│   └── esp_wifi_start()
├── start_webserver()
│   ├── httpd_start()
│   ├── uri_root_handler [GET /]
│   ├── uri_api_status_handler [GET /api/status]
│   └── uri_favicon_handler [GET /favicon.ico]
└── display_task() [FreeRTOS]
    ├── ina226_get_bus_voltage()
    ├── ina226_get_current()
    ├── ina226_get_power()
    ├── ina226_get_energy_wh()
    ├── gpio_get_level()
    └── lcd_print()
```

---

## 📝 Notas Importantes

1. **Sincronización de Hora:** La hora se sincroniza automáticamente via NTP cuando el ESP32 se conecta a internet.

2. **Energía Acumulada:** Se calcula integrando la potencia en el tiempo. Se resetea solo con reinicio del ESP32.

3. **Doble Conectividad WiFi:** El ESP32 funciona como router y cliente simultáneamente:
   - Como **SoftAP:** Permite conexiones locales sin necesidad de router
   - Como **STA:** Se conecta a un router para acceso remoto y sincronización NTP

4. **Consumo de Potencia del ESP32:** ~100-200 mA durante operación normal (incluye WiFi, I2C, servidor web).

5. **Precisión del Sensor:** El INA226 tiene precisión de ±0.1% en voltaje y ±0.8% en corriente.

---

## 📚 Componentes Utilizados

### Hardware

#### 1. **Microcontrolador ESP32**
- **Modelo:** ESP32-D0WDQ6 (NodeMCU-32S compatible)
- **Arquitectura:** Xtensa dual-core 32-bits
- **Frecuencia:** 160 MHz (configurable a 80/240 MHz)
- **RAM:** 520 KB SRAM (utilizable)
- **Flash:** 4 MB (particionado: bootloader 64KB, app 1.3MB, storage)
- **GPIO:** 34 pines digitales bidireccionales
- **ADC:** 12-bits, hasta 12 canales
- **I2C:** 2 módulos (Master/Slave)
- **SPI:** 4 módulos
- **UART:** 3 puertos seriales
- **WiFi:** 802.11 b/g/n integrado
- **Bluetooth:** BLE + BR/EDR integrado
- **Voltaje:** 3.3V
- **Consumo típico:** 100-200 mA (WiFi activo)

#### 2. **Sensor INA226 (Monitor de Potencia)**
- **Fabricante:** Texas Instruments
- **Función:** Monitor bidireccional de voltaje y corriente
- **Interfaz:** I2C (dirección 0x40)
- **Voltaje de entrada (VBUS):** 0V a 36V
- **Rango de corriente:** ±20A (mediante shunt)
- **Voltaje de operación:** 3.3V a 5.5V
- **Resolución de voltaje:** 1.25 mV/LSB
- **Resolución de potencia:** 25 mW/LSB
- **Precisión:** ±0.1% voltaje, ±0.8% corriente
- **Tiempo de conversión:** ~2.12 ms (modo 8 muestras)
- **Shunt resistencia:** 0.1Ω (2W típico)
- **Código:** [ina226.c](main/sources/ina226.c) y [ina226.h](main/headers/)

#### 3. **Pantalla LCD 16x2**
- **Tipo:** Display de caracteres LCD
- **Resolución:** 16 caracteres × 2 líneas
- **Interfaz:** Paralela 4-bits + PCF8574 I2C
- **Voltaje:** 5V típico (algunos modelos soportan 3.3V)
- **PCF8574 (Expansor I/O):**
  - Dirección I2C: 0x27 (configurable)
  - 8 pines I/O expuestos
  - 2 líneas I2C
  - Voltaje: 3.3V - 5.5V

#### 4. **Resistencias y Capacitores (Pull-ups)**
- Pull-up I2C SDA/SCL: 4.7kΩ (interno al ESP32)
- Capacitor desacoplamiento: 100nF (recomendado cerca de INA226)
- Capacitor desacoplamiento: 10µF (recomendado en ESP32)

#### 5. **Cables y Conectores**
- Bus I2C: Cable trenzado (twisted pair) AWG 22
- GPIO entrada: Cable estándar AWG 22
- Conector tipo: Dupont 2.54mm o terminal de tornillo
- Largo máximo I2C (100 kHz): 1 metro

### Software

#### Componentes de Software Principales

```
Project Structure:
├── main/
│   ├── sources/
│   │   ├── main.c              ◄── Punto de entrada
│   │   ├── wifi_manager.c      ◄── Gestión WiFi + Servidor HTTP
│   │   ├── ina226.c            ◄── Driver del sensor
│   │   ├── lcd.c               ◄── Control de pantalla LCD
│   │   └── (header files)
│   ├── CMakeLists.txt
│   ├── Kconfig.projbuild       ◄── Configuración del proyecto
│   └── idf_component.yml       ◄── Dependencias
├── CMakeLists.txt              ◄── Configuración CMake
├── sdkconfig                   ◄── Configuración compilada
└── build/                      ◄── Artefactos de compilación
```

#### 1. **Framework: ESP-IDF v6.0.1**
```
├─ FreeRTOS (sistema operativo tiempo real)
├─ Componentes de Red
│  ├─ esp_http_server (servidor HTTP embebido)
│  ├─ esp_wifi (controlador WiFi)
│  ├─ esp_event (manejador de eventos)
│  └─ esp_netif (interfaz de red)
├─ Drivers
│  ├─ i2c_master (controlador I2C)
│  └─ gpio (controlador GPIO)
├─ Utilidades
│  ├─ esp_sntp (sincronización NTP)
│  ├─ nvs_flash (almacenamiento no volátil)
│  └─ esp_log (sistema de logging)
└─ Bootloader (gestor de arranque)
```

#### 2. **Controladores I2C**
- **esp_i2c_master:** Controlador maestro I2C
- **Frecuencia:** 100 kHz (configurable)
- **Pull-ups internos:** Habilitados automáticamente
- **Timeout:** 1000 ms

#### 3. **Servidor HTTP**
- **Librería:** Apache esp_http_server (httpd)
- **Configuración:**
  - Máximo de sockets: 2
  - Máximo de peticiones simultáneas: 2
  - Stack size: 4096 bytes
  - Puerto: 80

#### 4. **Manejador de WiFi**
- **Modo:** WIFI_MODE_APSTA (dual AP + STA simultáneo)
- **Protocolo:** 802.11 b/g/n
- **DHCP:** Habilitado en STA

#### 5. **Sistema de Sincronización de Hora**
- **Protocolo:** SNTP (Simple Network Time Protocol)
- **Servidores:** pool.ntp.org
- **Modo:** SNTP_OPMODE_POLL
- **Frecuencia:** Cada 3600 segundos

#### 6. **FreeRTOS**
```
Tareas principales:
├─ main (Inicialización)
├─ wifi_event_task (Manejador de eventos WiFi)
├─ http_server_task (Servidor HTTP)
├─ display_task (Actualización LCD)
│   └─ Actualiza cada 2000 ms
└─ ina226_read_task (Lectura de sensor)
    └─ Actualiza cada 2000 ms
```

### Dependencias Externas

**Ninguna.** El proyecto utiliza solo componentes integrados en ESP-IDF v6.0.1:
- esp_http_server
- esp_wifi
- esp_event
- esp_netif
- driver/i2c_master
- driver/gpio
- esp_sntp

Esto resulta en un tamaño de binario reducido (~700 KB comprimido).

---

## 📖 Flujo de Ejecución del Sistema

### 1. Inicio del Sistema

```
┌─────────────────────────────┐
│   Power ON / Reset          │
└────────────┬────────────────┘
             │
┌────────────▼────────────────┐
│  Bootloader ejecuta         │
│  ├─ Verifica Flash          │
│  ├─ Carga aplicación        │
│  └─ Salta a main()          │
└────────────┬────────────────┘
             │
┌────────────▼────────────────┐
│  main() se ejecuta          │
│  ├─ nvs_flash_init()        │
│  ├─ i2c_master_init()       │
│  ├─ lcd_init()              │
│  ├─ ina226_init()           │
│  ├─ wifi_init_softap_sta()  │
│  ├─ start_webserver()       │
│  └─ Crea tasks FreeRTOS     │
└────────────┬────────────────┘
             │
┌────────────▼────────────────┐
│  Sistema listo              │
│  ├─ WiFi SoftAP activo      │
│  ├─ WiFi STA intenta conectar
│  ├─ Servidor web escuchando │
│  ├─ LCD mostrando datos     │
│  └─ INA226 tomando muestras │
└─────────────────────────────┘
```

### 2. Ciclo de Operación Normal

```
[Loop Principal - FreeRTOS]
│
├─ Cada 2 segundos:
│  ├─ Leer INA226 (I2C)
│  │  ├─ Voltaje
│  │  ├─ Corriente  
│  │  ├─ Potencia
│  │  └─ Acumular energía
│  │
│  ├─ Leer GPIO (32, 33)
│  │  ├─ Estado 1
│  │  └─ Estado 2
│  │
│  └─ Actualizar LCD
│     ├─ Línea 1: V: 12.45V I: 2.85A
│     └─ Línea 2: P: 35.48W E:1234.57Wh
│
├─ Servidor HTTP escucha (continuo)
│  ├─ GET / → Sirve HTML
│  ├─ GET /api/status → JSON con datos
│  └─ GET /favicon.ico → 204 No Content
│
├─ Manejador de eventos WiFi (contínuo)
│  ├─ Conectado a STA → Inicia NTP
│  └─ Nueva conexión AP → Log
│
└─ Sincronización NTP (cada hora)
   └─ Actualiza reloj del sistema
```

### 3. Solicitud HTTP (Navegador)

```
┌──────────────────────────────────────────┐
│  Dispositivo cliente                     │
│  Abre navegador: http://192.168.4.1     │
└────────────┬─────────────────────────────┘
             │ GET / HTTP/1.1
             │ Host: 192.168.4.1
             │
┌────────────▼──────────────────────────┐
│ ESP32 recibe solicitud                 │
│ ├─ root_get_handler() se ejecuta       │
│ └─ Prepara respuesta HTML              │
│    (incluye JavaScript para updates)   │
└────────────┬──────────────────────────┘
             │ HTTP/1.1 200 OK
             │ Content-Type: text/html
             │ [HTML + JavaScript]
             │
┌────────────▼───────────────────────────┐
│ Navegador recibe y renderiza           │
│ ├─ Muestra Dashboard                   │
│ ├─ JavaScript inicia loop cada 2s      │
│ │  └─ fetch('/api/status')             │
│ │
└────────────┬───────────────────────────┘
             │ GET /api/status HTTP/1.1
             │
┌────────────▼──────────────────────────┐
│ ESP32 recibe solicitud API             │
│ ├─ api_status_get_handler() se ejecuta │
│ ├─ Recopila datos:                     │
│ │  ├─ ina226_get_bus_voltage()         │
│ │  ├─ ina226_get_current()             │
│ │  ├─ ina226_get_power()               │
│ │  ├─ ina226_get_energy_wh()           │
│ │  ├─ gpio_get_level(32)               │
│ │  ├─ gpio_get_level(33)               │
│ │  └─ get_current_time_string()        │
│ ├─ Formatea como JSON                  │
│ └─ Envía respuesta                     │
└────────────┬──────────────────────────┘
             │ HTTP/1.1 200 OK
             │ Content-Type: application/json
             │ {"time":"2024-06-11 14:30:45",...}
             │
┌────────────▼──────────────────────────┐
│ Navegador recibe JSON                  │
│ ├─ Actualiza DOM con nuevos valores    │
│ ├─ Gráficos se actualizan              │
│ └─ Espera 2 segundos para próxima req. │
└──────────────────────────────────────┘
```

---

## 📊 Especificaciones Técnicas Consolidadas

### Mediciones Disponibles

| Medición | Rango | Precisión | Resolución | Tiempo de Muestreo |
|----------|-------|-----------|------------|-------------------|
| **Voltaje** | 0-36V | ±0.1% | 1.25 mV | 2.12 ms |
| **Corriente** | ±20A | ±0.8% | 0.16 mA* | 2.12 ms |
| **Potencia** | 0-720W | ±0.9% | 25 mW | 2.12 ms |
| **Energía** | 0-∞ | Acumulativo | 0.01 Wh | Continua |

*Depende del valor del shunt configurado

### Conectividad Disponible

| Protocolo | Puerto/Pin | Velocidad | Alcance | Estado |
|-----------|-----------|-----------|---------|---------|
| **I2C** | GPIO 21, 22 | 100 kHz | 1-2 m | Activo |
| **WiFi SoftAP** | 2.4 GHz | 54 Mbps | 50-100 m | Activo |
| **WiFi STA** | 2.4 GHz | 150 Mbps | Depende router | Configurable |
| **HTTP/REST** | Puerto 80 | JSON | Ambos WiFi | Activo |
| **GPIO Entrada** | 32, 33 | - | 5 m cable | Activo |

### Consumo de Energía (Operación Normal)

```
Componente              Corriente Típica
─────────────────────────────────────
ESP32 + WiFi            150-200 mA
INA226                  0.7 mA
LCD + PCF8574           5-10 mA
GPIO                    < 1 mA
──────────────────────────────────────
TOTAL                   160-210 mA @ 3.3V
Potencia Total          0.5-0.7 W
```

---

## 🔍 Guía de Diagnóstico y Solución de Problemas

### Tabla de Diagnóstico

| Problema | Síntomas | Verificar | Solución |
|----------|----------|-----------|----------|
| **WiFi no conecta STA** | Log dice "Fallo" repetido | SSID, contraseña, router disponible | menuconfig → credenciales correctas |
| **INA226 no funciona** | Siempre 0V/0A | Conexión I2C, dirección 0x40 | Revisa cables SDA/SCL |
| **LCD no muestra** | Pantalla en blanco | Dirección 0x27, alimentación | Intenta 5V, ajusta potenciómetro |
| **API muy lenta** | Respuestas lentas | WiFi signal, clientes conectados | Acerca dispositivo al ESP32 |
| **Datos inconsistentes** | Valores saltan | Calibración INA226, ruidoelectromagnético | Usa cable blindado I2C |
| **NTP no sincroniza** | Hora siempre igual | Conexión STA activa, internet disponible | Reinicia después de conectar |

### Comandos de Diagnóstico Útiles

```bash
# Ver logs en tiempo real con filtros
idf.py monitor | grep "WIFI\|INA226\|LCD"

# Compilar con máximo de debugging
idf.py build -- -DCMAKE_BUILD_TYPE=Debug

# Ver configuración actual
idf.py menuconfig (ir a Example Configuration)

# Limpiar build y recompilar
idf.py fullclean && idf.py build

# Monitorear puerto serial con Python
python -m esptool.py --port COM3 monitor
```

---

## 📚 Componentes Utilizados

- **ESP32** (Xtensa dual-core)
- **INA226** - Monitor de potencia I2C de Texas Instruments

---

## � Recursos Técnicos y Referencias

### Documentación Oficial

#### ESP-IDF Framework
- **Repositorio:** https://github.com/espressif/esp-idf
- **Documentación:** https://docs.espressif.com/projects/esp-idf/
- **Versión utilizada:** v6.0.1 LTS

#### INA226 Sensor
- **Datasheet:** Texas Instruments INA226 Bidirectional CURRENT/POWER MONITOR
- **Enlace:** https://www.ti.com/product/INA226
- **I2C Address:** 0x40 (sin pines A0-A3 conectados)
- **Protocolo:** I2C estándar
- **Referencia código:** [ina226.c](main/sources/ina226.c)

#### ESP32 Microcontroller
- **Datasheet:** Espressif ESP32 Technical Reference Manual
- **Enlace:** https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- **Hoja de referencia de pines:** NodeMCU-32S Pinout

#### PCF8574 Expansor I/O
- **Datasheet:** NXP PCF8574 REMOTE 8-BIT I/O EXPANDER
- **Dirección I2C predeterminada:** 0x27
- **Rango de voltaje:** 2.3V - 5.5V
- **Interfaz:** I2C estándar 100-400 kHz

### Librerías y Componentes Utilizados

```
Componente                  Versión    Fuente
────────────────────────────────────────────────────────
FreeRTOS                    v10.4.3    Integrado ESP-IDF
esp_http_server             v1.0       Integrado ESP-IDF
esp_wifi                    v1.0       Integrado ESP-IDF
esp_event                   v1.0       Integrado ESP-IDF
esp_netif                   v1.0       Integrado ESP-IDF
esp_sntp                    v1.0       Integrado ESP-IDF
driver/i2c_master           v1.0       Integrado ESP-IDF
driver/gpio                 v1.0       Integrado ESP-IDF
esp_log                     v1.0       Integrado ESP-IDF
nvs_flash                   v1.0       Integrado ESP-IDF
```

### Tutoriales y Ejemplos Relacionados

#### WiFi Dual Mode
- ESP-IDF → Examples → WiFi → softap_sta
- Manejador de eventos para SoftAP y STA simultáneo
- Configuración de credenciales mediante menuconfig

#### HTTP Server
- ESP-IDF → Examples → Protocols → http_server
- Servidor HTTP embebido
- Servicio de archivos estáticos y endpoints REST

#### I2C Master
- ESP-IDF → Examples → Peripherals → i2c
- Comunicación I2C con múltiples dispositivos
- Detección y escaneo de direcciones

#### SNTP
- ESP-IDF → Examples → System → sntp
- Sincronización de hora mediante NTP
- Configuración de servidores y zonas horarias

### Herramientas de Desarrollo Recomendadas

| Herramienta | Versión | Propósito | Enlace |
|-------------|---------|----------|--------|
| **ESP-IDF** | v6.0.1 | Framework de desarrollo | https://github.com/espressif/esp-idf |
| **Python** | 3.8+ | Herramientas de compilación | https://www.python.org/ |
| **CMake** | 3.20+ | Sistema de compilación | https://cmake.org/ |
| **Ninja** | 1.12+ | Constructor paralelo | https://ninja-build.org/ |
| **Git** | 2.x | Control de versiones | https://git-scm.com/ |
| **VS Code** | Latest | Editor de código | https://code.visualstudio.com/ |
| **Putty/Miniterm** | Latest | Monitor serial | https://www.putty.org/ |

### Configuración Recomendada de Entorno

#### Windows (PowerShell)
```powershell
# Configurar variables de entorno
$env:IDF_PATH = 'C:\esp\v6.0.1\esp-idf'
$env:IDF_TOOLS_PATH = 'C:\Users\[usuario]\.espressif'
$env:PYTHONPATH = "$env:IDF_PATH\tools"

# Agregar a PATH
$env:Path += ";$env:IDF_PATH\tools\bin"

# Verificar instalación
idf.py version
```

#### Linux/macOS (Bash)
```bash
export IDF_PATH=$HOME/esp/esp-idf
export IDF_TOOLS_PATH=$HOME/.espressif
export PYTHONPATH=$IDF_PATH/tools

export PATH=$IDF_PATH/tools/bin:$PATH

# Verificar instalación
idf.py version
```

### Resolución de Problemas de Compilación

#### Error: "CMake not found"
```bash
pip install cmake
# O en Windows: choco install cmake
```

#### Error: "Ninja not found"
```bash
pip install ninja
# O en Windows: choco install ninja
```

#### Error: "Python version"
```bash
# Requerir Python 3.8+
python --version

# O usar Python específico
python3.9 -m idf.py build
```

#### Error: "IDF_PATH not set"
```bash
# Configurar antes de usar idf.py
export IDF_PATH=/ruta/a/esp-idf
# O en Windows:
set IDF_PATH=C:\esp\v6.0.1\esp-idf
```

### Optimizaciones de Rendimiento

#### Velocidad de Compilación
```bash
# Compilación paralela (más rápido)
idf.py build -- -j 4

# Usar cache de compilación
idf.py build --cache-size 1G
```

#### Consumo de Memoria
```
Tamaño de binario típico: ~700 KB comprimido
RAM utilizada en runtime: ~300-400 KB
Flash utilizada: ~1.5 MB (app + datos)
```

#### Optimizaciones de Red
```bash
# Reducir canal WiFi:
idf.py menuconfig
# Example Configuration → WiFi AP Channel → 1 (menos interferencia)

# Aumentar potencia WiFi:
# ESP-IDF → WiFi → TX Power → 20 dBm (máximo)
```

### Frecuentes Preguntas Técnicas (FAQ)

**P: ¿Puedo usar a 5V en lugar de 3.3V?**
A: El ESP32 requiere estrictamente 3.3V. El LCD y PCF8574 pueden funcionar a 5V, pero el ESP32 se dañaría. Usa regulador de voltaje.

**P: ¿Cuántos clientes pueden conectarse al SoftAP?**
A: Máximo 4 clientes simultáneos (configurable). Se puede aumentar en menuconfig pero consume más RAM.

**P: ¿Qué pasa si el sensor INA226 no está conectado?**
A: El sistema continúa funcionando. Los valores del sensor serán 0 y se mostrará un mensaje en logs.

**P: ¿Puedo cambiar el puerto HTTP de 80 a otro?**
A: Sí, en `wifi_manager.c` línea de `httpd_config_t config = HTTPD_DEFAULT_CONFIG();` configura `config.server_port = 8080;`

**P: ¿Cómo reseteó los datos de energía acumulada?**
A: Reinicia el ESP32. También puedes modificar el código para agregar un endpoint POST `/api/reset-energy`.

**P: ¿Es posible agregár más sensores I2C?**
A: Sí. El bus I2C es compartido. Solo necesitas la dirección correcta de cada dispositivo.

---

## �📄 Licencia

Este proyecto es de código abierto. Libre para uso educativo y comercial con atribución.

---

## 🆘 Contacto y Soporte

Para reportar problemas o sugerencias, revisa los logs completos con:
```bash
idf.py monitor --baud 115200
```

**Última actualización:** Junio 2024  
**Versión ESP-IDF:** v6.0.1  
**Versión del código:** 1.0



