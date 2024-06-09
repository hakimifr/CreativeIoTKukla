#include <Bonezegei_DHT11.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiNINA.h>
#include <Servo.h>

#include "wifi_secrets.h"
#include "cloud_secret.h"
#include "netutils.h"
#include "servoutils.h"
#include "mdriverutils.h"
#include "usonicSensUtils.h"
// #include "led.h"
#include "phsensor.h"
#include "cloud.h"

// Motor driver pins
#define MDRIVER_OUT1 2
#define MDRIVER_OUT2 3
#define MDRIVER_OUT3 13
#define MDRIVER_OUT4 14
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

// pH sensor
#define PH_SENS_PIN A6  // connect to Po

OneWire wire(T_SENS_PIN);
DallasTemperature tempsens(&wire);

// Motors
Motor thruster_main_out34enB(MDRIVER_OUT3, MDRIVER_OUT4, MDRIVER_ENB);

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

bool orientation_ok;
long int duration;
int distance, prevDistance;
int rudder_pos, thruster_speed;  // rudder_pos: middle: 0, left: 1, right: 2
float temp, ph, prevTemp = -9999;
bool huskylens_is_obj_detected;

Servo _rudder_servo;
HakimiServo rudder_servo(&_rudder_servo, RUDDER_SERVO);

void setup() {
  int status = WL_IDLE_STATUS;
  pinMode(MDRIVER_OUT1, OUTPUT);
  pinMode(MDRIVER_OUT2, OUTPUT);
  pinMode(MDRIVER_OUT3, OUTPUT);
  pinMode(MDRIVER_OUT4, OUTPUT);
  pinMode(MDRIVER_ENA, OUTPUT);
  pinMode(MDRIVER_ENB, OUTPUT);
  pinMode(US_SENS_ECHO, INPUT);
  pinMode(US_SENS_TRIG, OUTPUT);
  pinMode(PH_SENS_PIN, INPUT);
  pinMode(ORIENTATION_BULB, INPUT);
  pinMode(T_SENS_PIN, INPUT);

  Serial.begin(9600);
  delay(1500);
  tempsens.begin();

  // led
  // White: Connecting to network
  // Blue: Connected
  // Yellow: Obstacle
  // Red: Error
  // set_color(255, 255, 255);

  // // WiFi connections
  // if (WiFi.status() == WL_NO_MODULE)
  //   Serial.println("Cannot detect wifi module!");
  // else
  //  Serial.println("WiFi module detected");

  // Serial.print("WiFiNINA FW version: ");
  // Serial.println(WiFi.firmwareVersion());

  // while (status != WL_CONNECTED) {
  //   Serial.print("Trying to connect to wifi network: ");
  //   Serial.println(ssid);
  //   status = WiFi.begin(ssid, password);
  //   tone(BUZZER1_PIN, 2000, 1000);
  //   delay(3000);

  //   switch (status) {
  //     case WL_CONNECT_FAILED:
  //       Serial.println("Failed to connect to network!");
  //       break;
  //     case WL_CONNECTED:
  //       Serial.println("Connected successfully");
  //       set_color(0, 0, 255);
  //       break;
  //   }

  // }

  // Arduino Cloud
  ArduinoCloud.setThingId(THING_ID);  // from cloud_secret.h
  ArduinoCloud.addProperty(ph, READ, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(temp, READWRITE, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(distance, READ, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(orientation_ok, READWRITE, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(huskylens_is_obj_detected, READWRITE, 1 * SECONDS, NULL);
  ArduinoCloud.addProperty(rudder_pos, READWRITE, ON_CHANGE, on_rudder_pos_change);
  ArduinoCloud.addProperty(thruster_speed, READWRITE, ON_CHANGE, on_thruster_change);
  ArduinoCloud.begin(iotPreferredCon);

  // printCurrentNet();
  // printWifiData();

  thruster_speed = 70;
  thruster_main_out34enB.start_motor(thruster_speed);
}

void on_thruster_change()
{
  Serial.println("Thruster value changed");
  bool should_reverse = false;

  if (!(-255<=thruster_speed<=255))
    return;

  if (thruster_speed < 0)
    should_reverse = true;

  thruster_main_out34enB.start_motor(abs(thruster_speed), should_reverse);
}

void on_rudder_pos_change()
{
  Serial.println("Rudder position changed");
  if (!(0<=rudder_pos<=2))
    return;

  switch (rudder_pos) {
    case 0:
      rudder_servo.servo_resetpos();
      Serial.println("Resetting servo position");
      break;
    case 1:
      rudder_servo.servo_steerLeft();
      Serial.println("Moving servo to the left");
      break;
    case 2:
      rudder_servo.servo_steerRight();
      Serial.println("Moving servo to the right");
  }

}

float get_stable_temp(float temp)
{
  int calibrationTemp;

  if (prevTemp == -9999) {
    tempsens.requestTemperatures();
    calibrationTemp = tempsens.getTempCByIndex(0);

    while (calibrationTemp == -127) {
      Serial.println("Waiting for valid reading to calibrate");
      tempsens.requestTemperatures();
      calibrationTemp = tempsens.getTempCByIndex(0);
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
  // if (WiFi.status() == WL_DISCONNECTED || WL_FAILURE)
  //   set_color(255, 0, 0);
  // else
  //   set_color(0, 0, 255);

  // srand(time(NULL));
  // if ((rand() % 2) == 0)
  //   huskylens_is_obj_detected = true;
  // else
  //   huskylens_is_obj_detected = false;


  ArduinoCloud.update();
  send_sonic_burst(US_SENS_TRIG);
  duration = pulseIn(US_SENS_ECHO, HIGH);
  distance = calc_usonic_sensor_distance(duration);
  orientation_ok = !digitalRead(ORIENTATION_BULB);
  Serial.print("Orientation bulb value: ");
  Serial.println(orientation_ok);

  if (distance == 0) {
    Serial.println("Warning: Ultrasonic sensor disconnected");
    distance = prevDistance;
  }
  else
    prevDistance = distance;

  // if (!orientation_ok) {
  //   if (thruster_main_out34enB.is_motor_running())
  //     thruster_main_out34enB.halt_motor();

  //   for (int i=0; i<5; i++) {
  //     tone(BUZZER1_PIN, 2000, 50);
  //     delay(100);
  //   }
  // }

  tempsens.requestTemperatures();
  temp = get_stable_temp(tempsens.getTempCByIndex(0));
  ph = calculate_ph(PH_SENS_PIN);

  Serial.print("distance=");
  Serial.print(distance);
  Serial.print("cm");

  Serial.print("  pH=");
  Serial.print(ph);

  Serial.print("  temp=");
  Serial.print(temp);
  Serial.print("∘C\n");

  if (temp > 40)
    Serial.println("Warning: Water temperature is too high, please re-position the submarine!");
  else if (temp > 35)
    Serial.println("Warning: Water temp is quite high...");

//   if (distance < 30)
//     thruster_main_out34enB.halt_motor();
//   else
//     thruster_main_out34enB.start_motor(200);
}
