#include <SPI.h>

const uint8_t CS_FLASH = 5;

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin();  // SCK=18, MISO=19, MOSI=23

  pinMode(CS_FLASH, OUTPUT);
  digitalWrite(CS_FLASH, HIGH);  // Deselect Flash

  delay(100);

  Serial.println("Reading JEDEC ID from SPI Flash...");

  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));

  digitalWrite(CS_FLASH, LOW);  // Select Flash
  SPI.transfer(0x9F);  // Send JEDEC ID command
  uint8_t manufacturer = SPI.transfer(0x00);
  uint8_t memoryType   = SPI.transfer(0x00);
  uint8_t capacity     = SPI.transfer(0x00);
  digitalWrite(CS_FLASH, HIGH);  // Deselect Flash

  SPI.endTransaction();

  Serial.print("JEDEC ID: ");
  Serial.print(manufacturer, HEX); Serial.print(" ");
  Serial.print(memoryType, HEX); Serial.print(" ");
  Serial.println(capacity, HEX);
}

void loop() {
  // Do nothing
}


