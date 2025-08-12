#include <SPI.h>

// Pin definitions (adjust for your hardware)
#define CS_PIN 5

// SPI settings at 20 MHz, mode 0
SPISettings spiSettings(20000000, MSBFIRST, SPI_MODE0);

uint32_t flashCapacityBytes = 0;
uint32_t badSectors = 0;

bool confirmAction(const char* prompt) {
  Serial.print(prompt);
  while (!Serial.available()) delay(10);
  String input = Serial.readStringUntil('\n');
  input.trim();
  return (input.equalsIgnoreCase("YES"));
}

uint32_t readJEDECID() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x9F); // JEDEC ID command
  uint8_t mfr = SPI.transfer(0);
  uint8_t memType = SPI.transfer(0);
  uint8_t capCode = SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  Serial.printf("JEDEC ID: %02X %02X %02X\n", mfr, memType, capCode);

  // Capacity = 1 << capCode (bits) / 8 for bytes
  uint32_t capacity = (1UL << capCode);
  capacity /= 8;
  return capacity;
}

bool testSector(uint32_t addr) {
  // Simple erase+verify test
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x20); // Sector erase (4KB)
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  delay(50); // wait minimal erase time (safe for test)

  // Read back to check if erased to 0xFF
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x03); // Read data
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);

  bool good = true;
  for (uint16_t i = 0; i < 256; i++) { // check first 256 bytes
    if (SPI.transfer(0x00) != 0xFF) {
      good = false;
      break;
    }
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  return good;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("=== Safe SPI Flash Capacity & Bad Sector Scan ===");
  Serial.println("This test will ERASE sampled sectors and may shorten flash lifespan!");

  if (!confirmAction("Type YES to continue: ")) {
    Serial.println("Aborted.");
    return;
  }
  if (!confirmAction("Are you ABSOLUTELY sure? Type YES again: ")) {
    Serial.println("Aborted.");
    return;
  }

  SPI.begin();
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  flashCapacityBytes = readJEDECID();
  Serial.printf("Detected raw capacity: %lu bytes (%.2f MB)\n",
                flashCapacityBytes, flashCapacityBytes / 1048576.0);

  Serial.println("Starting sector scan...");

  uint32_t sectors = flashCapacityBytes / 4096; // 4KB sectors
  for (uint32_t s = 0; s < sectors; s += 64) { // sample every 64th sector
    uint32_t addr = s * 4096;
    if (!testSector(addr)) {
      badSectors++;
      Serial.printf("Bad sector at 0x%06lX\n", addr);
    }
    yield(); // avoid watchdog reset
  }

  uint32_t usableBytes = flashCapacityBytes - (badSectors * 4096);
  Serial.printf("Bad sectors: %lu\n", badSectors);
  Serial.printf("Usable capacity: %lu bytes (%.2f MB)\n",
                usableBytes, usableBytes / 1048576.0);
}

void loop() {
  // Nothing here — one-time test
}





