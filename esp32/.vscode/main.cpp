#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define LED 2
#define RXD2 16
#define TXD2 17

#define SERVICE_UUID "6e74769c-8bbd-417a-8924-85b8f9e2008d"
#define CHARACTERISTIC_UUID_RX "be8a0622-ed83-4148-9183-d39134a62821"

// static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

BLECharacteristic *characteristic;
BLEScan *pBLEScan;

char ledOnOff = '1';
int scanTime = 5; //In seconds

class ServerCallbacks : public BLEServerCallbacks
{
	void onConnect(BLEServer *pServer)
	{
		Serial.println("Client bluetooth connected");
	};

	void onDisconnect(BLEServer *pServer)
	{
		Serial.println("Client bluetooth disconnected");
	};
};

class CharacteristicServerCallbacks : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string rxValue = pCharacteristic->getValue();
		Serial.print("Barcode: ");
		Serial.print(rxValue.c_str());
		Serial.println();
		if (rxValue.find("off"))
		{
			ledOnOff = '0';
		}
		else if (rxValue.find("on"))
		{
			ledOnOff = '1';
		}
		else
		{
			Serial2.write(rxValue.c_str());
		}
	};
};

void btStartBle()
{
	// Create the BLE Device
	BLEDevice::init("SMART_PANTRY");
	// Create the BLE Server
	BLEServer *server = BLEDevice::createServer();
	//Set initial callback
	// server->setCallbacks(new ServerCallbacks());
	// Create the BLE Service
	BLEService *service = server->createService(SERVICE_UUID);
	// Create a BLE Characteristic to read data from codebar reader
	characteristic = service->createCharacteristic(
			CHARACTERISTIC_UUID_RX,
			BLECharacteristic::PROPERTY_WRITE);
	// Set callback for read chatacteristic
	characteristic->setCallbacks(new CharacteristicServerCallbacks());
	// Add BLE descriptor
	characteristic->addDescriptor(new BLE2902());
	// Start service
	service->start();

	// Create advertising
	BLEAdvertising *advertising = BLEDevice::getAdvertising();
	advertising->addServiceUUID(SERVICE_UUID);
	advertising->setScanResponse(true);
	advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
	advertising->setMinPreferred(0x12);
	BLEDevice::startAdvertising();
	Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void setup()
{
	Serial.begin(115200);
	Serial2.begin(115200);
	pinMode(LED, OUTPUT);
	btStartBle();
}

void loop()
{
	digitalWrite(LED, HIGH);
	delay(1000);
	BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

	Serial.print("Devices found: ");
	Serial.println(foundDevices.getCount());
	Serial.println("Scan done!");
	pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
	digitalWrite(LED, LOW);
	delay(1000);
}