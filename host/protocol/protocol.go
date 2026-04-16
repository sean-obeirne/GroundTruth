// Package protocol handles parsing GroundTruth probe responses.
package protocol

import (
	"fmt"
	"strings"
)

// Response represents a parsed probe response.
type Response struct {
	OK     bool
	Raw    string
	Fields []string // all space-separated fields after OK/ERR
}

// Parse parses a raw response line from the probe.
func Parse(line string) (*Response, error) {
	line = strings.TrimSpace(line)
	if line == "" {
		return nil, fmt.Errorf("empty response")
	}

	parts := strings.Fields(line)
	r := &Response{Raw: line}

	switch parts[0] {
	case "OK":
		r.OK = true
		r.Fields = parts[1:]
	case "ERR":
		r.OK = false
		r.Fields = parts[1:]
	default:
		return nil, fmt.Errorf("unexpected response prefix: %s", parts[0])
	}

	return r, nil
}

// Error returns the error message if the response is an error.
func (r *Response) Error() string {
	if r.OK {
		return ""
	}
	return strings.Join(r.Fields, " ")
}

// Field returns the field at index i (0-based, after OK/ERR), or empty string.
func (r *Response) Field(i int) string {
	if i < 0 || i >= len(r.Fields) {
		return ""
	}
	return r.Fields[i]
}
