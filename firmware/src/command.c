/*
 * GroundTruth - Serial command parser and dispatcher
 *
 * Protocol: line-oriented, \n terminated
 * Request:  COMMAND [ARG1 [ARG2 ...]]
 * Response: OK ... | ERR ...
 */
#include "command.h"
#include "pins.h"
#include "adc.h"
#include "timing.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* USB CDC serial provided by LUFA-style or Arduino CDC */
extern void usb_serial_putchar(char c);
extern int16_t usb_serial_getchar(void);
extern uint8_t usb_serial_available(void);

static char cmd_buf[CMD_BUF_SIZE];
static uint8_t cmd_pos = 0;

/* Output buffer for snprintf responses */
static char resp_buf[80];

/* --- internal helpers --- */

static void send_str(const char *s) {
    while (*s)
        usb_serial_putchar(*s++);
}

void cmd_respond(const char *msg) {
    send_str(msg);
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
}

/* Uppercase a string in place */
static void str_upper(char *s) {
    for (; *s; s++)
        *s = toupper((unsigned char)*s);
}

/* --- command handlers --- */

static void cmd_ping(void) {
    cmd_respond("OK PONG");
}

static void cmd_map(void) {
    cmd_respond("OK MAP P1=A0 P2=A1 P3=A2 O1=D6 O2=D5 R3V=A3 R5V=A4");
}

static void cmd_read(char *args[], uint8_t argc) {
    if (argc < 1) { cmd_respond("ERR READ requires pin"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_INPUT)) { cmd_respond("ERR pin not input"); return; }

    pin_set_input(id);
    uint8_t val = pin_digital_read(id);
    snprintf(resp_buf, sizeof(resp_buf), "OK READ %s %s",
             pin_name(id), val ? "HIGH" : "LOW");
    cmd_respond(resp_buf);
}

static void cmd_voltage(char *args[], uint8_t argc) {
    if (argc < 1) { cmd_respond("ERR VOLTAGE requires pin"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_ANALOG)) { cmd_respond("ERR pin not analog"); return; }

    uint16_t mv = adc_read_mv(id);
    snprintf(resp_buf, sizeof(resp_buf), "OK VOLTAGE %s %u",
             pin_name(id), mv);
    cmd_respond(resp_buf);
}

static void cmd_set(char *args[], uint8_t argc) {
    if (argc < 2) { cmd_respond("ERR SET requires pin and HIGH/LOW"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_OUTPUT)) { cmd_respond("ERR pin not output"); return; }

    uint8_t val;
    if (strcasecmp(args[1], "HIGH") == 0)      val = 1;
    else if (strcasecmp(args[1], "LOW") == 0)   val = 0;
    else { cmd_respond("ERR expected HIGH or LOW"); return; }

    pin_set_output(id);
    pin_digital_write(id, val);
    snprintf(resp_buf, sizeof(resp_buf), "OK SET %s %s",
             pin_name(id), val ? "HIGH" : "LOW");
    cmd_respond(resp_buf);
}

static void cmd_watch(char *args[], uint8_t argc) {
    if (argc < 2) { cmd_respond("ERR WATCH requires pin and duration_ms"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_INPUT)) { cmd_respond("ERR pin not input"); return; }

    uint16_t ms = atoi(args[1]);
    if (ms == 0 || ms > 30000) { cmd_respond("ERR duration 1-30000 ms"); return; }

    uint16_t edges = watch_pin(id, ms);
    snprintf(resp_buf, sizeof(resp_buf), "OK WATCH %s %s %u",
             pin_name(id), edges ? "ACTIVE" : "IDLE", edges);
    cmd_respond(resp_buf);
}

static void cmd_edges(char *args[], uint8_t argc) {
    if (argc < 2) { cmd_respond("ERR EDGES requires pin and duration_ms"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_INPUT)) { cmd_respond("ERR pin not input"); return; }

    uint16_t ms = atoi(args[1]);
    if (ms == 0 || ms > 30000) { cmd_respond("ERR duration 1-30000 ms"); return; }

    uint16_t count = edges_count(id, ms);
    snprintf(resp_buf, sizeof(resp_buf), "OK EDGES %s %u",
             pin_name(id), count);
    cmd_respond(resp_buf);
}

static void cmd_rails(void) {
    uint16_t mv3 = adc_read_mv(PIN_R3V);
    uint16_t mv5 = adc_read_mv(PIN_R5V);
    snprintf(resp_buf, sizeof(resp_buf), "OK RAILS 3V3=%u 5V=%u",
             mv3, mv5);
    cmd_respond(resp_buf);
}

