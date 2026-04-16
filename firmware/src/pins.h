/*
 * GroundTruth - Pin mapping and GPIO helpers
 * Pro Micro (ATmega32U4)
 */
#ifndef GT_PINS_H
#define GT_PINS_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/* Logical pin IDs used in the serial protocol */
typedef enum {
    PIN_P1 = 0,  /* Probe 1: A0 / PF7 */
    PIN_P2,      /* Probe 2: A1 / PF6 */
    PIN_P3,      /* Probe 3: A2 / PF5 */
    PIN_O1,      /* Output 1: D6 / PD7 */
    PIN_O2,      /* Output 2: D5 / PC6 */
    PIN_R3V,     /* 3.3V rail sense: A3 / PF4 */
    PIN_R5V,     /* 5V rail sense: A4 / PF1 */
    PIN_COUNT,
    PIN_INVALID = 0xFF
} pin_id_t;

/* Pin capability flags */
#define PIN_CAP_INPUT   0x01
#define PIN_CAP_OUTPUT  0x02
#define PIN_CAP_ANALOG  0x04

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin_reg;
    uint8_t bit;
    uint8_t adc_channel;  /* 0xFF if no ADC */
    uint8_t caps;
    const char *name;
} pin_def_t;

/* Pin definitions table */
extern const pin_def_t pin_table[PIN_COUNT];

/* Parse a pin name string like "P1", "O2", "R3V" to pin_id_t */
pin_id_t pin_parse(const char *name);

/* Get the name string for a pin */
const char *pin_name(pin_id_t id);

/* Configure pin as input with pullup disabled */
void pin_set_input(pin_id_t id);

/* Configure pin as output */
void pin_set_output(pin_id_t id);

/* Read digital state: returns 1 (HIGH) or 0 (LOW) */
uint8_t pin_digital_read(pin_id_t id);

/* Write digital output */
void pin_digital_write(pin_id_t id, uint8_t val);

/* Check if pin has a capability */
bool pin_has_cap(pin_id_t id, uint8_t cap);

#endif /* GT_PINS_H */
