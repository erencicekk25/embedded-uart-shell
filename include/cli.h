#ifndef EMBEDDED_UART_SHELL_CLI_H
#define EMBEDDED_UART_SHELL_CLI_H

#include "ring_buffer.h"

#include <stdbool.h>
#include <stddef.h>

#define CLI_LINE_CAPACITY 64U
#define CLI_MAX_ARGUMENT_COUNT 8U
#define CLI_MAX_COMMAND_COUNT 8U

typedef struct cli cli_t;

typedef void (*cli_write_fn)(void *context, const char *data, size_t length);
typedef void (*cli_command_handler_fn)(void *context, cli_t *cli,
                                       size_t argument_count,
                                       const char *const arguments[]);

typedef struct {
    const char *name;
    const char *description;
    cli_command_handler_fn handler;
    void *context;
} cli_command_t;

struct cli {
    ring_buffer_t receive_buffer;
    char line[CLI_LINE_CAPACITY];
    size_t line_length;
    bool discard_line;
    bool ignore_next_line_feed;
    const cli_command_t *commands;
    size_t command_count;
    cli_write_fn write;
    void *write_context;
};

bool cli_init(cli_t *cli, const cli_command_t *commands, size_t command_count,
              cli_write_fn write, void *write_context);
bool cli_receive_byte(cli_t *cli, uint8_t byte);
void cli_process(cli_t *cli);
void cli_write(cli_t *cli, const char *text);

#endif