static void cmd_pulse(char *args[], uint8_t argc) {
    if (argc < 3) { cmd_respond("ERR PULSE requires pin count delay_us"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_OUTPUT)) { cmd_respond("ERR pin not output"); return; }

    uint16_t count = atoi(args[1]);
    uint16_t delay = atoi(args[2]);
    if (count == 0 || count > 10000) { cmd_respond("ERR count 1-10000"); return; }
    if (delay == 0 || delay > 50000) { cmd_respond("ERR delay_us 1-50000"); return; }

    pulse_train(id, count, delay);
    snprintf(resp_buf, sizeof(resp_buf), "OK PULSE %s %u", pin_name(id), count);
    cmd_respond(resp_buf);
}

static void cmd_button_test(char *args[], uint8_t argc) {
    if (argc < 2) { cmd_respond("ERR BUTTON_TEST requires pin and duration_ms"); return; }
    pin_id_t id = pin_parse(args[0]);
    if (id == PIN_INVALID) { cmd_respond("ERR unknown pin"); return; }
    if (!pin_has_cap(id, PIN_CAP_INPUT)) { cmd_respond("ERR pin not input"); return; }

    uint16_t ms = atoi(args[1]);
    if (ms == 0 || ms > 60000) { cmd_respond("ERR duration 1-60000 ms"); return; }

    uint16_t presses = button_test(id, ms);
    snprintf(resp_buf, sizeof(resp_buf), "OK BUTTON_TEST %s %u",
             pin_name(id), presses);
    cmd_respond(resp_buf);
}

static void cmd_encoder_test(char *args[], uint8_t argc) {
    if (argc < 3) { cmd_respond("ERR ENCODER_TEST requires pinA pinB duration_ms"); return; }
    pin_id_t pin_a = pin_parse(args[0]);
    pin_id_t pin_b = pin_parse(args[1]);
    if (pin_a == PIN_INVALID || pin_b == PIN_INVALID) {
        cmd_respond("ERR unknown pin"); return;
    }
    if (!pin_has_cap(pin_a, PIN_CAP_INPUT) || !pin_has_cap(pin_b, PIN_CAP_INPUT)) {
        cmd_respond("ERR pins must be input"); return;
    }

    uint16_t ms = atoi(args[2]);
    if (ms == 0 || ms > 60000) { cmd_respond("ERR duration 1-60000 ms"); return; }

    int16_t delta = encoder_test(pin_a, pin_b, ms);
    snprintf(resp_buf, sizeof(resp_buf), "OK ENCODER_TEST %d", delta);
    cmd_respond(resp_buf);
}

/* --- dispatcher --- */

static void dispatch(char *line) {
    char *tokens[CMD_MAX_ARGS + 1];
    uint8_t tc = 0;

    /* Tokenize by spaces */
    char *tok = strtok(line, " \t");
    while (tok && tc < CMD_MAX_ARGS + 1) {
        tokens[tc++] = tok;
        tok = strtok(NULL, " \t");
    }
    if (tc == 0) return;

    /* Uppercase the command verb */
    str_upper(tokens[0]);

    char **args = &tokens[1];
    uint8_t argc = tc - 1;

    /* Uppercase pin arguments too */
    for (uint8_t i = 0; i < argc; i++)
        str_upper(args[i]);

    /* Dispatch */
    if      (strcmp(tokens[0], "PING") == 0)          cmd_ping();
    else if (strcmp(tokens[0], "MAP") == 0)            cmd_map();
    else if (strcmp(tokens[0], "READ") == 0)           cmd_read(args, argc);
    else if (strcmp(tokens[0], "VOLTAGE") == 0)        cmd_voltage(args, argc);
    else if (strcmp(tokens[0], "SET") == 0)             cmd_set(args, argc);
    else if (strcmp(tokens[0], "WATCH") == 0)          cmd_watch(args, argc);
    else if (strcmp(tokens[0], "EDGES") == 0)          cmd_edges(args, argc);
    else if (strcmp(tokens[0], "RAILS") == 0)          cmd_rails();
    else if (strcmp(tokens[0], "PULSE") == 0)          cmd_pulse(args, argc);
    else if (strcmp(tokens[0], "BUTTON_TEST") == 0)    cmd_button_test(args, argc);
    else if (strcmp(tokens[0], "ENCODER_TEST") == 0)   cmd_encoder_test(args, argc);
    else {
        snprintf(resp_buf, sizeof(resp_buf), "ERR unknown command: %s", tokens[0]);
        cmd_respond(resp_buf);
    }
}

/* --- public API --- */

void command_init(void) {
    cmd_pos = 0;
}

void command_feed(char c) {
    if (c == '\r') return;  /* ignore CR */
    if (c == '\n') {
        if (cmd_pos > 0) {
            cmd_buf[cmd_pos] = '\0';
            dispatch(cmd_buf);
            cmd_pos = 0;
        }
        return;
    }
    if (cmd_pos < CMD_BUF_SIZE - 1) {
        cmd_buf[cmd_pos++] = c;
    }
    /* silently drop overflow characters */
}
