/*
 * GroundTruth - Timing and activity measurement helpers
 */
#include "timing.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Use Arduino core's millis() — declared in Arduino.h but we're in plain C */
extern unsigned long millis(void);

uint32_t gt_millis(void) {
    return (uint32_t)millis();
}

uint16_t edges_count(pin_id_t id, uint16_t duration_ms) {
    pin_set_input(id);
    uint16_t count = 0;
    uint8_t last = pin_digital_read(id);
    uint32_t start = gt_millis();

    while ((gt_millis() - start) < duration_ms) {
        uint8_t cur = pin_digital_read(id);
        if (cur != last) {
            count++;
            last = cur;
        }
    }
    return count;
}

uint16_t watch_pin(pin_id_t id, uint16_t duration_ms) {
    /* Same as edges_count — activity = non-zero edges */
    return edges_count(id, duration_ms);
}

void pulse_train(pin_id_t id, uint16_t count, uint16_t delay_us) {
    pin_set_output(id);
    for (uint16_t i = 0; i < count; i++) {
        pin_digital_write(id, 1);
        /* _delay_us needs compile-time constant; use loop for variable delay */
        for (uint16_t d = 0; d < delay_us; d++)
            _delay_us(1);
        pin_digital_write(id, 0);
        for (uint16_t d = 0; d < delay_us; d++)
            _delay_us(1);
    }
}

uint16_t button_test(pin_id_t id, uint16_t duration_ms) {
    /* Enable pullup for button detection */
    const pin_def_t *p = &pin_table[id];
    *p->ddr &= ~(1 << p->bit);   /* input */
    *p->port |= (1 << p->bit);   /* pullup on */

    uint16_t presses = 0;
    uint8_t last = 1;  /* idle high with pullup */
    uint8_t debounce = 0;
    uint32_t start = gt_millis();

    while ((gt_millis() - start) < duration_ms) {
        uint8_t cur = pin_digital_read(id);
        if (cur != last) {
            /* Simple debounce: wait ~20ms before accepting */
            if (++debounce >= 20) {
                if (cur == 0) {  /* active low press */
                    presses++;
                }
                last = cur;
                debounce = 0;
            }
            _delay_ms(1);
        } else {
            debounce = 0;
        }
    }

    /* Restore no-pullup state */
    *p->port &= ~(1 << p->bit);
    return presses;
}

int16_t encoder_test(pin_id_t pin_a, pin_id_t pin_b, uint16_t duration_ms) {
    /* Enable pullups on encoder pins */
    const pin_def_t *pa = &pin_table[pin_a];
    const pin_def_t *pb = &pin_table[pin_b];
    *pa->ddr &= ~(1 << pa->bit); *pa->port |= (1 << pa->bit);
    *pb->ddr &= ~(1 << pb->bit); *pb->port |= (1 << pb->bit);

    int16_t delta = 0;
    uint8_t last_a = pin_digital_read(pin_a);
    uint8_t last_b = pin_digital_read(pin_b);
    uint32_t start = gt_millis();

    while ((gt_millis() - start) < duration_ms) {
        uint8_t a = pin_digital_read(pin_a);
        uint8_t b = pin_digital_read(pin_b);
        if (a != last_a) {
            /* A changed: determine direction from B */
            if (a != b)
                delta++;
            else
                delta--;
            last_a = a;
        }
        if (b != last_b) {
            last_b = b;
        }
    }

    /* Restore no-pullup */
    *pa->port &= ~(1 << pa->bit);
    *pb->port &= ~(1 << pb->bit);
    return delta;
}
