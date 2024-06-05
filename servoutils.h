#include <Servo.h>

#define SERVO_OFFSET 32

class HakimiServo {  // Can't name it servo 'cause it's already in Servo.h
    private:
    Servo *servo;
    char last_pos = 'X';  // (M)iddle, (L)eft, (R)ight

    public:
    HakimiServo(Servo *servoaddr, uint8_t pin)
    {
        servoaddr->attach(pin);
        servo = servoaddr;
    }

    void servo_resetpos()
    {
        if (last_pos == 'M') {
            Serial.println("Position is already reset");
            return;
        }

        servo->write(0+SERVO_OFFSET);
        delay(15);
        servo->write(90+SERVO_OFFSET);
        delay(15);
        last_pos = 'M';
    }

    void servo_steerRight(bool resetpos=false)
    {
        if (resetpos)
            servo_resetpos();

        if (last_pos == 'R') {
            Serial.println("Position is already on the right");
            return;
        }

        servo->write(180+SERVO_OFFSET);
        delay(15);
        last_pos = 'R';
    }

    void servo_steerLeft(bool resetpos=false)
    {
        if (resetpos)
            servo_resetpos();

        if (last_pos == 'L') {
            Serial.println("Position is already on the left");
            return;
        }

        servo->write(0+SERVO_OFFSET);
        delay(15);
        last_pos = 'l';
    }
};
