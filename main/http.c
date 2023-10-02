
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <sys/param.h>
#include <esp_ota_ops.h>
#include <time.h>
#include "esp_flash.h"
#include "esp_log.h"
#include "http.h"
#include "console.h"

static const char *TAG = "http";

static httpd_handle_t http_server = NULL;
static httpd_handle_t hd = NULL;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[] asm("_binary_style_css_end");

esp_err_t index_get_handler(httpd_req_t *req)
{
	httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
	return ESP_OK;
}

esp_err_t css_get_handler(httpd_req_t *req)
{
	httpd_resp_send(req, (const char *) style_css_start, style_css_end - style_css_start);
	return ESP_OK;
}

// https://esp32tutorials.com/esp32-esp-idf-websocket-web-server/

static esp_err_t websocket_handler(httpd_req_t *req)
{
	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;

	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

	if (req->method == HTTP_GET)
	{
		ESP_LOGI(TAG, "Handshake done, the new connection was opened");
		hd = req->handle;
		return ESP_OK;
	}

	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	// Set max_len = 0 to get the frame len
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);

	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
		return ret;
	}

	if (ws_pkt.len)
	{
		buf = calloc(1, ws_pkt.len + 1);
		if (buf == NULL)
		{
			ESP_LOGE(TAG, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
	
		ws_pkt.payload = buf;

		// Set max_len = ws_pkt.len to get the frame payload
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK)
		{
			ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
		
		if (strncmp((char *)ws_pkt.payload, "Start", 5) == 0) {
				// Redirect log output to the custom function
				esp_log_set_vprintf(websocket_log_output);
				ESP_LOGI(TAG, "Redirecting console to websocket");
		}

		if (strncmp((char *)ws_pkt.payload, "Stop", 4) == 0) {
				// Redirect log output to the custom function
				ESP_LOGI(TAG, "Stopped redirecting console to websocket");
				esp_log_set_vprintf(uart_log_output);
		}

		free(buf);
	}

	return ESP_OK;
}

void ws_async_send(char *buf, size_t len)
{
	if (hd == NULL)
		return;

	httpd_ws_frame_t ws_pkt;

	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	
	ws_pkt.payload = (uint8_t *)buf;
	ws_pkt.len = len;
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	// Obtain file descriptors from httpd server
	static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
	size_t fds = max_clients;
	int client_fds[max_clients];

	esp_err_t ret = httpd_get_client_list(http_server, &fds, client_fds);

	if (ret != ESP_OK) {
		return;
	}

	for (int i = 0; i < fds; i++) {
		int client_info = httpd_ws_get_fd_info(http_server, client_fds[i]);
		if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
			httpd_ws_send_frame_async(hd, client_fds[i], &ws_pkt);
		}
	}
}

httpd_uri_t index_get = {
	.uri		= "/",
	.method		= HTTP_GET,
	.handler	= index_get_handler,
	.user_ctx	= NULL
};

httpd_uri_t ccs_get = {
	.uri		= "/style.css",
	.method		= HTTP_GET,
	.handler	= css_get_handler,
	.user_ctx	= NULL
};

httpd_uri_t ws = {
	.uri		= "/ws",
	.method		= HTTP_GET,
	.handler	= websocket_handler,
	.user_ctx	= NULL,
	.is_websocket = true
};

esp_err_t http_server_init(void)
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 10;

	ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);

	if (httpd_start(&http_server, &config) == ESP_OK) {
		httpd_register_uri_handler(http_server, &index_get);
		httpd_register_uri_handler(http_server, &ccs_get);
		httpd_register_uri_handler(http_server, &ws);
	}

	return http_server == NULL ? ESP_FAIL : ESP_OK;
}

esp_err_t http_server_stop(void)
{
	int ret;

	ESP_LOGI(TAG, "Stopping HTTP server");

	if (http_server) {
		ret = httpd_stop(http_server);
	} else {
		ret = ESP_ERR_INVALID_ARG;
	}
	return ret;
}