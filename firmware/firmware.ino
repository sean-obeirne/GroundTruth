/*
 * GroundTruth.ino — Arduino sketch wrapper
 *
 * All real logic lives in src/*.c. This file bridges to the
 * Arduino setup/loop pattern so init() runs first and USB enumerates.
 */

extern "C" void gt_setup(void);
extern "C" void gt_loop(void);

void setup() { gt_setup(); }
void loop() { gt_loop(); }
