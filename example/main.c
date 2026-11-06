#include "cli.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int threshold;
    bool reboot_requested;
} app_context_t;

static void terminal_write(void *context, const char *data, size_t length)
{
    (void)context;
    (void)fwrite(data, 1U, length, stdout);
    (void)fflush(stdout);
}

static void handle_set_param(void *context, cli_t *cli, size_t argument_count,
                             const char *const arguments[])
{
    app_context_t *app = context;
    char *end = NULL;
    long value;

    if (argument_count != 3U || strcmp(arguments[1], "threshold") != 0) {
        cli_write(cli, "Usage: set_param threshold <0-1000>\r\n");
        return;
    }

    value = strtol(arguments[2], &end, 10);
    if (end == arguments[2] || *end != '\0' || value < 0L || value > 1000L) {
        cli_write(cli, "Error: threshold must be between 0 and 1000.\r\n");
        return;
    }

    app->threshold = (int)value;
    cli_write(cli, "OK: parameter updated.\r\n");
}

static void handle_get_status(void *context, cli_t *cli, size_t argument_count,
                              const char *const arguments[])
{
    app_context_t *app = context;
    char status[64];
    int length;

    (void)arguments;
    if (argument_count != 1U) {
        cli_write(cli, "Usage: get_status\r\n");
        return;
    }

    length = snprintf(status, sizeof(status), "threshold=%d, reboot=%s\r\n",
                      app->threshold,
                      app->reboot_requested ? "requested" : "idle");
    if (length > 0) {
        cli_write(cli, status);
    }
}

static void handle_reboot(void *context, cli_t *cli, size_t argument_count,
                          const char *const arguments[])
{
    app_context_t *app = context;

    (void)arguments;
    if (argument_count != 1U) {
        cli_write(cli, "Usage: reboot\r\n");
        return;
    }

    app->reboot_requested = true;
    cli_write(cli, "Reboot requested (simulation).\r\n");
}

int main(void)
{
    cli_t cli;
    app_context_t app = { .threshold = 100, .reboot_requested = false };
    const cli_command_t commands[] = {
        { "set_param", "Set a device parameter", handle_set_param, &app },
        { "get_status", "Show device status", handle_get_status, &app },
        { "reboot", "Request a device reboot", handle_reboot, &app },
    };
    int input;

    if (!cli_init(&cli, commands, sizeof(commands) / sizeof(commands[0]),
                  terminal_write, NULL)) {
        return EXIT_FAILURE;
    }

    (void)fputs("> ", stdout);
    (void)fflush(stdout);
    while ((input = getchar()) != EOF) {
        if (cli_receive_byte(&cli, (uint8_t)input)) {
            cli_process(&cli);
        } else {
            cli_write(&cli, "Error: receive buffer full.\r\n");
        }
        if (input == '\r' || input == '\n') {
            (void)fputs("> ", stdout);
            (void)fflush(stdout);
        }
    }

    return EXIT_SUCCESS;
}