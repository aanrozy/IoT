#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char* server   = "broker.mqttdashboard.com"; //your broker server
const char* ssid     = "mqtt"; //your wifi ssid
const char* password = "abcdefgh"; //your wifi password
const char* message; 
const int pin1 = 34; // pin for sensor you want to read
std::string value;
int value1 = 0;
String rain = "";
String condition;

WiFiClient wifiClient;
PubSubClient mqttClient(server, 1883, wifiClient);

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");
    if(pRemoteCharacteristic->canRead()) {
      value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};

void wifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}

void mqtt(){
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Skrips1-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      mqttClient.publish("Blimbing", "connected");
      mqttClient.subscribe("Blimbingin");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  wifi();
  mqtt();
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    wifi();
  }
  if (!mqttClient.connected()) {
    mqtt();
  }
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  if (connected) {
    value = pRemoteCharacteristic->readValue();
    value1 = analogRead(pin1);
    if (value1 < 1000) {
      rain = "Deras ";
    }
    else if ((value1 > 1000) && (value1 < 3600)) {
      rain = "Hujan ";
    }
    else {
      rain = "Cerah ";
    };
  }else if(doScan){
    BLEDevice::getScan()->start(0);
  }
  condition = rain + value.c_str() + "di Jl. Borobudur, http://bit.ly/3bbULJG";
  message = condition.c_str();
  int length = strlen(message);
  mqttClient.publish("Blimbing", (byte*)message, length, true);
  mqttClient.loop();
  Serial.println(message);
  delay(2000);
}
