// Package serial provides communication with the GroundTruth probe via USB serial.
package serial

import (
	"bufio"
	"fmt"
	"strings"
	"time"

	"go.bug.st/serial"
)

// Conn wraps a serial connection to the probe.
type Conn struct {
	port    serial.Port
	scanner *bufio.Scanner
	Name    string
}

// DefaultBaud is the baud rate for the Pro Micro CDC serial.
const DefaultBaud = 115200

// Open connects to the specified serial port.
func Open(portName string) (*Conn, error) {
	mode := &serial.Mode{
		BaudRate: DefaultBaud,
		DataBits: 8,
		StopBits: serial.OneStopBit,
		Parity:   serial.NoParity,
	}
	p, err := serial.Open(portName, mode)
	if err != nil {
		return nil, fmt.Errorf("open %s: %w", portName, err)
	}
	if err := p.SetReadTimeout(3 * time.Second); err != nil {
		p.Close()
		return nil, fmt.Errorf("set timeout: %w", err)
	}
	return &Conn{
		port:    p,
		scanner: bufio.NewScanner(p),
		Name:    portName,
	}, nil
}

// Close the serial connection.
func (c *Conn) Close() error {
	return c.port.Close()
}

// Send writes a command line to the probe (appends \n).
func (c *Conn) Send(cmd string) error {
	_, err := c.port.Write([]byte(cmd + "\n"))
	return err
}

// ReadLine reads one \n-terminated line from the probe.
func (c *Conn) ReadLine() (string, error) {
	if c.scanner.Scan() {
		return strings.TrimRight(c.scanner.Text(), "\r"), nil
	}
	if err := c.scanner.Err(); err != nil {
		return "", err
	}
	return "", fmt.Errorf("connection closed")
}

// Command sends a command and reads the response line.
func (c *Conn) Command(cmd string) (string, error) {
	if err := c.Send(cmd); err != nil {
		return "", fmt.Errorf("send: %w", err)
	}
	return c.ReadLine()
}

// Discover returns a list of serial ports that look like they might be a Pro Micro.
func Discover() ([]string, error) {
	ports, err := serial.GetPortsList()
	if err != nil {
		return nil, err
	}
	var results []string
	for _, p := range ports {
		// On Linux, Pro Micro shows as /dev/ttyACM*
		// On macOS, /dev/cu.usbmodem*
		if strings.Contains(p, "ttyACM") || strings.Contains(p, "usbmodem") {
			results = append(results, p)
		}
	}
	return results, nil
}
