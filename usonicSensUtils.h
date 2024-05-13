long int calc_usonic_sensor_distance(long int duration)
{
  return duration * 0.034 / 2;
}

void send_sonic_burst(uint8_t trigger_pin)
{
  // Ensure the pin is cleared out
  digitalWrite(trigger_pin, LOW);

  // By sending 10us pulse
  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, LOW);
}