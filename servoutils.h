#include <Servo.h>

#define SERVO_OFFSET 30

void servo_resetpos(Servo *servo)
{
    servo->write(0+SERVO_OFFSET);
    delay(15);
    servo->write(90+SERVO_OFFSET);
    delay(15);
}

void servo_steerRight(Servo *servo, bool resetpos=false)
{
    if (resetpos)
        servo_resetpos(servo);

    servo->write(180+SERVO_OFFSET);
    delay(15);
}

void servo_steerLeft(Servo *servo, bool resetpos=false)
{
    if (resetpos)
        servo_resetpos(servo);

    servo->write(0+SERVO_OFFSET);
    delay(15);
}