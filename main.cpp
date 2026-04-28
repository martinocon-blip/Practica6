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

  // ---------- INICIALIZACIÓN RFID ----------
  SPI.begin(36, 37, 35, SS_RFID); // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();

  Serial.println("RFID listo");

  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("Version RFID: ");
  Serial.println(version, HEX);

  // ---------- INICIALIZACIÓN SD ----------
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

  // ---------- DETECCIÓN TARJETA ----------
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // ---------- MOSTRAR UID ----------
  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // ---------- GUARDAR EN SD ----------
  if (sdDisponible) {

    File file = SD.open("/log.txt", FILE_APPEND);

    if (file) {
      file.print("Tiempo(ms): ");
      file.print(millis());
      file.print(" - UID: ");

      for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) file.print("0");
        file.print(mfrc522.uid.uidByte[i], HEX);
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

  // ---------- FINALIZAR ----------
  mfrc522.PICC_HaltA();

  delay(1000);
}