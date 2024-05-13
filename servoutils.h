#include <Servo.h>

#define SERVO_OFFSET 30

class HakimiServo {  // Can't name it servo 'cause it's already in Servo.h
    private:
    Servo *servo;

    public:
    HakimiServo(Servo *servoaddr, uint8_t pin)
    {
        servoaddr->attach(pin);
        servo = servoaddr;
    }

    void servo_resetpos()
    {
        servo->write(0+SERVO_OFFSET);
        delay(15);
        servo->write(90+SERVO_OFFSET);
        delay(15);
    }

    void servo_steerRight(bool resetpos=false)
    {
        if (resetpos)
            servo_resetpos();

        servo->write(180+SERVO_OFFSET);
        delay(15+SERVO_OFFSET);
    }

    void servo_steerLeft(bool resetpos=false)
    {
        if (resetpos)
            servo_resetpos();

        servo->write(0+SERVO_OFFSET);
        delay(15);
    }
};
