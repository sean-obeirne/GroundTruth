/*
 * GroundTruth - ADC helpers
 * ATmega32U4 10-bit ADC, AVCC reference
 */
#include "adc.h"
#include <avr/io.h>

void adc_init(void) {
    /* AVCC reference, right-adjusted result */
    ADMUX = (1 << REFS0);
    /* Enable ADC, prescaler /128 for 16MHz -> 125kHz ADC clock */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    /* Disable digital input buffers on ADC pins to save power */
    DIDR0 = 0xFF;
}

uint16_t adc_read_raw(pin_id_t id) {
    if (id >= PIN_COUNT) return 0xFFFF;
    uint8_t ch = pin_table[id].adc_channel;
    if (ch == 0xFF) return 0xFFFF;

    /* Select channel, keep AVCC reference */
    ADMUX = (1 << REFS0) | (ch & 0x07);
    /* MUX5 for channels >= 8 (not needed for our pins, but future-proof) */
    if (ch & 0x08)
        ADCSRB |= (1 << MUX5);
    else
        ADCSRB &= ~(1 << MUX5);

    /* Start conversion */
    ADCSRA |= (1 << ADSC);
    /* Wait for completion */
    while (ADCSRA & (1 << ADSC))
        ;

    return ADC;  /* reads ADCL then ADCH */
}

uint16_t adc_read_mv(pin_id_t id) {
    uint16_t raw = adc_read_raw(id);
    if (raw == 0xFFFF) return 0xFFFF;
    /* (raw * 5000) / 1023 — use 32-bit intermediate to avoid overflow */
    return (uint16_t)(((uint32_t)raw * 5000UL) / 1023UL);
}
