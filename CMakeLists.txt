cmake_minimum_required(VERSION 3.16)
project(EmbeddedUartShell VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(embedded_uart_shell
    src/cli.c
    src/ring_buffer.c
)
target_include_directories(embedded_uart_shell PUBLIC include)

add_executable(shell_demo example/main.c)
target_link_libraries(shell_demo PRIVATE embedded_uart_shell)

include(CTest)
if(BUILD_TESTING)
    add_executable(cli_tests tests/test_cli.c)
    target_link_libraries(cli_tests PRIVATE embedded_uart_shell)
    add_test(NAME cli_tests COMMAND cli_tests)
endif()