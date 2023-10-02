#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <sys/param.h>
#include "console.h"
#include "http.h"

size_t remove_ansi_escape_codes(char *input, char *output, size_t len)
{
	char *ptr_out = output;
	char *ptr_in = input;

	while (ptr_in <= (input + len - 1)) {
		if (*ptr_in == '\033') {
			// Search for 'm';
			ptr_in = strchr(ptr_in, 'm') + 1;
		} else {
			*ptr_out++ = *ptr_in++;
		}
	}
	return ((ptr_out -1) - output);
}

int websocket_log_output(const char* szFormat, va_list args)
{
	static char log_print_buffer[512];
	static char buffer[512];

	int ret = vsnprintf (log_print_buffer, sizeof(log_print_buffer), szFormat, args);
	if(ret >= 0) {
		// Also copy console to UART
		printf("%.*s", ret, log_print_buffer);
		// esp-idf use ANSI escape codes to colour text on the console
		ret = remove_ansi_escape_codes(log_print_buffer, buffer, ret);
		ws_async_send(buffer, ret);
	}
	return(0);
}

int uart_log_output(const char* szFormat, va_list args)
{
	static char log_print_buffer[512];
	int ret = vsnprintf (log_print_buffer, sizeof(log_print_buffer), szFormat, args);
	if(ret >= 0) {
		// Also copy console to UART
		printf("%.*s", ret, log_print_buffer);
	}
	return(0);
}