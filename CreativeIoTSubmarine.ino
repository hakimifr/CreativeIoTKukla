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
Motor thruster_updown_out12enA(MDRIVER_OUT1, MDRIVER_OUT2, MDRIVER_ENA);
Motor thruster_main_out34enB(MDRIVER_OUT3, MDRIVER_OUT4, MDRIVER_ENB);

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

bool orientation_ok;
long int duration;
long int distance;
float temp, prevTemp = -9999;

Servo rudder_servo;

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

  rudder_servo.attach(RUDDER_SERVO);
  servo_resetpos(&rudder_servo);

  Serial.begin(9600);
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
    delay(3000);

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

void loop() {
  send_sonic_burst(US_SENS_TRIG);
  orientation_ok = !digitalRead(ORIENTATION_BULB);

  if (!orientation_ok) {
    if (thruster_main_out34enB.is_motor_running())
      thruster_main_out34enB.halt_motor();

    for (int i=0; i<5; i++) {
      tone(BUZZER1_PIN, 2000, 50);
      delay(100);
    }
  }

  if (!thruster_main_out34enB.is_motor_running()) {
    thruster_main_out34enB.start_motor(50);
  }

  duration = pulseIn(US_SENS_ECHO, HIGH);
  distance = calc_usonic_sensor_distance(duration);
  tempsens.requestTemperatures();
  temp = get_stable_temp(tempsens.getTempCByIndex(0));

  Serial.print("distance=");
  Serial.print(distance);

  Serial.print("  temp=");
  Serial.print(temp);
  Serial.print("∘C\n");

  if (temp > 40)
    Serial.println("Warning: Water temperature is too high, please re-position the submarine!");
  else if (temp > 35)
    Serial.println("Warning: Water temp is quite high...");

  if (distance < 20) {
    Serial.println("Obstacle detected, moving away!");
    thruster_updown_out12enA.start_motor(50, true);
    tone(BUZZER1_PIN, 2000, 500);
    servo_steerRight(&rudder_servo);
  } else {
    servo_resetpos(&rudder_servo);
    thruster_updown_out12enA.halt_motor();
  }
}

