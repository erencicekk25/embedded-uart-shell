#include "cli.h"

#include <string.h>

static void write_bytes(cli_t *cli, const char *text, size_t length)
{
    if (cli->write != NULL && length > 0U) {
        cli->write(cli->write_context, text, length);
    }
}

void cli_write(cli_t *cli, const char *text)
{
    if (cli == NULL || text == NULL) {
        return;
    }

    write_bytes(cli, text, strlen(text));
}

static bool is_separator(char value)
{
    return value == ' ' || value == '\t';
}

static void write_help(cli_t *cli)
{
    size_t index;

    cli_write(cli, "Available commands:\r\n  help         Show this list\r\n");
    for (index = 0U; index < cli->command_count; ++index) {
        cli_write(cli, "  ");
        cli_write(cli, cli->commands[index].name);
        cli_write(cli, " - ");
        cli_write(cli, cli->commands[index].description);
        cli_write(cli, "\r\n");
    }
}

static void execute_line(cli_t *cli)
{
    const char *arguments[CLI_MAX_ARGUMENT_COUNT];
    size_t argument_count = 0U;
    size_t index;

    cli->line[cli->line_length] = '\0';
    index = 0U;

    while (cli->line[index] != '\0') {
        while (is_separator(cli->line[index])) {
            cli->line[index++] = '\0';
        }
        if (cli->line[index] == '\0') {
            break;
        }
        if (argument_count == CLI_MAX_ARGUMENT_COUNT) {
            cli_write(cli, "Error: too many arguments.\r\n");
            return;
        }
        arguments[argument_count++] = &cli->line[index];
        while (cli->line[index] != '\0' && !is_separator(cli->line[index])) {
            ++index;
        }
    }

    if (argument_count == 0U) {
        return;
    }
    if (strcmp(arguments[0], "help") == 0) {
        write_help(cli);
        return;
    }

    for (index = 0U; index < cli->command_count; ++index) {
        if (strcmp(arguments[0], cli->commands[index].name) == 0) {
            cli->commands[index].handler(cli->commands[index].context, cli,
                                         argument_count, arguments);
            return;
        }
    }

    cli_write(cli, "Error: unknown command.\r\n");
}

static void finish_line(cli_t *cli)
{
    if (cli->discard_line) {
        cli_write(cli, "Error: input line too long.\r\n");
    } else {
        execute_line(cli);
    }
    cli->line_length = 0U;
    cli->discard_line = false;
}

bool cli_init(cli_t *cli, const cli_command_t *commands, size_t command_count,
              cli_write_fn write, void *write_context)
{
    size_t index;

    if (cli == NULL || write == NULL || command_count > CLI_MAX_COMMAND_COUNT ||
        (command_count > 0U && commands == NULL)) {
        return false;
    }

    for (index = 0U; index < command_count; ++index) {
        if (commands[index].name == NULL || commands[index].description == NULL ||
            commands[index].handler == NULL ||
            strcmp(commands[index].name, "help") == 0) {
            return false;
        }
    }

    ring_buffer_init(&cli->receive_buffer);
    cli->line_length = 0U;
    cli->discard_line = false;
    cli->ignore_next_line_feed = false;
    cli->commands = commands;
    cli->command_count = command_count;
    cli->write = write;
    cli->write_context = write_context;
    return true;
}

bool cli_receive_byte(cli_t *cli, uint8_t byte)
{
    if (cli == NULL) {
        return false;
    }
    return ring_buffer_push(&cli->receive_buffer, byte);
}

void cli_process(cli_t *cli)
{
    uint8_t byte;

    if (cli == NULL) {
        return;
    }

    while (ring_buffer_pop(&cli->receive_buffer, &byte)) {
        if (byte == '\n' && cli->ignore_next_line_feed) {
            cli->ignore_next_line_feed = false;
            continue;
        }

        cli->ignore_next_line_feed = false;
        if (byte == '\r' || byte == '\n') {
            finish_line(cli);
            cli->ignore_next_line_feed = byte == '\r';
        } else if (byte == '\b' || byte == 0x7FU) {
            if (cli->line_length > 0U && !cli->discard_line) {
                --cli->line_length;
            }
        } else if (!cli->discard_line) {
            if (cli->line_length < CLI_LINE_CAPACITY - 1U) {
                cli->line[cli->line_length++] = (char)byte;
            } else {
                cli->discard_line = true;
            }
        }
    }
}