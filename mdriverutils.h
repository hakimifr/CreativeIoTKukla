void start_motor(uint8_t outx, uint8_t outy,
                 uint8_t enx, int speed)
{
    analogWrite(enx, speed);
    digitalWrite(outx, HIGH);
    digitalWrite(outy, LOW);
}

void halt_motor(uint8_t outx, uint8_t outy,
                 uint8_t enx)
{
    digitalWrite(outx, LOW);
    digitalWrite(outy, LOW);
}