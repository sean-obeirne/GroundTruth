# GroundTruth Usage Guide

## Setup

Plug the Pro Micro into USB. Solder or attach leads to these pins:

```
Pro Micro Pin    Label    What it's for
─────────────    ─────    ──────────────────────────
A0               P1       Probe input (digital + analog)
A1               P2       Probe input (digital + analog)
A2               P3       Probe input (digital + analog)
D6               O1       Output driver
D5               O2       Output driver
A3               R3V      3.3V rail sense
A4               R5V      5V rail sense
GND              GND      Common ground (MUST connect to target GND)
```

Wire R3V and R5V to the power rails on the boards you debug most often,
or leave them free and clip on per-session.

P1–P3 and O1–O2 are your general-purpose probes. Use clip leads, dupont
wires, or whatever gets the job done.

**Important:** Always connect GND between the Pro Micro and your target board.
Without a common ground, all readings are garbage.


## First check: is it alive?

```
gt ping
```

If you get `PONG`, you're good. If not:
- Is it plugged in? `gt --list` shows available ports.
- Wrong port? `gt --port /dev/ttyACM1 ping`
- No port at all? Check `ls /dev/ttyACM*`. If nothing, try a different USB cable.


## Checking power

This is usually the first thing to do when a board isn't working.

```
gt rails
```

Output looks like:
```
  3V3: 3280 mV
  5V: 4950 mV
```

What to look for:
- **Both rails read close to 0:** Board has no power. Check the USB cable, battery, or power supply.
- **5V is fine, 3.3V is 0 or very low:** The 3.3V regulator is dead or shorted.
- **Voltages are low but not zero (e.g. 2.1V on the 3.3V rail):** Something is loading the rail down — possible short, bad component, or excessive current draw.
- **Rails are floating/noisy:** You forgot to connect GND, or the sense wire isn't making contact.


## Reading a signal

Digital read — is a pin high or low?
```
gt read p1
```

Analog read — what's the actual voltage?
```
gt voltage p1
```

Use `read` when you care about logic levels (is this pin being driven?).
Use `voltage` when you need to see what's actually there (pull-up voltage?
brownout? divided signal?).


## Checking if a signal is alive

If you're trying to figure out whether a clock, data line, or PWM signal is
actually toggling:

```
gt watch p1 1000
```

This watches P1 for 1 second and reports whether it saw any transitions.
Output:
```
P1: ACTIVE (42 edges)
```
or:
```
P1: IDLE (0 edges)
```

If you need the actual edge count:
```
gt edges p1 1000
```

### What this is good for

- Is the crystal oscillator running?
- Is the SPI clock toggling during a transaction?
- Is this PWM output actually doing anything?
- Is the I2C bus active?

You won't get the frequency or decode the protocol — but you'll know
instantly if the line is dead or alive.


## Driving outputs

Set a pin high or low:
```
gt set o1 high
gt set o1 low
```

Generate a burst of pulses (count + half-period in microseconds):
```
gt pulse o1 10 500
```
That sends 10 pulses with 500µs high / 500µs low (1kHz).

### What this is good for

- Pull a reset line low to force a reset
- Drive an enable pin to see if a chip wakes up
- Fake a clock signal to step through something manually
- Inject a signal and see if the downstream responds


## Testing a button

Wire the button between P1 and GND (the firmware enables a pull-up
internally):

```
gt button-test p1 5000
```

Press the button a few times during the 5-second window. Output:
```
P1: 3 presses detected
```

This does basic debouncing. Good for verifying:
- Does the button make contact at all?
- Is it bouncing so badly that one press registers as many?
- Is the pull-up working?


## Testing a rotary encoder

Wire encoder A to P1, encoder B to P2, common to GND:

```
gt encoder-test p1 p2 5000
```

Turn the encoder during the 5-second window. Output:
```
Encoder delta: 12 steps
```

Positive = one direction, negative = the other. Good for verifying:
- Is the encoder working at all?
- Are A and B swapped? (direction will be reversed)
- Is it mechanically damaged? (erratic counts)


## Self-test with a loopback

Connect O1 directly to P1 with a jumper wire. Then:

```
gt set o1 high
gt read p1         # should say HIGH

gt set o1 low
gt read p1         # should say LOW

gt voltage p1      # should read ~0 mV or ~5000 mV
```

This validates the entire signal chain without any target board.


## Using raw commands

If you want to talk to the firmware directly (or use a serial monitor):

```
gt raw PING
gt raw READ P1
gt raw VOLTAGE P2
```

This prints the raw protocol response with no formatting. Useful for
scripting or debugging the tool itself.

You can also open a serial monitor directly:
```
picocom -b 115200 /dev/ttyACM0
```
And type commands by hand. The protocol is plain text — no binary framing,
no escaping, nothing clever.


## Common debugging playbook

### "The board is totally dead"

```
gt rails                  # Is there power?
gt voltage p1             # Probe VCC and GND on the target
gt read p1                # Check reset pin — is it held low?
gt set o1 high            # Try driving the reset pin high
```

### "It powers on but nothing happens"

```
gt rails                  # Rails look okay?
gt watch p1 1000          # Probe the crystal/oscillator — is it running?
gt watch p2 1000          # Check the main clock output
gt read p1                # Check boot config pins
```

### "Communication isn't working"

```
gt watch p1 500           # Probe SCL/SCK — any clock activity?
gt watch p2 500           # Probe SDA/MOSI — any data?
gt edges p1 1000          # Count clock edges during a transaction
```

### "The output should be doing something but isn't"

```
gt read p1                # Is the output pin high or low?
gt voltage p1             # What's the actual voltage?
gt watch p1 2000          # Is it toggling at all?
gt set o1 high            # Drive it externally and see if downstream responds
```

### "I want to force a chip to reset"

```
gt set o1 low             # Pull reset low (connect O1 to reset pin)
gt set o1 high            # Release — chip should boot
gt watch p1 1000          # Watch for activity after reset
```


## Tips

- **Always connect GND first.** Nothing works without it. Floating ground = fantasy readings.
- **Voltage readings are 0–5V only.** The Pro Micro's ADC uses a 5V reference. Anything above 5V will read as 5000 mV and could damage the pin.
- **Don't probe voltages above 5V** without a resistor divider. The ATmega32U4 inputs are not 12V tolerant.
- **Don't drive O1/O2 into a pin that's already being driven.** You could short two outputs against each other. Use a series resistor (220Ω–1kΩ) if you're not sure.
- **The probe pins are high-impedance inputs by default.** They won't load down the signal you're measuring.
- **Edge counting is software-polled**, so it won't catch very fast signals (above ~50kHz). But it's plenty for I2C, SPI at moderate speeds, PWM, etc.
- **Blocking commands (watch, edges, button-test, encoder-test) tie up the probe** for the specified duration. The probe won't respond to other commands until the measurement finishes.
