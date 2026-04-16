/*
 * GroundTruth - Timing and activity measurement helpers
 */
#ifndef GT_TIMING_H
#define GT_TIMING_H

#include <stdint.h>
#include "pins.h"

/* millis() is provided by the Arduino core (Timer0-based).
 * No timing_init() needed — Arduino core handles it. */
uint32_t gt_millis(void);

/* Count rising+falling edges on a pin over duration_ms.
 * Blocking call. Returns total edge count. */
uint16_t edges_count(pin_id_t id, uint16_t duration_ms);

/* Monitor a pin for any activity over duration_ms.
 * Returns edge count (0 = idle). Blocking. */
uint16_t watch_pin(pin_id_t id, uint16_t duration_ms);

/* Generate count pulses on an output pin with half-period delay_us.
 * Blocking call. */
void pulse_train(pin_id_t id, uint16_t count, uint16_t delay_us);

/* Button test: count debounced presses over duration_ms.
 * Active-low assumed (pullup enabled). Blocking. */
uint16_t button_test(pin_id_t id, uint16_t duration_ms);

/* Encoder test: track quadrature delta over duration_ms.
 * Returns signed delta. Blocking. */
int16_t encoder_test(pin_id_t pin_a, pin_id_t pin_b, uint16_t duration_ms);

#endif /* GT_TIMING_H */
