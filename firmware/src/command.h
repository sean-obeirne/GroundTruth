/*
 * GroundTruth - Serial command parser and dispatcher
 */
#ifndef GT_COMMAND_H
#define GT_COMMAND_H

#include <stdint.h>

/* Maximum command line length */
#define CMD_BUF_SIZE 64

/* Maximum number of tokens in a command */
#define CMD_MAX_ARGS 5

/* Initialize the command processor */
void command_init(void);

/* Feed a character from UART. When a full line is received,
 * it is parsed and executed, and the response is sent. */
void command_feed(char c);

/* Send a response string over USB serial (CRLF terminated) */
void cmd_respond(const char *msg);

#endif /* GT_COMMAND_H */
