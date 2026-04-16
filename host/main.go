// GroundTruth host CLI — gt
//
// Usage: gt [--port /dev/ttyACM0] <command> [args...]
package main

import (
	"fmt"
	"os"

	"github.com/seanvelasco/groundtruth/cmd"
	gtserial "github.com/seanvelasco/groundtruth/serial"
)

func main() {
	args := os.Args[1:]
	port := ""

	// Parse global flags
	for len(args) > 0 {
		switch args[0] {
		case "--port", "-p":
			if len(args) < 2 {
				fatal("--port requires a value")
			}
			port = args[1]
			args = args[2:]
		case "--list", "-l":
			listPorts()
			return
		case "--help", "-h":
			cmd.PrintUsage()
			return
		default:
			goto done
		}
	}
done:

	if len(args) == 0 {
		cmd.PrintUsage()
		os.Exit(1)
	}

	verb := args[0]
	cmdArgs := args[1:]

	// Look up command handler
	handler, ok := cmd.RunDefs[verb]
	if !ok {
		fmt.Fprintf(os.Stderr, "unknown command: %s\n\n", verb)
		cmd.PrintUsage()
		os.Exit(1)
	}

	// Auto-detect port if not specified
	if port == "" {
		ports, err := gtserial.Discover()
		if err != nil {
			fatal("port discovery: %v", err)
		}
		if len(ports) == 0 {
			fatal("no probe found — use --port to specify")
		}
		if len(ports) > 1 {
			fmt.Fprintf(os.Stderr, "multiple ports found, using %s (use --port to specify):\n", ports[0])
			for _, p := range ports {
				fmt.Fprintf(os.Stderr, "  %s\n", p)
			}
		}
		port = ports[0]
	}

	// Connect
	conn, err := gtserial.Open(port)
	if err != nil {
		fatal("connect: %v", err)
	}
	defer conn.Close()

	ctx := &cmd.Context{Conn: conn}
	if err := handler(ctx, cmdArgs); err != nil {
		fatal("%v", err)
	}
}

func listPorts() {
	ports, err := gtserial.Discover()
	if err != nil {
		fatal("discovery: %v", err)
	}
	if len(ports) == 0 {
		fmt.Println("No serial ports found.")
		return
	}
	fmt.Println("Available ports:")
	for _, p := range ports {
		fmt.Printf("  %s\n", p)
	}
}

func fatal(format string, a ...any) {
	fmt.Fprintf(os.Stderr, "gt: "+format+"\n", a...)
	os.Exit(1)
}
