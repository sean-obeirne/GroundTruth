# GroundTruth

A host-controlled microcontroller debugging multitool. Uses a Pro Micro (ATmega32U4) as a USB-connected probe engine and a Go CLI on the host for user interaction.

## Architecture

```
┌─────────────┐   USB Serial (115200)   ┌──────────────┐
│  Host (Go)  │ ◄────────────────────► │  Pro Micro   │
│  CLI: gt    │   Line protocol         │  Firmware    │
└─────────────┘                         └──────┬───────┘
                                               │ GPIO/ADC
                                        ┌──────┴───────┐
                                        │ Target Board │
                                        └──────────────┘
```

**Firmware** handles: GPIO, ADC, timing, edge counting, command parsing, responses.
**Host** handles: CLI UX, port discovery, output formatting, workflow composition.

## Protocol

Line-oriented, human-readable, `\n`-terminated. Testable with any serial monitor.

```
→ PING
← OK PONG

→ READ P1
← OK READ P1 HIGH

→ VOLTAGE P2
← OK VOLTAGE P2 3320

→ SET O1 HIGH
← OK SET O1 HIGH

→ RAILS
← OK RAILS 3V3=3280 5V=4950

→ EDGES P1 1000
← OK EDGES P1 42

→ WATCH P1 500
← OK WATCH P1 ACTIVE 12

→ PULSE O1 5 1000
← OK PULSE O1 5

→ BUTTON_TEST P1 5000
← OK BUTTON_TEST P1 3

→ ENCODER_TEST P1 P2 5000
← OK ENCODER_TEST 12

→ MAP
← OK MAP P1=A0 P2=A1 P3=A2 O1=D6 O2=D5 R3V=A3 R5V=A4
```

## Pin Mapping (V1)

| Logical | Arduino Pin | ATmega32U4 | Function |
|---------|-------------|------------|----------|
| P1 | A0 | PF7 / ADC7 | Probe input (analog + digital) |
| P2 | A1 | PF6 / ADC6 | Probe input (analog + digital) |
| P3 | A2 | PF5 / ADC5 | Probe input (analog + digital) |
| O1 | D6 | PD7 | Output driver |
| O2 | D5 | PC6 | Output driver |
| R3V | A3 | PF4 / ADC4 | 3.3V rail sense |
| R5V | A4 | PF1 / ADC1 | 5V rail sense |

## Directory Structure

```
firmware/           Pro Micro firmware (C, Arduino build)
  firmware.ino      Arduino sketch wrapper
  src/
    main.c          Entry point
    command.c/.h    Serial command parser + dispatcher
    pins.c/.h       Pin mapping and GPIO
    adc.c/.h        ADC voltage reading
    timing.c/.h     Timing, edge counting, pulse generation
    usb_serial.cpp  Arduino USB CDC bridge
  Makefile          arduino-cli build

host/               Host CLI application (Go)
  main.go           CLI entry point
  cmd/              Command handlers
  serial/           Serial port transport
  protocol/         Response parsing
```

## Building

### Prerequisites

**Firmware:**
```bash
# Install arduino-cli
# Linux
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Add SparkFun board package
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/sparkfun/Arduino_Boards/main/IDE_Board_Manager/package_sparkfun_index.json
arduino-cli core update-index
arduino-cli core install SparkFun:avr
```

**Host:**
```bash
# Go 1.21+
go version
```

### Build Firmware

```bash
cd firmware
make build               # compile
make upload PORT=/dev/ttyACM0  # flash to Pro Micro
```

### Build Host CLI

```bash
cd host
go build -o gt .
# optionally install to PATH:
go install .
```

## Usage

```bash
# Auto-detect probe
gt ping
gt read p1
gt voltage p1
gt set o1 high
gt rails

# Specify port
gt --port /dev/ttyACM0 ping

# List available ports
gt --list

# Monitor activity
gt watch p1 1000
gt edges p1 2000

# Output pulses
gt pulse o1 10 500

# Specialized tests
gt button-test p1 5000
gt encoder-test p1 p2 5000

# Raw command (bypass formatting)
gt raw PING

# Show pin mapping
gt map
```

## Testing

### 1. Firmware Alone (Serial Monitor)

Flash the firmware, then use any serial monitor at 115200 baud:

```bash
# arduino-cli built-in monitor
make monitor

# or screen / minicom / picocom
picocom -b 115200 /dev/ttyACM0
```

Type commands directly:
```
PING
READ P1
VOLTAGE P1
RAILS
SET O1 HIGH
SET O1 LOW
```

You should see `OK ...` or `ERR ...` responses. This is the fastest way to validate firmware changes.

### 2. Host CLI Against Device

```bash
cd host && go build -o gt .
./gt ping           # should print PONG
./gt voltage p1     # touch P1 wire to 3.3V and verify
./gt rails          # verify rail sense pins
```

### 3. Loopback Tests

Connect O1 to P1 with a jumper wire:
```bash
./gt set o1 high
./gt read p1        # should show HIGH
./gt set o1 low
./gt read p1        # should show LOW
./gt pulse o1 10 1000
# Then in another terminal:
./gt edges p1 2000  # should detect edges
```

### 4. Debugging Workflow

1. **Start simple:** `PING` first — if this doesn't work, the issue is USB/serial
2. **Check rails:** `RAILS` to verify power to your target board
3. **Probe signals:** `READ` and `VOLTAGE` to check individual pins
4. **Check activity:** `WATCH` and `EDGES` to see if a signal is toggling
5. **Drive outputs:** `SET` and `PULSE` to inject signals
6. **Specialized:** `BUTTON_TEST` and `ENCODER_TEST` for specific component testing

## Next Steps

- [ ] WAVE / WAVE_STOP for continuous square wave generation
- [ ] UART listen mode (passive serial sniffer)
- [ ] Configurable voltage reference for non-5V targets
- [ ] TUI mode (bubbletea or tcell)
- [ ] Continuous monitoring mode with streaming output
- [ ] Pin state snapshot (read all pins at once)
- [ ] Frequency measurement mode
