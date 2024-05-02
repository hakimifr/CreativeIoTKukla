// Motor driver pins
#define MDRIVER_OUT1 2
#define MDRIVER_OUT2 3

// Ultrasonic sensor pins
#define US_SENS_TRIG 6
#define US_SENS_ECHO 7

long int duration;
long int distance;

void setup() {
  pinMode(MDRIVER_OUT1, OUTPUT);
  pinMode(MDRIVER_OUT2, OUTPUT);
  pinMode(US_SENS_ECHO, INPUT);
  pinMode(US_SENS_TRIG, OUTPUT);

  Serial.begin(9600);
}

long int calc_distance(long int duration)
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
  distance = calc_distance(duration);

  Serial.print(distance);
  Serial.print("\n");
}
