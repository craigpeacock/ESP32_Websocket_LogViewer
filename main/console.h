
size_t remove_ansi_escape_codes(char *input, char *output, size_t len);
int websocket_log_output(const char* szFormat, va_list args);
int uart_log_output(const char* szFormat, va_list args);