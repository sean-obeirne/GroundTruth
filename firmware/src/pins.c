/*
 * GroundTruth - Pin mapping and GPIO helpers
 * Pro Micro (ATmega32U4)
 */
#include "pins.h"
#include <string.h>

/*
 * ATmega32U4 Pro Micro pin mapping:
 *   A0 = PF7 (ADC7)    A1 = PF6 (ADC6)    A2 = PF5 (ADC5)
 *   A3 = PF4 (ADC4)    A4 = PF1 (ADC1)
 *   D5 = PC6            D6 = PD7
 */
const pin_def_t pin_table[PIN_COUNT] = {
    [PIN_P1]  = { &DDRF, &PORTF, &PINF, PF7, 7,    PIN_CAP_INPUT | PIN_CAP_ANALOG, "P1"  },
    [PIN_P2]  = { &DDRF, &PORTF, &PINF, PF6, 6,    PIN_CAP_INPUT | PIN_CAP_ANALOG, "P2"  },
    [PIN_P3]  = { &DDRF, &PORTF, &PINF, PF5, 5,    PIN_CAP_INPUT | PIN_CAP_ANALOG, "P3"  },
    [PIN_O1]  = { &DDRD, &PORTD, &PIND, PD7, 0xFF, PIN_CAP_OUTPUT,                 "O1"  },
    [PIN_O2]  = { &DDRC, &PORTC, &PINC, PC6, 0xFF, PIN_CAP_OUTPUT,                 "O2"  },
    [PIN_R3V] = { &DDRF, &PORTF, &PINF, PF4, 4,    PIN_CAP_INPUT | PIN_CAP_ANALOG, "R3V" },
    [PIN_R5V] = { &DDRF, &PORTF, &PINF, PF1, 1,    PIN_CAP_INPUT | PIN_CAP_ANALOG, "R5V" },
};

pin_id_t pin_parse(const char *name) {
    if (!name) return PIN_INVALID;
    for (uint8_t i = 0; i < PIN_COUNT; i++) {
        if (strcasecmp(name, pin_table[i].name) == 0) {
            return (pin_id_t)i;
        }
    }
    return PIN_INVALID;
}

const char *pin_name(pin_id_t id) {
    if (id >= PIN_COUNT) return "??";
    return pin_table[id].name;
}

void pin_set_input(pin_id_t id) {
    if (id >= PIN_COUNT) return;
    const pin_def_t *p = &pin_table[id];
    *p->ddr  &= ~(1 << p->bit);  /* direction = input */
    *p->port &= ~(1 << p->bit);  /* pullup off */
}

void pin_set_output(pin_id_t id) {
    if (id >= PIN_COUNT) return;
    const pin_def_t *p = &pin_table[id];
    *p->ddr |= (1 << p->bit);
}

uint8_t pin_digital_read(pin_id_t id) {
    if (id >= PIN_COUNT) return 0;
    const pin_def_t *p = &pin_table[id];
    return (*p->pin_reg >> p->bit) & 1;
}

void pin_digital_write(pin_id_t id, uint8_t val) {
    if (id >= PIN_COUNT) return;
    const pin_def_t *p = &pin_table[id];
    if (val)
        *p->port |= (1 << p->bit);
    else
        *p->port &= ~(1 << p->bit);
}

bool pin_has_cap(pin_id_t id, uint8_t cap) {
    if (id >= PIN_COUNT) return false;
    return (pin_table[id].caps & cap) != 0;
}
