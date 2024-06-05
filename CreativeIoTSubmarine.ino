#include <Bonezegei_DHT11.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiNINA.h>
#include <Servo.h>

#include "wifi_secrets.h"
#include "netutils.h"
#include "servoutils.h"
#include "mdriverutils.h"
#include "usonicSensUtils.h"
#include "cloud.h"

// Motor driver pins
#define MDRIVER_OUT1 2
#define MDRIVER_OUT2 3
#define MDRIVER_OUT3 11
#define MDRIVER_OUT4 12
#define MDRIVER_ENA 5
#define MDRIVER_ENB 4

// Rudder Servo
#define RUDDER_SERVO 0

// Ultrasonic sensor pins
#define US_SENS_TRIG 6
#define US_SENS_ECHO 7

// Orientation bulb
#define ORIENTATION_BULB 1

// Buzzer
#define BUZZER1_PIN 8

// Temp sensor
#define T_SENS_PIN 9

OneWire wire(T_SENS_PIN);
DallasTemperature tempsens(&wire);

// Motors
Motor thruster_main_out34enB(MDRIVER_OUT3, MDRIVER_OUT4, MDRIVER_ENB);
Motor cooling_fan(MDRIVER_OUT1, MDRIVER_OUT2, MDRIVER_ENA);

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

bool orientation_ok;
long int duration;
long int distance, prevDistance;
float temp, prevTemp = -9999;

Servo _rudder_servo;
HakimiServo rudder_servo(&_rudder_servo, RUDDER_SERVO);

void setup() {
  int status = WL_IDLE_STATUS;
  int numberOfDevices;
  pinMode(MDRIVER_OUT1, OUTPUT);
  pinMode(MDRIVER_OUT2, OUTPUT);
  pinMode(MDRIVER_OUT3, OUTPUT);
  pinMode(MDRIVER_OUT4, OUTPUT);
  pinMode(MDRIVER_ENA, OUTPUT);
  pinMode(MDRIVER_ENB, OUTPUT);
  pinMode(US_SENS_ECHO, INPUT);
  pinMode(US_SENS_TRIG, OUTPUT);
  pinMode(ORIENTATION_BULB, INPUT);

  rudder_servo.servo_resetpos();

  Serial.begin(115200);
  tempsens.begin();

  // WiFi connections
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
    tone(BUZZER1_PIN, 2000, 1000);
    delay(3000);

    switch (status) {
      case WL_CONNECT_FAILED:
        Serial.println("Failed to connect to network!");
        break;
      case WL_CONNECTED:
        Serial.println("Connected successfully");
        break;
    }

    // Arduino Cloud
    ArduinoCloud.begin(iotPreferredCon);
    ArduinoCloud.addProperty(temp, WRITE);
  }

  printCurrentNet();
  printWifiData();

  // Never turn off this cooling fan, or the motor
  // driver will get very hot! (melecur tangan 😌)
  tone(BUZZER1_PIN, 2000, 1000);
  cooling_fan.start_motor(255);
  delay(5000);
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

void loop() {
  ArduinoCloud.update();
  send_sonic_burst(US_SENS_TRIG);
  duration = pulseIn(US_SENS_ECHO, HIGH);
  distance = calc_usonic_sensor_distance(duration);
  orientation_ok = !digitalRead(ORIENTATION_BULB);
  Serial.print("Orientation bulb value: ");
  Serial.println(orientation_ok);

  if (distance == 0)
    distance = prevDistance;
  else
    prevDistance = distance;

  if (!orientation_ok) {
    if (thruster_main_out34enB.is_motor_running())
      thruster_main_out34enB.halt_motor();

    for (int i=0; i<5; i++) {
      tone(BUZZER1_PIN, 2000, 50);
      delay(100);
    }
  }

  if (!thruster_main_out34enB.is_motor_running()) {
    thruster_main_out34enB.start_motor(128);
  }

  tempsens.requestTemperatures();
  temp = get_stable_temp(tempsens.getTempCByIndex(0));

  Serial.print("distance=");
  Serial.print(distance);
  Serial.print("cm");

  Serial.print("  temp=");
  Serial.print(temp);
  Serial.print("∘C\n");

  if (temp > 40)
    Serial.println("Warning: Water temperature is too high, please re-position the submarine!");
  else if (temp > 35)
    Serial.println("Warning: Water temp is quite high...");

  if (distance < 20) {
    Serial.println("Obstacle detected, moving away!");
    tone(BUZZER1_PIN, 2000, 500);
    rudder_servo.servo_steerRight();
  } else {
    rudder_servo.servo_steerLeft();
  }
}
