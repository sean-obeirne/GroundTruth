# GroundTruth Firmware - Arduino CLI build
#
# Prerequisites:
#   brew install arduino-cli   (or apt/pacman equivalent)
#   arduino-cli core install arduino:avr
#
# The Pro Micro uses the SparkFun board package:
#   arduino-cli core install SparkFun:avr \
#     --additional-urls https://raw.githubusercontent.com/sparkfun/Arduino_Boards/main/IDE_Board_Manager/package_sparkfun_index.json

FQBN    ?= SparkFun:avr:promicro:cpu=16MHzatmega32U4
PORT    ?= /dev/ttyACM0
SKETCH  := .

.PHONY: build upload monitor clean

build:
	arduino-cli compile --fqbn $(FQBN) $(SKETCH)

upload: build
	arduino-cli upload --fqbn $(FQBN) --port $(PORT) $(SKETCH)

monitor:
	arduino-cli monitor --port $(PORT) --config baudrate=115200

clean:
	rm -rf build/
