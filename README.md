# ESP32 Websocket based Real Time Log Viewer
This example redirects the esp32 console in real-time to webpage using websockets. Developed using [esp-idf](https://www.espressif.com/en/products/sdks/esp-idf).

This application dynamically redirects the esp console to the web browser when the user presses the start button and stops the redirection when pressing stop. Redirection is performed using the 
[``esp_log_set_vprintf()``](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html#_CPPv419esp_log_set_vprintf14vprintf_like_t) function. 

The esp-idf log framework highlights the different log levels using colours specified by [ANSI Escape Codes](https://en.wikipedia.org/wiki/ANSI_escape_code). These codes are stripped out using the ``remove_ansi_escape_codes()`` function prior to sending down the websocket.

Javascript running on the client side replaces greater than and less than symbols with the correct HTML codes to provide HTML compliance. When the end user presses the save button, the various elements making up the log file is iterated and saved to file. This provides a real-time interface without any flash wear that could occur if files are written to SPIFFS and retrieved.

When the log panel is full, a scroll bar will appear and the last line shown similar to a ``tail -f``.

<P ALIGN="CENTER"><IMG SRC="https://raw.githubusercontent.com/craigpeacock/ESP32_Websocket_LogViewer/master/img/screenshot.png" width=80% height=80%></P> 
 
