/*
 * GroundTruth - Arduino USB serial bridge
 *
 * This file provides the USB serial functions expected by our C code
 * using the Arduino Serial (CDC) interface. Compile this as C++ since
 * Arduino Serial is a C++ class.
 */

#include <Arduino.h>

extern "C"
{
    void usb_serial_init(void);
    void usb_serial_putchar(char c);
    int16_t usb_serial_getchar(void);
    uint8_t usb_serial_available(void);
}

void usb_serial_init(void)
{
    Serial.begin(115200);
    /* Wait up to 2 seconds for USB connection */
    uint32_t start = millis();
    while (!Serial && (millis() - start < 2000))
        ;
}

void usb_serial_putchar(char c)
{
    Serial.write(c);
}

int16_t usb_serial_getchar(void)
{
    if (Serial.available() > 0)
    {
        return Serial.read();
    }
    return -1;
}

uint8_t usb_serial_available(void)
{
    return Serial.available() > 0 ? 1 : 0;
}
