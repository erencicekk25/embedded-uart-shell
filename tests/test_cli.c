#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        (void)fprintf(stderr, "Test failed: %s (%s:%d)\n", #condition, \
                      __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

typedef struct {
    char data[1024];
    size_t length;
    size_t calls;
} test_context_t;

static void capture_write(void *context, const char *data, size_t length)
{
    test_context_t *test = context;
    size_t available = sizeof(test->data) - test->length - 1U;
    size_t copied = length < available ? length : available;

    memcpy(&test->data[test->length], data, copied);
    test->length += copied;
    test->data[test->length] = '\0';
}

static void handle_ping(void *context, cli_t *cli, size_t argument_count,
                        const char *const arguments[])
{
    test_context_t *test = context;

    (void)arguments;
    if (argument_count == 1U) {
        ++test->calls;
        cli_write(cli, "pong\r\n");
    }
}

static void send_text(cli_t *cli, const char *text)
{
    while (*text != '\0') {
        CHECK(cli_receive_byte(cli, (uint8_t)*text));
        ++text;
    }
    cli_process(cli);
}

static void test_ring_buffer(void)
{
    ring_buffer_t buffer;
    uint8_t value;
    size_t index;

    ring_buffer_init(&buffer);
    for (index = 0U; index < RING_BUFFER_CAPACITY - 1U; ++index) {
        CHECK(ring_buffer_push(&buffer, (uint8_t)index));
    }
    CHECK(!ring_buffer_push(&buffer, 0xFFU));
    CHECK(ring_buffer_pop(&buffer, &value) && value == 0U);
    CHECK(ring_buffer_push(&buffer, 0xFFU));

    for (index = 1U; index < RING_BUFFER_CAPACITY - 1U; ++index) {
        CHECK(ring_buffer_pop(&buffer, &value));
        CHECK(value == (uint8_t)index);
    }
    CHECK(ring_buffer_pop(&buffer, &value) && value == 0xFFU);
    CHECK(!ring_buffer_pop(&buffer, &value));
}

int main(void)
{
    cli_t cli;
    test_context_t test = { 0 };
    const cli_command_t commands[] = {
        { "ping", "Check the command interface", handle_ping, &test },
    };
    char long_line[CLI_LINE_CAPACITY + 2U];

    test_ring_buffer();
    CHECK(cli_init(&cli, commands, 1U, capture_write, &test));
    send_text(&cli, "ping\r\n");
    CHECK(test.calls == 1U);
    CHECK(strstr(test.data, "pong\r\n") != NULL);

    send_text(&cli, "missing\n");
    CHECK(strstr(test.data, "Error: unknown command.") != NULL);

    memset(long_line, 'x', sizeof(long_line) - 2U);
    long_line[sizeof(long_line) - 2U] = '\n';
    long_line[sizeof(long_line) - 1U] = '\0';
    send_text(&cli, long_line);
    CHECK(strstr(test.data, "Error: input line too long.") != NULL);

    (void)puts("All CLI tests passed.");
    return 0;
}