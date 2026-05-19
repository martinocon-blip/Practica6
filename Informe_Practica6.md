# Práctica 6 - Bus SPI con ESP32, RFID y Tarjeta SD

## Objetivo

El objetivo de esta práctica es comprender el funcionamiento del bus SPI utilizando el ESP32 junto con dos periféricos diferentes:

- un lector RFID RC522,
- una tarjeta microSD.

Además, se implementa un sistema capaz de:

- detectar tarjetas RFID,
- leer su UID,
- almacenar la información automáticamente en un archivo dentro de la tarjeta SD.

La práctica permite trabajar con múltiples dispositivos SPI compartiendo el mismo bus de comunicación.

---

# Introducción teórica

El bus SPI (Serial Peripheral Interface) es un protocolo de comunicación síncrono de alta velocidad ampliamente utilizado en sistemas embebidos. :contentReference[oaicite:0]{index=0}

SPI utiliza arquitectura maestro-esclavo:

- el ESP32 actúa como maestro,
- los periféricos (RFID y SD) actúan como esclavos.

La comunicación SPI requiere normalmente 4 líneas:

| Línea | Función |
|---|---|
| MOSI | Maestro → Esclavo |
| MISO | Esclavo → Maestro |
| SCK | Señal de reloj |
| SS / CS | Selección de dispositivo |

El ESP32 controla qué dispositivo SPI está activo mediante las líneas `SS` o `CS`. :contentReference[oaicite:1]{index=1}

---

# Ventajas del bus SPI

- Alta velocidad de comunicación.
- Comunicación Full Duplex.
- Baja complejidad hardware.
- Compatible con gran cantidad de dispositivos.

---

# Desventajas del bus SPI

- Requiere varios cables.
- Necesita una línea CS independiente por periférico.
- No incorpora control de errores.

---

# Material utilizado

- ESP32-S3-DevKitC-1
- Módulo RFID RC522
- Módulo microSD SPI
- Tarjeta microSD
- Visual Studio Code
- PlatformIO
- Framework Arduino

---

# Configuración hardware

## Pines RFID

| Señal | GPIO |
|---|---|
| SS | GPIO9 |
| RST | GPIO8 |
| SCK | GPIO36 |
| MISO | GPIO37 |
| MOSI | GPIO35 |

---

# Pines SD

| Señal | GPIO |
|---|---|
| CS | GPIO10 |
| SCK | GPIO12 |
| MISO | GPIO13 |
| MOSI | GPIO11 |

---

# Código implementado

## Archivo `main.cpp`

```cpp
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>

// ---------- PINES RFID ----------
#define SS_RFID 9
#define RST_PIN 8

// ---------- PINES SD ----------
#define CS_SD 10

// ---------- OBJETOS ----------
MFRC522 mfrc522(SS_RFID, RST_PIN);
SPIClass spiSD(HSPI);

// ---------- CONTROL SD ----------
bool sdDisponible = false;

void setup() {

  Serial.begin(115200);

  delay(1000);

  // ---------- RFID ----------
  SPI.begin(36, 37, 35, SS_RFID);

  mfrc522.PCD_Init();

  Serial.println("RFID listo");

  byte version = mfrc522.PCD_ReadRegister(
    mfrc522.VersionReg
  );

  Serial.print("Version RFID: ");

  Serial.println(version, HEX);

  // ---------- SD ----------
  spiSD.begin(12, 13, 11, CS_SD);

  if (!SD.begin(CS_SD, spiSD)) {

    Serial.println("Error inicializando SD");

    sdDisponible = false;

  } else {

    Serial.println("SD OK");

    sdDisponible = true;
  }

  Serial.println("Acerca una tarjeta...");
}

void loop() {

  // Detectar tarjeta
  if (!mfrc522.PICC_IsNewCardPresent()) return;

  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Mostrar UID
  Serial.print("UID: ");

  for (byte i = 0; i < mfrc522.uid.size; i++) {

    if (mfrc522.uid.uidByte[i] < 0x10)
      Serial.print("0");

    Serial.print(
      mfrc522.uid.uidByte[i],
      HEX
    );

    Serial.print(" ");
  }

  Serial.println();

  // Guardar en SD
  if (sdDisponible) {

    File file = SD.open(
      "/log.txt",
      FILE_APPEND
    );

    if (file) {

      file.print("Tiempo(ms): ");

      file.print(millis());

      file.print(" - UID: ");

      for (byte i = 0; i < mfrc522.uid.size; i++) {

        if (mfrc522.uid.uidByte[i] < 0x10)
          file.print("0");

        file.print(
          mfrc522.uid.uidByte[i],
          HEX
        );

        file.print(" ");
      }

      file.println();

      file.close();

      Serial.println("Guardado en SD");

    } else {

      Serial.println("Error escribiendo en SD");
    }

  } else {

    Serial.println("SD no disponible");
  }

  mfrc522.PICC_HaltA();

  delay(1000);
}
```

