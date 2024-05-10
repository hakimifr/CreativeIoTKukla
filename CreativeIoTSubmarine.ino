#include <Bonezegei_DHT11.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiNINA.h>

#include "wifi_secrets.h"
#include "netutils.h"

// Motor driver pins
#define MDRIVER_OUT1 2
#define MDRIVER_OUT2 3

// Ultrasonic sensor pins
#define US_SENS_TRIG 6
#define US_SENS_ECHO 7

// Buzzer
#define BUZZER1_PIN 5

// dd
#define T_SENS_PIN 4

OneWire wire(T_SENS_PIN);
DallasTemperature tempsens(&wire);

/* ----------------- ANALOG TEMP SENSOR ----------------------*/
// Temp sensor pin
const int ThermistorPin = A0;
int Vo;

const float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
float R1 = 10300; // value of R1 on board
float logR2, R2, T;
/* ---------------------------------------------------------*/

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

long int duration;
long int distance;
float temp, prevTemp = -9999;

void setup() {
  int status = WL_IDLE_STATUS;
  int numberOfDevices;
  pinMode(MDRIVER_OUT1, OUTPUT);
  pinMode(MDRIVER_OUT2, OUTPUT);
  pinMode(US_SENS_ECHO, INPUT);
  pinMode(US_SENS_TRIG, OUTPUT);

  Serial.begin(31250);
  while (!Serial);
  tempsens.begin();

  if (WiFi.status() == WL_NO_MODULE)
    Serial.println("Cannot detect wifi module!");
  else
   Serial.println("WiFi module detected");

  Serial.print("WiFiNINA FW version: ");
  Serial.println(WiFi.firmwareVersion());

  while (status != WL_CONNECTED) {
    Serial.print("Trying to connect to wifi network: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    delay(10000);

    switch (status) {
      case WL_CONNECT_FAILED:
        Serial.println("Failed to connect to network!");
        break;
      case WL_CONNECTED:
        Serial.println("Connected successfully");
        break;
    }
  }

  printCurrentNet();
  printWifiData();
}

float get_stable_temp(float temp)
{
  int calibrationTemp;

  if (prevTemp == -9999) {
    tempsens.requestTemperatures();
    calibrationTemp = tempsens.getTempCByIndex(0);

    while (calibrationTemp == -127) {
      Serial.println("Waiting for valid reading to calibrate");
    }

    prevTemp = calibrationTemp;
  }

  if (abs(temp-prevTemp) > 10) {
    // Re-use previous temp instead
    Serial.println("Warning: Temperature probe disconnected, check connection, using old values for now...");
    return prevTemp;
  }

    prevTemp = temp;
    return temp;
}

long int calc_usonic_sensor_distance(long int duration)
{
  return duration * 0.034 / 2;
}

void send_sonic_burst()
{
  // Ensure the pin is cleared out
  digitalWrite(US_SENS_TRIG, LOW);

  // By sending 10us pulse
  digitalWrite(US_SENS_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_SENS_TRIG, LOW);
}

void loop() {
  send_sonic_burst();

  duration = pulseIn(US_SENS_ECHO, HIGH);
  distance = calc_usonic_sensor_distance(duration);
  tempsens.requestTemperatures();
  temp = get_stable_temp(tempsens.getTempCByIndex(0));

  Serial.print("distance=");
  Serial.print(distance);

  Serial.print("  temp=");
  Serial.print(temp);
  Serial.print("∘C\n");

  if (distance < 20) {
    Serial.println("Obstacle detected, moving away!");
    digitalWrite(MDRIVER_OUT1, LOW);
    digitalWrite(MDRIVER_OUT2, HIGH);
    tone(BUZZER1_PIN, 2000, 500);
  } else {
    digitalWrite(MDRIVER_OUT1, LOW);
    digitalWrite(MDRIVER_OUT2, LOW);
  }
}

