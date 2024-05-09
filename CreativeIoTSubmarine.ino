#include <Bonezegei_DHT11.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Motor driver pins
#define MDRIVER_OUT1 2
#define MDRIVER_OUT2 3

// Ultrasonic sensor pins
#define US_SENS_TRIG 6
#define US_SENS_ECHO 7

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

long int duration;
long int distance;
float temp, prevTemp = -9999;

void setup() {
  int numberOfDevices;
  pinMode(MDRIVER_OUT1, OUTPUT);
  pinMode(MDRIVER_OUT2, OUTPUT);
  pinMode(US_SENS_ECHO, INPUT);
  pinMode(US_SENS_TRIG, OUTPUT);

  Serial.begin(31250);
  tempsens.begin();
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

  Serial.print("distance=");
  Serial.print(distance);

  tempsens.requestTemperatures();
  temp = tempsens.getTempCByIndex(0);
  Serial.print("  temp=");
  Serial.print(temp);
  Serial.print("∘C\n");

  if (distance < 50) {
    digitalWrite(MDRIVER_OUT1, HIGH);
    digitalWrite(MDRIVER_OUT2, LOW);
  } else {
    digitalWrite(MDRIVER_OUT1, LOW);
    digitalWrite(MDRIVER_OUT2, LOW);
  }
}

