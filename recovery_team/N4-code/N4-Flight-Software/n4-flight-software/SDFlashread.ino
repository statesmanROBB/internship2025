#include <SPI.h>
#include <SD.h>
#include <CustomSerialFlash.h>

// === Pin Configuration ===
#define FLASH_CS   5
#define FLASH_WP   255
#define FLASH_HOLD 255
#define SD_CS      26

#define SPI_SCK    18
#define SPI_MISO   19
#define SPI_MOSI   23

const char* filename ="flight_data.txt";
const char* filename_flash ="flight_data.txt";

//List all files in SD-card
void listSDCardFiles(const char* dirPath = "/", uint8_t numTabs = 0) {
  File root = SD.open(dirPath);
  if (!root) {
    Serial.println("❌ Failed to open directory!");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("❌ Not a directory!");
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    for (uint8_t i = 0; i < numTabs; i++) Serial.print('\t');

    if (file.isDirectory()) {
      Serial.print("📁 ");
      Serial.println(file.name());
      listSDCardFiles(file.name(), numTabs + 1);  // Recursive listing
    } else {
      Serial.print("📄 ");
      Serial.print(file.name());
      Serial.print("\t\t");
      Serial.print(file.size());
      Serial.println(" bytes");
    }

    file = root.openNextFile();
  }

  root.close();
}


void disableAllDevices() {
  digitalWrite(FLASH_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set CS pins
  pinMode(FLASH_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  disableAllDevices();

  // Initialize SPI with custom pins
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // === Initialize and Read from Flash ===
  Serial.println("📦 Initializing SPI Flash...");
  if (!SerialFlash.begin(FLASH_CS, FLASH_WP, FLASH_HOLD)) {
    Serial.println("❌ Flash init failed!");
  } else {
    Serial.println("✅ Flash initialized.");
    SerialFlash.listFiles();
    SerialFlashFile file = SerialFlash.open(filename_flash);
    if (!file) {
      Serial.println("❌ File not found on Flash.");
    } else {
      Serial.printf("📄 Reading %s from Flash (%lu bytes):\n", filename_flash, file.size());

      const uint16_t BUFFER_SIZE = 64;
      char buffer[BUFFER_SIZE];

      for (uint32_t i = 0; i < file.size(); i += BUFFER_SIZE) {
        uint32_t bytesToRead = min((uint32_t)BUFFER_SIZE, file.size() - i);
        file.read(i, buffer, bytesToRead);

        for (uint16_t j = 0; j < bytesToRead; j++) {
          if ((uint8_t)buffer[j] != 0xFF) {
            Serial.print(buffer[j]);
          }
        }
      }

      file.close();
      Serial.println("\n✅ Done reading from Flash.");
    }
  }

  // === Initialize and Read from SD Card ===
  Serial.println("\n📀 Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD card initialization failed!");
  } else {
    Serial.println("✅ SD card initialized.");
    
    listSDCardFiles();
    Serial.println("✅ \Listed all files");
    File sdFile = SD.open("/" + String(filename), FILE_READ);
    if (!sdFile) {
      Serial.println("❌ File not found on SD card.");
    } else {
      Serial.printf("📄 Reading %s from SD card (%lu bytes):\n", filename, sdFile.size());
      while (sdFile.available()) {
        char c = sdFile.read();
        Serial.print(c);
      }
      sdFile.close();
      Serial.println("\n✅ Done reading from SD card.");
    }
  }
}

void loop() {
  // Nothing here
}

