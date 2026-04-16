/*
 * GroundTruth Firmware - Main entry point
 * Pro Micro (ATmega32U4) USB-connected debugging probe
 *
 * Uses the Arduino setup/loop pattern so Arduino's init() runs first
 * and properly initializes USB. The .ino file calls gt_setup/gt_loop.
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pins.h"
#include "adc.h"
#include "timing.h"
#include "command.h"

/* Provided by usb_serial.cpp (wraps Arduino Serial) */
void usb_serial_putchar(char c);
int16_t usb_serial_getchar(void);
uint8_t usb_serial_available(void);
void usb_serial_init(void);

/* Board status LED on Pro Micro: pin 17 / PB0 (RX LED) */
#define LED_DDR   DDRB
#define LED_PORT  PORTB
#define LED_BIT   PB0

static void led_init(void) {
    LED_DDR |= (1 << LED_BIT);
    LED_PORT |= (1 << LED_BIT);  /* LED off (active low on Pro Micro) */
}

static void led_toggle(void) {
    LED_PORT ^= (1 << LED_BIT);
}

void gt_setup(void) {
    led_init();
    adc_init();
    command_init();
    usb_serial_init();

    /* Quick startup blink */
    for (uint8_t i = 0; i < 6; i++) {
        led_toggle();
        for (volatile uint16_t d = 0; d < 30000; d++) ;
    }

    /* Configure default pin directions */
    pin_set_input(PIN_P1);
    pin_set_input(PIN_P2);
    pin_set_input(PIN_P3);
    pin_set_input(PIN_R3V);
    pin_set_input(PIN_R5V);
    pin_set_output(PIN_O1);
    pin_set_output(PIN_O2);
    pin_digital_write(PIN_O1, 0);
    pin_digital_write(PIN_O2, 0);
}

void gt_loop(void) {
    int16_t c = usb_serial_getchar();
    if (c >= 0) {
        command_feed((char)c);
    }
}
