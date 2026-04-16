/*
 * GroundTruth - ADC helpers
 * ATmega32U4 10-bit ADC, AVCC reference (5V)
 */
#ifndef GT_ADC_H
#define GT_ADC_H

#include <stdint.h>
#include "pins.h"

/* Initialize the ADC peripheral */
void adc_init(void);

/* Read the ADC channel for a given pin, returns raw 10-bit value.
 * Returns 0xFFFF if pin doesn't support analog. */
uint16_t adc_read_raw(pin_id_t id);

/* Read voltage in millivolts (0–5000 for 5V ref) */
uint16_t adc_read_mv(pin_id_t id);

#endif /* GT_ADC_H */
