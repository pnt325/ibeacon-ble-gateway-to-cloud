/*
 * wifi_config.c
 *
 *  Created on: May 09, 2022
 *      Author: Phat.N
 */

#include "wifi_config.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "cJSON.h"
#include "nvs_data.h"

static const char *TAG = "HTTP";

static void process_wifi_config(char* data);

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[128] = {0};
    int ret, remaining = req->content_len;
    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        process_wifi_config(buf);

        /* Log data received */
        // ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        // ESP_LOGI(TAG, "%.*s", ret, buf);
        // ESP_LOGI(TAG, "====================================");

        // /* Send back the same data */
        // memset(buf, 0x00, sizeof(buf));
        // snprintf(buf, sizeof(buf), "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", "Success", "Save data to nvs",
        //          "Client ID", "0a:0b:0c:0d:0e:0f",
        //          "Hostname", "192.168.4.1");

        httpd_resp_send_chunk(req, buf, strlen(buf));
        remaining -= ret;
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t http_uri_post = {
    .uri = "/api/set_wifi",
    .method = HTTP_POST,
    .handler = echo_post_handler,
    .user_ctx = NULL};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &http_uri_post);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void WIFI_CONFIG_init(void)
{
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    server = start_webserver();
}

void WIFI_CONFIG_start(void)
{
}

static void process_wifi_config(char *data)
{
    // {
    //     "ssid":"BGATEWAY",
    //     "password":"password2432"
    // }
    char* ssid = NULL;
    char* password = NULL;

    cJSON *root = cJSON_Parse((const char*)data);
    if (cJSON_GetObjectItem(root, "ssid"))
    {
        ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
        ESP_LOGI(TAG, "ssid=%s", ssid);
    }
    if (cJSON_GetObjectItem(root, "password"))
    {
        password = cJSON_GetObjectItem(root, "password")->valuestring;
        ESP_LOGI(TAG, "password=%s", password);
    }
    cJSON_Delete(root);

    if(ssid && password)
    {
        NVS_DATA_ssid_set(ssid);
        NVS_DATA_password_set(password);
        NVS_DATA_init_config_set(1);
    }
}
