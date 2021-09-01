#include <Arduino.h>
#include "BLEDevice.h"
//#include "BLEScan.h"

#define LED 2
#define RXD2 16
#define TXD2 17

// The remote service we wish to connect to.
static BLEUUID adversingServiceUUID("0000fee7-0000-1000-8000-00805f9b34fb");
static BLEUUID serviceUUID("0000feea-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("00002AA1-0000-1000-8000-00805F9B34FB");
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

BLEScan *pBLEScan;

char ledOnOff = '1';
int scanTime = 5; //In seconds

static void notifyCallback(
		BLERemoteCharacteristic *pBLERemoteCharacteristic,
		uint8_t *pData,
		size_t length,
		bool isNotify)
{
	Serial.print("Notify callback for characteristic ");
	Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	String data((char *)pData);
	data.remove(data.length() - 1, 1);
	Serial.println(data.length());
	Serial.print("Data without n: ");
	Serial.println(data);
	Serial2.print(data);
}

class ClientCallback : public BLEClientCallbacks
{
	void onConnect(BLEClient *pClient)
	{
		Serial.println("Connected: id = " + pClient->getConnId());
	}

	void onDisconnect(BLEClient *pClient)
	{
		connected = false;
		Serial.println("onDisconnect");
	}
};

bool connectToServer()
{
	Serial.print("Forming a connection to ");
	Serial.println(myDevice->getAddress().toString().c_str());

	BLEClient *pClient = BLEDevice::createClient();
	Serial.println(" - Created client");

	pClient->setClientCallbacks(new ClientCallback());

	// Connect to the remove BLE Server.
	pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
	Serial.println(" - Connected to server");

	// Obtain a reference to the service we are after in the remote BLE server.
	BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
	if (pRemoteService == nullptr)
	{
		Serial.print("Failed to find our service UUID: ");
		Serial.println(serviceUUID.toString().c_str());
		pClient->disconnect();
		return false;
	}
	Serial.println(" - Found our service");

	// Obtain a reference to the characteristic in the service of the remote BLE server.
	pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
	if (pRemoteCharacteristic == nullptr)
	{
		Serial.print("Failed to find our characteristic UUID: ");
		Serial.println(charUUID.toString().c_str());
		pClient->disconnect();
		return false;
	}
	Serial.println(" - Found our characteristic");

	// Read the value of the characteristic.
	if (pRemoteCharacteristic->canRead())
	{
		std::string value = pRemoteCharacteristic->readValue();
		Serial.print("The characteristic value was: ");
		Serial.println(value.c_str());
	}

	if (pRemoteCharacteristic->canNotify())
		pRemoteCharacteristic->registerForNotify(notifyCallback);

	connected = true;
	return true;
}

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
	/**
   * Called for each advertising BLE server.
   */
	void onResult(BLEAdvertisedDevice advertisedDevice)
	{
		Serial.print("BLE Advertised Device found: ");
		Serial.println(advertisedDevice.toString().c_str());

		// We have found a device, let us now see if it contains the service we are looking for.
		if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(adversingServiceUUID))
		{
			Serial.println("Stop scanning.");
			BLEDevice::getScan()->stop();
			myDevice = new BLEAdvertisedDevice(advertisedDevice);
			doConnect = true;
			doScan = true;

		} // Found our server
	}		// onResult
};		// AdvertisedDeviceCallbacks

void btStartBle()
{
	Serial.println("Starting Arduino BLE Client application...");
	BLEDevice::init("SMART_PANTRY");

	// Retrieve a Scanner and set the callback we want to use to be informed when we
	// have detected a new device.  Specify that we want active scanning and start the
	// scan to run for 5 seconds.
	BLEScan *pBLEScan = BLEDevice::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
	pBLEScan->setInterval(1349);
	pBLEScan->setWindow(449);
	pBLEScan->setActiveScan(true);
	pBLEScan->start(5, false);
}

void setup()
{
	Serial.begin(115200);
	delay(500);
	Serial2.begin(115200);
	pinMode(LED, OUTPUT);
	btStartBle();
}

void loop()
{
	if (!connected)
	{
		digitalWrite(LED, HIGH);
		delay(1000);
		digitalWrite(LED, LOW);
		delay(1000);
	}
	else
	{
		digitalWrite(LED, HIGH);
		delay(2000);
	}

	// If the flag "doConnect" is true then we have scanned for and found the desired
	// BLE Server with which we wish to connect.  Now we connect to it.  Once we are
	// connected we set the connected flag to be true.
	if (doConnect == true)
	{
		if (connectToServer())
		{
			Serial.println("We are now connected to the BLE Server.");
		}
		else
		{
			Serial.println("We have failed to connect to the server; there is nothin more we will do.");
		}
		doConnect = false;
	}

	// If we are connected to a peer BLE Server, update the characteristic each time we are reached
	// with the current time since boot.
	if (connected)
	{
		String newValue = "Time since boot: " + String(millis() / 1000);
		Serial.println("Setting new characteristic value to \"" + newValue + "\"");

		// Set the characteristic's value to be the array of bytes that is actually a string.
		// pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
	}
	else if (doScan)
	{
		BLEDevice::getScan()->start(0); // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
	}
}