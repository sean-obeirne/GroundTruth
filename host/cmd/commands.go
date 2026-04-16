// Package cmd implements CLI commands for the GroundTruth host tool.
package cmd

import (
	"fmt"
	"os"
	"strings"

	gtserial "github.com/seanvelasco/groundtruth/serial"
	"github.com/seanvelasco/groundtruth/protocol"
)

// Context holds shared state for all commands.
type Context struct {
	Conn *gtserial.Conn
}

// Run sends a raw command, parses the response, and prints it.
// Returns the parsed response or an error.
func (ctx *Context) Run(command string) (*protocol.Response, error) {
	raw, err := ctx.Conn.Command(command)
	if err != nil {
		return nil, fmt.Errorf("serial: %w", err)
	}

	resp, err := protocol.Parse(raw)
	if err != nil {
		return nil, fmt.Errorf("parse: %w (raw: %q)", err, raw)
	}

	if !resp.OK {
		return resp, fmt.Errorf("probe error: %s", resp.Error())
	}

	return resp, nil
}

// RunDefs is the command table. Each entry maps a CLI verb to its handler.
var RunDefs = map[string]func(ctx *Context, args []string) error{
	"ping":         cmdPing,
	"read":         cmdRead,
	"voltage":      cmdVoltage,
	"set":          cmdSet,
	"watch":        cmdWatch,
	"edges":        cmdEdges,
	"rails":        cmdRails,
	"pulse":        cmdPulse,
	"button-test":  cmdButtonTest,
	"encoder-test": cmdEncoderTest,
	"map":          cmdMap,
	"raw":          cmdRaw,
}

func cmdPing(ctx *Context, args []string) error {
	resp, err := ctx.Run("PING")
	if err != nil {
		return err
	}
	fmt.Println(resp.Field(0)) // PONG
	return nil
}

func cmdRead(ctx *Context, args []string) error {
	if len(args) < 1 {
		return fmt.Errorf("usage: read <pin>")
	}
	resp, err := ctx.Run("READ " + strings.ToUpper(args[0]))
	if err != nil {
		return err
	}
	// Fields: READ P1 HIGH
	fmt.Printf("%s: %s\n", resp.Field(1), resp.Field(2))
	return nil
}

func cmdVoltage(ctx *Context, args []string) error {
	if len(args) < 1 {
		return fmt.Errorf("usage: voltage <pin>")
	}
	resp, err := ctx.Run("VOLTAGE " + strings.ToUpper(args[0]))
	if err != nil {
		return err
	}
	// Fields: VOLTAGE P1 3320
	fmt.Printf("%s: %s mV\n", resp.Field(1), resp.Field(2))
	return nil
}

func cmdSet(ctx *Context, args []string) error {
	if len(args) < 2 {
		return fmt.Errorf("usage: set <pin> <HIGH|LOW>")
	}
	cmd := fmt.Sprintf("SET %s %s", strings.ToUpper(args[0]), strings.ToUpper(args[1]))
	resp, err := ctx.Run(cmd)
	if err != nil {
		return err
	}
	fmt.Printf("%s -> %s\n", resp.Field(1), resp.Field(2))
	return nil
}

func cmdWatch(ctx *Context, args []string) error {
	if len(args) < 2 {
		return fmt.Errorf("usage: watch <pin> <duration_ms>")
	}
	cmd := fmt.Sprintf("WATCH %s %s", strings.ToUpper(args[0]), args[1])
	resp, err := ctx.Run(cmd)
	if err != nil {
		return err
	}
	// Fields: WATCH P1 ACTIVE 42
	fmt.Printf("%s: %s (%s edges)\n", resp.Field(1), resp.Field(2), resp.Field(3))
	return nil
}

func cmdEdges(ctx *Context, args []string) error {
	if len(args) < 2 {
		return fmt.Errorf("usage: edges <pin> <duration_ms>")
	}
	cmd := fmt.Sprintf("EDGES %s %s", strings.ToUpper(args[0]), args[1])
	resp, err := ctx.Run(cmd)
	if err != nil {
		return err
	}
	// Fields: EDGES P1 42
	fmt.Printf("%s: %s edges\n", resp.Field(1), resp.Field(2))
	return nil
}

func cmdRails(ctx *Context, args []string) error {
	resp, err := ctx.Run("RAILS")
	if err != nil {
		return err
	}
	// Fields: RAILS 3V3=3280 5V=4950
	for _, f := range resp.Fields {
		parts := strings.SplitN(f, "=", 2)
		if len(parts) == 2 {
			fmt.Printf("  %s: %s mV\n", parts[0], parts[1])
		}
	}
	return nil
}

