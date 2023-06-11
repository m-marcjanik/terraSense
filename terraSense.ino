#include <esp_sleep.h>
#include <WiFiManager.h>
#include <UbidotsEsp32Mqtt.h>
#include "config.h"

// define the analog pin used for moisture measurement
constexpr int MOIST_PIN = A0;

// define the analog pin used for voltage measurement
constexpr float VOLTAGE_PIN = A3;

// define high and low battery voltage to use with percentage battery level
float batteryVoltageHigh = 4.2;
float batteryVoltageLow = 3.3;

// define values for dry and wet sensor
int sensorWet = 1615;
int sensorDry = 2770;

// define deep sleep time in minutes
unsigned int deepSleepTimeMinutes = 360;
int numMeasurements = 6;
int delayTime = 1000;
int voltageMeasurements = 6;

Ubidots ubidots(UBIDOTS_TOKEN);

void setup()
{
  // Serial.begin(115200);
  // delay(1500);

  pinMode(MOIST_PIN, INPUT);

  WiFiManager wifiManager;
  wifiManager.autoConnect("terraSense");

  // ubidots.setCallback(callback);
  // ubidots.setup();
  // ubidots.reconnect();
}

void loop()
{
  // moisture measurement
  int sum = 0;

  for (int i = 1; i <= numMeasurements; i++)
  {
    int rawMoisture = analogRead(MOIST_PIN);
    int percentageMoisture = map(rawMoisture, sensorDry, sensorWet, 0, 100);
    Serial.println(percentageMoisture);
    sum += percentageMoisture;
    delay(delayTime);
  }

  int moisture = sum / numMeasurements;

  // battery measurement

  uint32_t Vbatt = 0;

  for (int i = 0; i < voltageMeasurements; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(VOLTAGE_PIN); // ADC with correction
  }
  float batteryVoltage = 2 * Vbatt / voltageMeasurements / 1000.0;     // attenuation ratio 1/2, mV --> V

  float batteryVoltagePercentage = (batteryVoltage - batteryVoltageLow) / (batteryVoltageHigh - batteryVoltageLow) * 100;


  // open connection to Ubidots and send measurements

  ubidots.setup();
  ubidots.reconnect();

  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  else
  {
    // first argument is Ubidots label, second is variable name inside code
    ubidots.add(VARIABLE_LABEL1, moisture);
    ubidots.add(VARIABLE_LABEL2, batteryVoltagePercentage);
    ubidots.add(VARIABLE_LABEL3, batteryVoltage);

    ubidots.publish(DEVICE_LABEL);
  }

  // Serial.println("Sending and entering deep sleep...");
  delay(5000);

  // convert deepSleepTimeMinutes to seconds
  unsigned int deepSleepTimeSeconds = deepSleepTimeMinutes * 60;

  // go into deep sleep for specified time
  esp_sleep_enable_timer_wakeup(deepSleepTimeSeconds * 1000000ULL);
  esp_deep_sleep_start();

  // print information after waking up from deep sleep
  // perial.println("Waking up from deep sleep...");
}
