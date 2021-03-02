#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic = NULL;
std::string kondisi = "";
const int pin1 = 34;
const int pin2 = 35;
int value1 = 0;
int value2 = 0;

void setup() {
  Serial.begin(9600);
  BLEDevice::init("Long name works now");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setValue(kondisi);
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  value1 = analogRead(pin1);
  value2 = analogRead(pin2);
  if ((value1 > 0) && (value1 < 1000)){
    kondisi = "Banjir < 10cm ";
    pCharacteristic->setValue(kondisi);
    pCharacteristic->notify();
  } else if ((value1 >= 1000) && (value2 <= 1000)) {
    kondisi = "Banjir > 10cm ";
    pCharacteristic->setValue(kondisi);
    pCharacteristic->notify();
  } else if (value2 > 1000) {
    kondisi = "Banjir ± 20cm ";
    pCharacteristic->setValue(kondisi);
    pCharacteristic->notify();
  } else {
    kondisi = "Tidak Banjir  ";
    pCharacteristic->setValue(kondisi);
    pCharacteristic->notify();
  }
  Serial.println(kondisi.c_str());
  delay(2000);
}