func cmdPulse(ctx *Context, args []string) error {
	if len(args) < 3 {
		return fmt.Errorf("usage: pulse <pin> <count> <delay_us>")
	}
	cmd := fmt.Sprintf("PULSE %s %s %s", strings.ToUpper(args[0]), args[1], args[2])
	resp, err := ctx.Run(cmd)
	if err != nil {
		return err
	}
	fmt.Printf("%s: %s pulses sent\n", resp.Field(1), resp.Field(2))
	return nil
}

func cmdButtonTest(ctx *Context, args []string) error {
	if len(args) < 2 {
		return fmt.Errorf("usage: button-test <pin> <duration_ms>")
	}
	cmd := fmt.Sprintf("BUTTON_TEST %s %s", strings.ToUpper(args[0]), args[1])
	fmt.Printf("Listening for button presses on %s for %s ms...\n",
		strings.ToUpper(args[0]), args[1])
	resp, err := ctx.Run(cmd)
	if err != nil {
		return err
	}
	fmt.Printf("%s: %s presses detected\n", resp.Field(1), resp.Field(2))
	return nil
}

func cmdEncoderTest(ctx *Context, args []string) error {
	if len(args) < 3 {
		return fmt.Errorf("usage: encoder-test <pinA> <pinB> <duration_ms>")
	}
	cmd := fmt.Sprintf("ENCODER_TEST %s %s %s",
		strings.ToUpper(args[0]), strings.ToUpper(args[1]), args[2])
	fmt.Printf("Tracking encoder on %s/%s for %s ms...\n",
		strings.ToUpper(args[0]), strings.ToUpper(args[1]), args[2])
	resp, err := ctx.Run(cmd)
	if err != nil {
		return err
	}
	fmt.Printf("Encoder delta: %s steps\n", resp.Field(1))
	return nil
}

func cmdMap(ctx *Context, args []string) error {
	resp, err := ctx.Run("MAP")
	if err != nil {
		return err
	}
	// Fields: MAP P1=A0 P2=A1 ...
	fmt.Println("Pin mapping:")
	for _, f := range resp.Fields {
		if f == "MAP" {
			continue
		}
		parts := strings.SplitN(f, "=", 2)
		if len(parts) == 2 {
			fmt.Printf("  %-4s -> %s\n", parts[0], parts[1])
		}
	}
	return nil
}

func cmdRaw(ctx *Context, args []string) error {
	if len(args) < 1 {
		return fmt.Errorf("usage: raw <command string>")
	}
	rawCmd := strings.Join(args, " ")
	raw, err := ctx.Conn.Command(rawCmd)
	if err != nil {
		return err
	}
	fmt.Println(raw)
	return nil
}

// PrintUsage prints available commands.
func PrintUsage() {
	fmt.Fprintln(os.Stderr, "GroundTruth — microcontroller debugging multitool")
	fmt.Fprintln(os.Stderr, "")
	fmt.Fprintln(os.Stderr, "Usage: gt [--port PORT] <command> [args...]")
	fmt.Fprintln(os.Stderr, "")
	fmt.Fprintln(os.Stderr, "Commands:")
	fmt.Fprintln(os.Stderr, "  ping                           Check probe connection")
	fmt.Fprintln(os.Stderr, "  read <pin>                     Read digital state")
	fmt.Fprintln(os.Stderr, "  voltage <pin>                  Read analog voltage (mV)")
	fmt.Fprintln(os.Stderr, "  set <pin> <HIGH|LOW>           Set output pin")
	fmt.Fprintln(os.Stderr, "  watch <pin> <ms>               Monitor pin for activity")
	fmt.Fprintln(os.Stderr, "  edges <pin> <ms>               Count edge transitions")
	fmt.Fprintln(os.Stderr, "  rails                          Read power rail voltages")
	fmt.Fprintln(os.Stderr, "  pulse <pin> <count> <delay_us> Generate pulse train")
	fmt.Fprintln(os.Stderr, "  button-test <pin> <ms>         Test button presses")
	fmt.Fprintln(os.Stderr, "  encoder-test <pinA> <pinB> <ms> Test rotary encoder")
	fmt.Fprintln(os.Stderr, "  map                            Show pin mapping")
	fmt.Fprintln(os.Stderr, "  raw <command>                  Send raw command")
	fmt.Fprintln(os.Stderr, "")
	fmt.Fprintln(os.Stderr, "Pins: P1 P2 P3 (inputs), O1 O2 (outputs), R3V R5V (rails)")
	fmt.Fprintln(os.Stderr, "")
	fmt.Fprintln(os.Stderr, "Options:")
	fmt.Fprintln(os.Stderr, "  --port, -p  Serial port (default: auto-detect)")
	fmt.Fprintln(os.Stderr, "  --list, -l  List available serial ports")
}
