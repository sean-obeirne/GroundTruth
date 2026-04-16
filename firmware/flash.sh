#!/bin/bash
# flash.sh — catch the Pro Micro bootloader and flash GroundTruth firmware
# Usage: ./flash.sh [hex_file]
#
# 1. Run this script
# 2. Short RST→GND twice on the Pro Micro
# 3. Script detects the port and flashes immediately

HEX="${1:-$(find ~/.cache/arduino/sketches -name 'firmware.ino.hex' 2>/dev/null | head -1)}"

if [ -z "$HEX" ]; then
    echo "ERROR: No hex file found. Run 'make build' first."
    exit 1
fi

echo "Hex file: $HEX"
echo "Waiting for bootloader... Short RST→GND twice NOW!"

while true; do
    PORT=$(ls /dev/ttyACM* 2>/dev/null | head -1)
    if [ -n "$PORT" ]; then
        echo "Found $PORT — flashing!"
        avrdude -v -p atmega32u4 -c avr109 -P "$PORT" -b 57600 -D -U flash:w:"$HEX":i
        exit $?
    fi
    sleep 0.05
done