---

# Funcionamiento del programa

El sistema trabaja utilizando dos periféricos SPI diferentes:

- el lector RFID,
- la tarjeta SD.

Durante el `setup()`:

1. Se inicializa el puerto serie.
2. Se inicializa el lector RFID.
3. Se comprueba la versión del chip RC522.
4. Se inicializa la tarjeta SD.
5. Se informa del estado del sistema.

---

# Lectura RFID

En el `loop()`:

```cpp
mfrc522.PICC_IsNewCardPresent()
```

comprueba si existe una tarjeta cercana.

Posteriormente:

```cpp
mfrc522.PICC_ReadCardSerial()
```

lee el UID de la tarjeta.

El UID se muestra por puerto serie en formato hexadecimal.

---

# Escritura en tarjeta SD

Cuando la SD está disponible:

```cpp
SD.open("/log.txt", FILE_APPEND)
```

abre el archivo `log.txt`.

El programa almacena:

- el tiempo desde el arranque (`millis()`),
- el UID de la tarjeta detectada.

Cada lectura queda registrada automáticamente.

---

# Explicación del uso de múltiples SPI

En este proyecto se utilizan dos dispositivos SPI diferentes.

Cada periférico dispone de:

- una línea CS independiente,
- configuración SPI propia.

Esto permite compartir el mismo protocolo SPI sin conflictos entre dispositivos.

El ESP32 selecciona automáticamente qué periférico está activo mediante:

- `SS_RFID`
- `CS_SD`

---

# Explicación de funciones importantes

## SPI.begin()

```cpp
SPI.begin()
```

Inicializa el bus SPI para el lector RFID.

---

## SPIClass

```cpp
SPIClass spiSD(HSPI);
```

Permite crear un segundo bus SPI independiente para la SD.

---

## PCD_Init()

```cpp
mfrc522.PCD_Init();
```

Inicializa el módulo RFID RC522.

---

## SD.begin()

```cpp
SD.begin()
```

Inicializa la comunicación con la tarjeta SD.

---

## millis()

```cpp
millis()
```

Devuelve el tiempo transcurrido desde el arranque del ESP32.

---

# Salida observada por puerto serie

Ejemplo de salida:

```text
RFID listo
Version RFID: 92
SD OK
Acerca una tarjeta...

UID: 93 A4 1F 2B
Guardado en SD

UID: 5A 3C 91 D0
Guardado en SD
```

---

# Contenido generado en log.txt

```text
Tiempo(ms): 12543 - UID: 93 A4 1F 2B
Tiempo(ms): 18210 - UID: 5A 3C 91 D0
```

---

# Aplicaciones reales

Este sistema puede utilizarse en:

- control de accesos,
- registro de asistencia,
- identificación de usuarios,
- sistemas domóticos,
- inventario inteligente,
- sistemas IoT.

---

# Ventajas del sistema implementado

- Lectura RFID rápida.
- Almacenamiento persistente en SD.
- Comunicación SPI eficiente.
- Capacidad de registrar múltiples tarjetas automáticamente.

---

# Conclusiones

En esta práctica se ha aprendido:

- El funcionamiento del bus SPI.
- Cómo conectar múltiples periféricos SPI al ESP32.
- Cómo utilizar un lector RFID RC522.
- Cómo leer el UID de tarjetas RFID.
- Cómo almacenar información en una tarjeta microSD.
- Cómo gestionar diferentes buses SPI simultáneamente.

Además, se ha comprobado la capacidad del ESP32 para trabajar con varios dispositivos SPI de forma concurrente utilizando líneas CS independientes.