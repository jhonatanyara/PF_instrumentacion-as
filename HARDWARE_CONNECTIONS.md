# Guía de Conexiones de Hardware

Documentación detallada sobre cómo conectar correctamente todos los componentes del sistema de instrumentación DC.

---

## 📌 Índice

1. [Resumen de conexiones](#resumen-de-conexiones)
2. [Conexión de la fuente DC](#conexión-de-la-fuente-dc)
3. [Conexión del sensor INA226](#conexión-del-sensor-ina226)
4. [Conexión del display LCD](#conexión-del-display-lcd)
5. [Conexión del ESP32](#conexión-del-esp32)
6. [Diagrama eléctrico completo](#diagrama-eléctrico-completo)
7. [Lista de componentes recomendados](#lista-de-componentes-recomendados)
8. [Verificación de continuidad](#verificación-de-continuidad)

---

## Resumen de Conexiones

### Vista General del Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                    FUENTE DE ALIMENTACIÓN DC                    │
│                      (0V - 32V, 0A - 20A)                      │
└────────┬──────────────────────────────────────┬─────────────────┘
         │                                      │
         │ (+) Línea Positiva                   │ (-) Línea Negativa (GND)
         │                                      │
    ┌────▼──────────────────────────┐      ┌────▼─────────┐
    │   INA226 (Monitor de Potencia)│      │              │
    │   - Mide voltaje              │      │              │
    │   - Mide corriente            │      │              │
    │   - Calcula potencia          │      │ Carga a medir │
    └────┬──────────────────────────┘      │              │
         │                                  │              │
         └──────────────┬───────────────────┘              │
                        │                                  │
                    ┌───▼──────────────────────────────────▼─────┐
                    │                                            │
                    │  GND COMÚN (Todas las fuentes)            │
                    │                                            │
                    └────────────┬─────────────────────────────────┘
                                 │
                    ┌────────────▼──────────────┐
                    │    ESP32 + Periféricos    │
                    │    (Alimentación 3.3V)    │
                    └───────────────────────────┘
```

---

## Conexión de la Fuente DC

### Requisitos de la Fuente

- **Voltaje de entrada:** 0V a 32V DC
- **Corriente máxima:** Depende de la carga a medir
- **Precisión recomendada:** ±1% o mejor
- **Regulación de carga:** ±2% (deseable)
- **Tipo:** Fuente conmutada o lineal (sin importa para medición)

### Seguridad Recomendada

```
[Fusible 25A] ─── [Protección térmica] ─── [Interruptor] ─── [Fuente DC]
       │                    │                     │
   Protección          Sobrecarga            Control manual
```

### Etapas de Conexión

#### Paso 1: Identificar polaridad
```
Fuente DC típica:
    Rojo   = Positivo (+)
    Negro  = Negativo/Tierra (-)
```

#### Paso 2: Aislamiento inicial
```
1. APAGAR la fuente de alimentación
2. DESCONECTAR todos los cables
3. ESPERAR 5 segundos
4. Verificar con multímetro que no hay tensión residual
```

#### Paso 3: Conexión del positivo
```
Fuente DC (+) ──────────┬─────────────────────── INA226 IN+
                        │
                   [Fusible 25A] (Recomendado)
```

#### Paso 4: Conexión de carga
```
INA226 IN- ───────────────────────── Carga (Resistencia, Motor, etc.)
                                        │
                                        │
                                        ▼
                                    (Carga activa)
                                        │
                                        │
Fuente DC (-) ─────────┬──────────────── Retorno de carga
                       │
                    [GND Común]
```

#### Paso 5: Conexión de GND común
```
Fuente DC (-)  ───┐
                   │
INA226 GND     ────┼── [Punto de unión]
                   │
ESP32 GND      ────┘

IMPORTANTE: Todos los negativos deben estar conectados al mismo punto
            (No hacer conexiones en serie de GND)
```

---

## Conexión del Sensor INA226

### Ubicación en el Circuito

El INA226 debe colocarse **en serie** con la carga a medir:

```
Fuente (+) ───► INA226 IN+ ──[Shunt interno]──► INA226 IN- ───► Carga
                                                                  │
                                        Fuente (-) ◄─────────────┘
```

### Pines del INA226

```
┌─────────────────────────┐
│      INA226 Module      │
│                         │
│  ┌───────────────────┐  │
│  │  Sensor Izquierda │  │
│  │                   │  │
│  │ IN+ ─────┐        │  │
│  │ IN- ─────┤        │  │
│  │ VCC ─────┤ (Lado │  │
│  │ GND ─────┤  de   │  │
│  │ SCL ─────┤  Power)  │
│  │ SDA ─────┘        │  │
│  └───────────────────┘  │
│                         │
│  A0  A1  A2  A3 = GND   │
│  (Dirección I2C = 0x40) │
└─────────────────────────┘
```

### Tabla de Conexión del INA226

| Pin INA226 | Conexión | Voltaje |
|-----------|----------|---------|
| **IN+** | Línea positiva de entrada | Variable (0-36V) |
| **IN-** | Salida a carga (retorno negativo) | Variable (0-36V) |
| **VCC** | Alimentación del circuito integrado | 3.3V ✓ |
| **GND** | Tierra/GND común | 0V |
| **SCL** | GPIO 22 (Reloj I2C) | 3.3V |
| **SDA** | GPIO 21 (Datos I2C) | 3.3V |
| **A0-A3** | Selectores de dirección | GND (0x40) |

### Características del Shunt Interno

```
El INA226 tiene un shunt interno de precisión:

Shunt Value: 0.1Ω (típico)
Max Current: ±20A

Por ley de Ohm: V_shunt = I × R
                V_shunt = 20A × 0.1Ω = 2V (máximo)
                
El INA226 mide la caída de tensión en el shunt
y calcula la corriente: I = V_shunt / 0.1Ω
```

### Rango de Medición

| Parámetro | Rango | Resolución |
|-----------|-------|-----------|
| Voltaje de entrada | 0 - 36V | 1.25 mV/LSB |
| Corriente | ±20A | ±1 mA/LSB |
| Potencia | 0 - 720W | 25 mW/LSB |

---

## Conexión del Display LCD

### Módulo I2C PCF8574 + LCD

El display no se alimenta directamente de la fuente DC, sino del ESP32:

```
┌─────────────────────────────────┐
│    LCD 16x2 + PCF8574 Module    │
│                                 │
│  ┌─────────────────────────┐   │
│  │      LCD Display        │   │
│  │                         │   │
│  │ [Pantalla 16x2]         │   │
│  │                         │   │
│  └─────────────────────────┘   │
│                                 │
│  Pines del Módulo PCF8574:      │
│  ┌──────────────────────────┐  │
│  │ GND  → GND del ESP32     │  │
│  │ VCC  → 5V o 3.3V*       │  │
│  │ SDA  → GPIO 21          │  │
│  │ SCL  → GPIO 22          │  │
│  │ A0-2 → GND (Addr 0x27)  │  │
│  └──────────────────────────┘  │
└─────────────────────────────────┘

* Si no funciona con 3.3V, usar 5V
```

### Conexión Física Recomendada

```
[ESP32]                              [LCD + PCF8574]
 │                                        │
 ├─ GPIO 21 (SDA) ────────────────────── SDA
 │                                        │
 ├─ GPIO 22 (SCL) ────────────────────── SCL
 │                                        │
 ├─ 3.3V (o 5V*) ──────────────────────── VCC
 │                                        │
 └─ GND ────────────────────────────────── GND

* Prueba primero con 3.3V. Si no funciona, cambiar a 5V
```

### Ajustes del LCD

Una vez conectado, el LCD debería mostrar caracteres. Si no aparece nada:

1. **Ajusta el contraste:** Potenciómetro en la parte trasera
   - Gira lentamente hasta ver caracteres
   - Posición típica: 50% del recorrido

2. **Verifica la iluminación (backlight):** 
   - Debe estar iluminado (LED azul o verde típicamente)
   - Si no está iluminado, revisar conexión de alimentación

3. **Prueba de comunicación I2C:**
   ```bash
   idf.py monitor
   # Deberías ver mensajes de inicialización del LCD
   ```

---

## Conexión del ESP32

### Pines Principales Utilizados

```
┌──────────────────────────────────┐
│          ESP32 / NodeMCU         │
│                                  │
│  POWER PINS:                     │
│  ├─ VIN (5V entrada)             │
│  ├─ 3V3 (3.3V salida)            │
│  └─ GND (Tierra)                 │
│                                  │
│  I2C PINS:                       │
│  ├─ GPIO 21 (SDA)                │
│  ├─ GPIO 22 (SCL)                │
│  └─ GND (Común)                  │
│                                  │
│  INPUT PINS:                     │
│  ├─ GPIO 32 (Estado 1)           │
│  ├─ GPIO 33 (Estado 2)           │
│  └─ GND (Referencia)             │
│                                  │
│  UART PINS (Monitoreo):          │
│  ├─ GPIO 1 (TX)                  │
│  └─ GPIO 3 (RX)                  │
└──────────────────────────────────┘
```

### Conexión de Alimentación del ESP32

```
┌─────────────────────────────────┐
│   Fuente auxiliar 5V (USB)       │
│                                  │
│   Ejemplos:                      │
│   - Puerto USB de computadora    │
│   - Power bank USB 5V            │
│   - Regulador LDO 5V             │
└────────┬──────────────────────────┘
         │
         │ USB 5V
         │
    ┌────▼──────────┐
    │ ESP32 / Node  │
    │                │
    │ VIN  ◄────────┤ (5V entrada)
    │ GND  ◄────────┤ (GND)
    │ 3V3  ├────────┤ (3.3V salida - Regulador interno)
    │      │        │
    │ GPIO 21 ◄─────┤ (SDA - a sensor + LCD)
    │ GPIO 22 ◄─────┤ (SCL - a sensor + LCD)
    │ GPIO 32 ◄─────┤ (Entrada 1)
    │ GPIO 33 ◄─────┤ (Entrada 2)
    │      │        │
    └──────┼────────┘
           │
        GND COMÚN
```

### Pinout Detallado del ESP32 (NodeMCU-32S)

```
                         ┌─────────────────────────────┐
                         │     ESP32 DEVKIT TOP        │
                         │     (Vista Superior)        │
                         │                             │
        USB             │                             │
    ┌──────┐            │ GND  RST  TX0  RX0  3V3 EN  │
    │ PWR  │───────────→│  │    │    │    │    │  │    │
    └──────┘            │  ├────┴────┴────┴────┴──┤    │
                        │  │                      │    │
      (lado USB)        │  │     CHIP ESP32       │    │
                        │  │                      │    │
                        │  ├────┬────┬────┬────┬──┤    │
                        │ D4 D5 D18 D19 D20 D21 D22 D23│
                        │                             │
                        │ D12 D13 D14 D27 D25 D26 D32 D33│
                        │                             │
                        │ D34 D35 D39 D36 GND 3V3 D0  D1 │
                        │                             │
                        └─────────────────────────────┘
```

#### **Pines de Potencia**
```
┌─────────────────────────────────┐
│     PINES DE ALIMENTACIÓN       │
├─────────────────────────────────┤
│ Pin   │ Nombre   │ Función     │ Voltaje
├───────┼──────────┼─────────────┼─────────
│  1    │ GND      │ Tierra      │   0V
│  2    │ 3V3      │ Salida 3.3V │  3.3V
│  3    │ EN       │ Enable      │  3.3V
│  4    │ VIN      │ Entrada 5V  │  5V
└─────────────────────────────────┘
```

#### **Pines GPIO Principales para este Proyecto**
```
┌──────────────────────────────────────────────────────┐
│           PINES I2C (Bus Compartido)                 │
├──────────────────────────────────────────────────────┤
│ Pin   │ GPIO │ Función │ Dir. I2C │ Velocidad
├───────┼──────┼─────────┼──────────┼──────────
│  23   │ 21   │ SDA     │ 100kHz   │ Típica
│  22   │ 22   │ SCL     │ 100kHz   │ Típica
│       │      │ (ambos) │ 400kHz   │ Máxima
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│        PINES GPIO (Entradas Digitales)               │
├──────────────────────────────────────────────────────┤
│ Pin   │ GPIO │ Función │ Pull-up │ Rango
├───────┼──────┼─────────┼─────────┼─────────────
│  8    │ 32   │ Estado1 │ Interno │ 0-3.3V
│  9    │ 33   │ Estado2 │ Interno │ 0-3.3V
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│        PINES UART (Monitoreo / Sin usar)             │
├──────────────────────────────────────────────────────┤
│ Pin   │ GPIO │ Función │ Nivel   │ Uso
├───────┼──────┼─────────┼─────────┼─────────────
│  41   │ 1    │ TX      │ 3.3V    │ Disponible
│  42   │ 3    │ RX      │ 3.3V    │ Disponible
└──────────────────────────────────────────────────────┘
```

#### **Orden de Pines del ESP32 (Lado Izquierdo - USB a arriba)**
```
Lado Izquierdo         Lado Derecho
──────────────         ─────────────
GND  ──────────────────┐    ┌──────── EN
TX0  ──────────────────┤    ├──────── 3V3
RX0  ──────────────────┤    ├──────── GND
3V3  ──────────────────┤    ├──────── D23
EN   ──────────────────┤    ├──────── D22
D4   ──────────────────┤    ├──────── D21
D5   ──────────────────┤    ├──────── D20
D18  ──────────────────┤    ├──────── D19
D19  ──────────────────┤    ├──────── D18
D20  ──────────────────┤    ├──────── D5
D21  ──────────────────┤    ├──────── D4
D22  ──────────────────┤    ├──────── EN
D23  ──────────────────┤    ├──────── 3V3
GND  ──────────────────┤    ├──────── GND
3V3  ──────────────────┤    ├──────── D1
D12  ──────────────────┤    ├──────── D0
D13  ──────────────────┤    ├──────── D36
D14  ──────────────────┤    ├──────── D39
D27  ──────────────────┤    ├──────── D35
D25  ──────────────────┤    ├──────── D34
D26  ──────────────────┤    ├──────── GND
D32  ──────────────────┤    ├──────── 3V3
D33  ──────────────────┘    └──────── (fin)
```

### Especificaciones Eléctricas de los Pines GPIO

```
┌──────────────────────────────────┐
│   PARÁMETROS DE LOS PINES GPIO   │
├──────────────────────────────────┤
│ Voltaje de operación:    3.3V
│ Rango de entrada:        0V - 3.3V
│ Umbral de alto (HIGH):   2.4V - 3.3V
│ Umbral de bajo (LOW):    0V - 0.8V
│ Zona gris (transición):  0.8V - 2.4V
│                                  │
│ Corriente máx. por pin:  40 mA
│ Corriente total GPIO:    1.2 A
│ Impedancia pull-up int:  ~45kΩ
│ Impedancia pull-down int: ~45kΩ
│                                  │
│ Capacidad ESD:           ±4 kV
│ Frecuencia máxima:       ~1 MHz
└──────────────────────────────────┘
```

### Pull-ups para I2C

El ESP32 tiene pull-ups internos configurables. La mayoría de módulos I2C también incluyen pull-ups, por lo que generalmente **no son necesarios** resistores externos.

**Configuración típica:**
- Pull-ups internos: HABILITADOS
- Valor: ~45kΩ interno del ESP32
- Adicionales: ~10kΩ en INA226 y LCD (integrados en los módulos)

Si experimentas problemas de comunicación I2C, puedes agregar resistores externos de 4.7kΩ:

```
    3.3V
      │
   [4.7kΩ]       [4.7kΩ]
      │              │
    GPIO21 (SDA)   GPIO22 (SCL)
      │              │
     ●──────────────●  (Bus I2C compartido)
      │              │
    GND            GND

Ubicación: Cerca de los pines GPIO del ESP32
Nota: Solo agregar si hay problemas de comunicación
```

### Capacidades Especiales de los Pines

```
GPIO 21 (SDA):
├─ Protocolo: I2C (Open-Drain)
├─ Pull-up: Habilitado (interno ~45kΩ)
├─ Velocidad I2C: Hasta 400 kHz
└─ Dispositivos: INA226, LCD16x2+PCF8574

GPIO 22 (SCL):
├─ Protocolo: I2C (Open-Drain)
├─ Pull-up: Habilitado (interno ~45kΩ)
├─ Velocidad I2C: Hasta 400 kHz
└─ Dispositivos: INA226, LCD16x2+PCF8574

GPIO 32 (Estado 1):
├─ Modo: Entrada digital
├─ Pull-up: Habilitado (interno ~45kΩ)
├─ Rango: 0-3.3V
└─ Uso: Monitoreo de estado externo

GPIO 33 (Estado 2):
├─ Modo: Entrada digital
├─ Pull-up: Habilitado (interno ~45kΩ)
├─ Rango: 0-3.3V
└─ Uso: Monitoreo de estado externo
```

---

## Diagrama Eléctrico Completo

### Circuito de Medición

```
┌─────────────────────────────────────────────────────────────────┐
│                     FUENTE DC 0-32V                             │
└────────┬────────────────────────────────────────────┬───────────┘
         │                                            │
         │ (+)                                        │ (-)
         │                                            │
    ┌────▼──────────────┐                       ┌────▼───────┐
    │    [Fusible 25A]  │                       │             │
    └────┬──────────────┘                       │ GND COMÚN   │
         │                                      │             │
         │                                      │  ┌──────────┘
    ┌────▼─────────────────────────┐        │  │
    │   INA226 Sensor              │        │  │
    │  [Power monitoring IC]       │        │  │
    │                              │        │  │
    │ IN+ ───────────────────┐   │        │  │
    │ IN- ───────┬──────────┤   │        │  │
    │ VCC ───┐   │         │   │        │  │
    │ GND ───┼───┼─────────┴───┼────────┘  │
    │ SCL ───┼───│ I2C        │            │
    │ SDA ───┼───│ Bus        │            │
    └────────┼───┼────────────┼─────────────┘
             │   │            │
             │   │        ┌───▼─────────┐
             │   │        │ Load/Carga  │
             │   │        │ (0-20A)     │
             │   │        └───┬─────────┘
             │   │            │
             │   └────────────┴────────── (Retorno a -DC)
             │
        ┌────▼──────────────────────┐
        │    ESP32 / NodeMCU        │
        │                           │
        │ 3V3  ─────────────┐      │
        │ GND  ─────────────┼───┬──┤  GND COMÚN
        │ GPIO 21 (SDA) ────┘   │  │
        │ GPIO 22 (SCL) ────────┘  │
        │ GPIO 32 ─────────────────┤  (Entrada opcional)
        │ GPIO 33 ─────────────────┤  (Entrada opcional)
        │                           │
        └───────────────────────────┘
               │
               │ USB 5V (desde computadora)
               └─────────────────────────
```

### Circuito I2C

```
         GPIO 21          GPIO 22
         (SDA)            (SCL)
         │                 │
         │                 │
    ┌────┴────┐       ┌────┴────┐
    │   |SDA| │       │   |SCL| │
    └────┬────┘       └────┬────┘
         │                 │
         │                 │
    ┌────┴─────────────────┴───┐
    │    I2C Bus (100-400 kHz)  │
    └─┬──────────────────────┬──┘
      │                      │
      │                      │
  ┌───▼──────┐          ┌───▼──────┐
  │ INA226   │          │ PCF8574+ │
  │0x40      │          │  LCD16x2 │
  │Sensor    │          │   0x27   │
  └──────────┘          └──────────┘
```

---

## Cableado Recomendado

### Especificación de Cables por Función

```
┌──────────────────────────────────────────────────────┐
│           CABLES I2C (SDA / SCL)                     │
├──────────────────────────────────────────────────────┤
│ Tipo:           Twisted Pair (par trenzado)
│ Gauge:          AWG 22 - 24 (0.33 - 0.51 mm²)
│ Longitud máx:   1 metro (@ 100 kHz)
│ Longitud máx:   0.5 metros (@ 400 kHz)
│ Color típico:   Blanco/Azul o Blanco/Verde
│ Impedancia:     100-120Ω
│ Capacidad:      <100pF/metro
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│        CABLES DE POTENCIA (+ / GND / Medida)         │
├──────────────────────────────────────────────────────┤
│ Tipo:           Cable sólido o flexible
│ Gauge:          AWG 18 - 20 (0.82 - 1.04 mm²)
│ Longitud máx:   3 metros (sin restricción)
│ Color:          Rojo (+), Negro (-)
│ Corriente máx:  15-20A (depende del calibre)
│ Aislamiento:    600V (mínimo)
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│      CABLES GPIO ENTRADA (Estado 1 / Estado 2)       │
├──────────────────────────────────────────────────────┤
│ Tipo:           Blindado (shield) recomendado
│ Gauge:          AWG 22 - 24 (0.33 - 0.51 mm²)
│ Longitud máx:   2 metros (sin blindaje)
│ Longitud máx:   5 metros (con blindaje)
│ Color:          Cualquiera (identifica con etiqueta)
│ Blindaje:       Conectar a GND en ESP32
└──────────────────────────────────────────────────────┘
```

### Conectores Recomendados

```
Para conexiones reutilizables:

┌─ I2C (SDA/SCL):
│  └─ Conectores JST-PH 2.54mm (microfit)
│     Código: JST-2P
│     Color típico: Rojo (+), Negro (-)
│
├─ Potencia (hasta 15A):
│  └─ Conectores XT60 (high-current)
│     Código: XT60
│     Capacidad: Hasta 60A
│
└─ GPIO entrada:
   └─ Conectores DuPont 2.54mm
      Código: DuPont-1P
      Capacidad: Hasta 3A
```

---

## Diagrama de Flujo de Conexión

```
START
 │
 ├─→ [1. Preparar herramientas]
 │    ├─ Multímetro
 │    ├─ Cables y conectores
 │    └─ Fuente 5V (USB)
 │
 ├─→ [2. Verificar continuidad] ✓
 │    ├─ GND: Fuente → INA226 → ESP32 → LCD
 │    ├─ I2C: GPIO21 ↔ SDA, GPIO22 ↔ SCL
 │    └─ Entradas: GPIO32, GPIO33
 │
 ├─→ [3. Conectar alimentación ESP32]
 │    ├─ USB 5V → VIN
 │    └─ GND → GND común
 │
 ├─→ [4. Conectar I2C]
 │    ├─ GPIO21 → SDA (INA226 + LCD)
 │    ├─ GPIO22 → SCL (INA226 + LCD)
 │    └─ 3V3 → VCC (INA226 + LCD)
 │
 ├─→ [5. Conectar entradas GPIO]
 │    ├─ GPIO32 → Sensor estado 1
 │    └─ GPIO33 → Sensor estado 2
 │
 ├─→ [6. Cargar firmware]
 │    └─ idf.py flash monitor
 │
 ├─→ [7. Verificar comunicación I2C]
 │    └─ Buscar mensajes de inicialización
 │
 ├─→ [8. Conectar fuente DC (sin carga)]
 │    ├─ Verificar voltaje en INA226
 │    └─ Verificar lecturas en LCD
 │
 └─→ [9. Conectar carga]
      └─ Monitorear corriente y potencia
      
END (Sistema operativo)
```

### Componentes Esenciales

| Componente | Modelo/Rango | Cantidad | Notas |
|-----------|--------------|----------|-------|
| ESP32 | NodeMCU-32S o Dev Kit | 1 | Microcontrolador principal |
| INA226 | Texas Instruments | 1 | Sensor de potencia I2C |
| LCD 16x2 | Genérico | 1 | Display LCD 16 caracteres x 2 líneas |
| PCF8574 | I2C expander | 1 | Adaptador I2C para LCD |
| Fuente DC | 0-32V regulada | 1 | Alimentación de carga a medir |
| Fuente 5V | USB o regulador | 1 | Alimentación del ESP32 |

### Componentes de Protección (Recomendados)

| Componente | Rango | Cantidad | Función |
|-----------|-------|----------|---------|
| Fusible | 25A | 1 | Protección de línea positiva |
| Disyuntor térmico | 25A | 1 | Protección por sobrecarga |
| Condensadores 100nF | - | 3 | Desacoplamiento (VCC de cada IC) |
| Resistencias 4.7kΩ | - | 2 | Pull-ups I2C (si es necesario) |
| Cable AWG 18 | - | 2m | Cables de potencia (positivo/negativo) |
| Cable UTP | - | 0.5m | Cables I2C (SDA/SCL) |

---

## Verificación de Continuidad

### Lista de Verificación Previa

Antes de conectar la fuente, verifica con un multímetro en modo de continuidad (beeper):

```
□ Continuidad en GND:
  - Fuente (-) a INA226 GND           ✓
  - INA226 GND a ESP32 GND            ✓
  - ESP32 GND a LCD GND               ✓

□ Continuidad en I2C:
  - ESP32 GPIO 21 a INA226 SDA        ✓
  - ESP32 GPIO 22 a INA226 SCL        ✓
  - ESP32 GPIO 21 a LCD SDA           ✓
  - ESP32 GPIO 22 a LCD SCL           ✓

□ No hay cortocircuitos:
  - Entre VCC y GND                   ✗ (debe marcar infinito)
  - Entre SDA y GND                   ✗ (debe marcar infinito)
  - Entre SCL y GND                   ✗ (debe marcar infinito)

□ Alimentación del ESP32:
  - VIN del ESP32 a 5V                ✓
  - GND del ESP32 a GND común         ✓
```

### Procedimiento de Prueba

1. **Desconectar todo** de la fuente DC
2. **Multímetro en modo continuidad**
3. **Probar cada conexión** con los puntos de prueba en la punta del probador
4. **Si hay un beep**, hay continuidad (bueno para GND, malo para VCC-GND)
5. **Si no hay beep**, verificar conexión

---

## Checklist Final Antes de Encender

- [ ] Todos los cables están conectados firmemente
- [ ] No hay pines doblados en los módulos
- [ ] La fuente DC está APAGADA
- [ ] Se ha verificado la continuidad
- [ ] No hay cortocircuitos evidentes
- [ ] El ESP32 está correctamente alimentado (USB conectado)
- [ ] Se ha verificado la polaridad de la fuente DC
- [ ] Los pines A0-A3 del INA226 están conectados a GND (dirección 0x40)
- [ ] Los pines de dirección del PCF8574 están en GND (dirección 0x27)
- [ ] Se ha subido el firmware al ESP32 con `idf.py flash`

---

## Solución de Problemas de Conexión

### Problema: LCD no muestra nada

**Causas posibles:**
- Contraste ajustado incorrectamente
- Dirección I2C incorrecta (no es 0x27)
- Alimentación insuficiente (VCC bajo)
- Cable I2C suelto

**Solución:**
```bash
# Ver direcciones I2C disponibles
idf.py monitor
# Buscar mensajes de escaneo I2C
```

### Problema: Sensor INA226 no responde

**Causas posibles:**
- Cables I2C invertidos
- Dirección I2C incorrecta (no es 0x40)
- Voltage en VCC del sensor muy bajo

**Verificar:**
- Medir 3.3V en VCC del INA226
- Revisar continuidad de SDA/SCL

### Problema: ESP32 no se comunica con sensor

**Causas posibles:**
- Bus I2C no inicializado
- Velocidad I2C demasiado alta
- Pines GPIO configurados incorrectamente

**Solución:**
```bash
idf.py menuconfig
# Verificar: I2C SDA pin = 21, I2C SCL pin = 22
```

---

**Última actualización:** Junio 2024  
**Versión:** 1.0
