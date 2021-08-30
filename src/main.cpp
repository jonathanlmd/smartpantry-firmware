#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LED 23
#define SERVICE_UUID "6e74769c-8bbd-417a-8924-85b8f9e2008d"
#define CHARACTERISTIC_UUID_RX "be8a0622-ed83-4148-9183-d39134a62821"

BLECharacteristic *characteristic;

char ledOnOff = '1';

String HOST_NAME = "https://smartpantryuepg.herokuapp.com/pantry/add-item/";
String USER_ID = "612b0e8787a4d2750f4a3268";

// class ServerCallbacks : public BLEServerCallbacks
// {
// 	void onConnect(BLEServer *pServer)
// 	{
// 		Serial.println("Bluetooth Connected");
// 	};

// 	void onDisconnect(BLEServer *pServer)
// 	{
// 		Serial.println("Bluetooth Connected");
// 	};
// };

class CharacteristicServerCallbacks : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string rxValue = pCharacteristic->getValue();
		if (rxValue.length() > 0)
		{
			for (int i = 0; i < rxValue.length(); i++)
			{
				Serial.print(rxValue[i]);
			}
			Serial.println();
		}
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
			WiFi.begin("Rebecca", "re010716");
			Serial.print("Waiting for WiFi to connect...");
			while (WiFi.status() != WL_CONNECTED)
			{
				Serial.print(".");
			}
			Serial.println(" connected");
			Serial.println(WiFi.localIP());

			HTTPClient http;
			String PATH = HOST_NAME + rxValue.c_str() + "/" + USER_ID;
			http.begin(PATH.c_str());
			http.GET();
			http.end();
		}
	};
};

void setup()
{
	Serial.begin(115200);
	pinMode(LED, OUTPUT);

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

void loop()
{
	if (ledOnOff == '0')
	{
		digitalWrite(LED, HIGH); // turn the LED on (HIGH is the voltage level)
	}
	if (ledOnOff == '1')
	{
		digitalWrite(LED, LOW); // turn the LED on (HIGH is the voltage level)
	}
	delay(1000);
}