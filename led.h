#include <utility/wifi_drv.h>

#define RED_LED 25
#define GREEN_LED 26
#define BLUE_LED 27

struct prev_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

bool led_inited = false;
struct prev_color pc;

bool set_color(uint8_t red, uint8_t green, uint8_t blue);


void init_led()
{
    WiFiDrv::pinMode(RED_LED, OUTPUT);
    WiFiDrv::pinMode(GREEN_LED, OUTPUT);
    WiFiDrv::pinMode(BLUE_LED, OUTPUT);

    pc.red = 0;
    pc.green = 0;
    pc.blue = 0;
}

bool set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if (!led_inited)
        init_led();

    if (red > 255 || green > 255 || blue > 255)
        return false;

    if (pc.red == red && pc.green == green && pc.blue == blue)
        return false;

    WiFiDrv::analogWrite(RED_LED, red);
    WiFiDrv::analogWrite(GREEN_LED, green);
    WiFiDrv::analogWrite(BLUE_LED, blue);

    pc.red = red;
    pc.green = green;
    pc.blue = blue;
    return true;
}
