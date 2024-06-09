#define PHSENS_CALIBRATION_VALUE 0

float calculate_ph(uint8_t sensor_pin)
{
    float raw_reading = analogRead(sensor_pin);
    Serial.println(raw_reading);
    return ((raw_reading * 5.0 / 1023)) + PHSENS_CALIBRATION_VALUE;
}