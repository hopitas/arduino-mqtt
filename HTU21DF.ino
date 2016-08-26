#include <Adafruit_HTU21DF.h>
#include <Wire.h>

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (A5 on UNO)
// Connect SDA to I2C data pin (A4 on UNO)

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
bool htu21dfInitialised = false;

void initHtu21df() {
	if (htu21dfInitialised) { return; }
	htu.begin();
	htu21dfInitialised = true;
}

void getHtu21dfReadings() {
	const int numberOfSamples = 4;

	initHtu21df();
	data.temperature = data.humidity = 0;

	for (int c = 0; c < numberOfSamples; c++) {
		data.temperature += htu.readTemperature();
		data.humidity += htu.readHumidity();
		delay(500);
	}
	data.temperature /= numberOfSamples;
	data.humidity /= numberOfSamples;
}