class Motor {
    private:
    uint8_t outx;
    uint8_t outy;
    uint8_t enx;
    uint8_t current_running_speed;
    bool motor_running;

    public:
    Motor(uint8_t motor_outx, uint8_t motor_outy, uint8_t motor_enx) {
        outy = motor_outx;
        outx = motor_outy;
        enx = motor_enx;
        motor_running = false;

        pinMode(motor_enx, OUTPUT);
        pinMode(motor_outx, OUTPUT);
        pinMode(motor_outy, OUTPUT);
    }

    void start_motor(uint8_t speed, bool inverse = false)
    {
        uint8_t tmpOutx;
        uint8_t tmpOuty;

        if (motor_running && speed == current_running_speed) {
            Serial.println("Motor is already running!");
            return;
        }

        Serial.print("Writing ");
        Serial.print(speed);
        Serial.print(" to pin ");
        Serial.println(enx);

        if (inverse) {
            tmpOutx = outy;
            tmpOuty = outx;
        } else {
            tmpOutx = outx;
            tmpOuty = outy;
        }

        Serial.println(tmpOutx);
        Serial.println(tmpOuty);

        analogWrite(enx, speed);
        digitalWrite(tmpOutx, 1);
        digitalWrite(tmpOuty, 0);
        motor_running = true;
        current_running_speed = speed;

    }

    void halt_motor()
    {
        if (!motor_running) {
            Serial.println("Motor is already stopped!");
            return;
        }

        digitalWrite(outx, 0);
        digitalWrite(outy, 0);
        motor_running = false;
    }

    bool is_motor_running()
    {
        return motor_running;
    }
};
