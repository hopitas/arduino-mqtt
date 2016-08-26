/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
	it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
	else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
	   http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson - installed via library manager
#include "globals.h"        // global structures and enums used by the applocation
//#include <WiFiClientSecure.h>

CloudConfig cloud;
DeviceConfig device;
SensorData data;

IPAddress timeServer(62, 237, 86, 238);

// Update these with values suitable for your network.

void initDeviceConfig() { // Example device configuration
	device.boardType = WeMos;            // BoardType enumeration: NodeMCU, WeMos, SparkfunThing, Other (defaults to Other). This determines pin number of the onboard LED for wifi and publish status. Other means no LED status
	device.deepSleepSeconds = 0;         // if greater than zero with call ESP8266 deep sleep (default is 0 disabled). GPIO16 needs to be tied to RST to wake from deepSleep. Causes a reset, execution restarts from beginning of sketch
	cloud.cloudMode = IoTHub;            // CloudMode enumeration: IoTHub and EventHub (default is IoTHub)
	cloud.publishRateInSeconds = 60;     // limits publishing rate to specified seconds (default is 90 seconds).  Connectivity problems may result if number too small eg 2
	cloud.sasExpiryDate = 1737504000;    // Expires Wed, 22 Jan 2025 00:00:00 GMT (defaults to Expires Wed, 22 Jan 2025 00:00:00 GMT)
	cloud.id = "WeMos01";
	//cloud.geo = "63.1783670, 23.7790370"; // Vimpeli city
	cloud.geo = "60.1720170, 24.8273260"; // Microsoft Espoo
}

const char* ssid = "yourssid";
const char* password = "yourpass";
const char* mqtt_server = "youriothubname.azure-devices.net";

WiFiClientSecure espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int sendCount = 0;
char buffer[256];

void setup() {
	// pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	Serial.begin(115200);
	setup_wifi();
	getCurrentTime();
	client.setServer(mqtt_server, 8883);
	client.setCallback(callback);
	initDeviceConfig();
	reconnect();
	pinMode(D6, OUTPUT);
	digitalWrite(D6, LOW);
}

void setup_wifi() {

	delay(10);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	// Switch on the LED if an 1 was received as first character
	if ((char)payload[0] == '1') {
		digitalWrite(D6, HIGH);   // Turn the LED on (Note that LOW is the voltage level
		// but actually the LED is on; this is because
		// it is acive low on the ESP-01)
	}
  if ((char)payload[0] == '0') {
    digitalWrite(D6, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  }
	else {

	}

}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("devid", "youriothubname.azure-devices.net/devid", "use device explorer to generate SAS token and copy/paste whole thing here")) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			measureSensor();
			client.publish("devices/devid/messages/events/", serializeData(data));
			// ... and resubscribe
			client.subscribe("devices/devid/messages/devicebound/#");
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void getCurrentTime() {
	int ntpRetryCount = 0;
	while (timeStatus() == timeNotSet && ++ntpRetryCount < 10) { // get NTP time
		Serial.println(WiFi.localIP());
		setSyncProvider(getNtpTime);
		setSyncInterval(60 * 60);
	}
}

void measureSensor() {  // uncomment sensor, default is getFakeWeatherReadings()
  //  getFakeWeatherReadings();
  //  getDht11Readings();
 //   getDht22Readings();
  //  getBmp180Readings();
  //  getBmp280Readings();
	getHtu21dfReadings();
	//  getLdrReadings(); // when enabled causes the DHT11 sensor to fail
}

void getFakeWeatherReadings() {
	data.temperature = 25;
	data.humidity = 50;
	data.pressure = 1000;
}

void loop() {
	client.loop();  //reads command...
	long now = millis();
	if (now - lastMsg > 2000) {
		lastMsg = now;
		++value;
		measureSensor();
		client.publish("devices/devid/messages/events/", serializeData(data));
		Serial.print("Publish message: ");
		Serial.print(value);
		Serial.println(serializeData(data));	
	}
}

