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
#include "bg22_config.h"

static const char *TAG = "HTTP";

#define URI_SET_WIFI "/api/set_wifi"
#define URI_WHOAMI   "/api/whoami"
#define URI_RESET    "/api/reset"

extern uint8_t mac[8];

static bool process_wifi_config(char* data);
static bool reset_handle(char* data);

/* An HTTP GET handler */
static esp_err_t whoami_get_handler(httpd_req_t *req)
{
    char *buf = (char*)malloc(128);
    bg22_assert(buf);
    memset(buf, 0, 128);

    /*
        {
        "clientId": "ec:94:cb:65:51:70",
        "Success": true,
        "firmwareVersion": "VALUE-firmwareVersion",
        "modelNumber": "some-static-value"
        }
    */
   snprintf(buf, 128, "{\"clientId\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"Success\": true,\"firmwareVersion\":\"%d\",\"modelNumber\": \"%s\"}",           
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],     
                        123,
                        "ABC-123");
    ESP_LOGI(TAG, "GET response: %s\n", buf);
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);

    free(buf);
    buf = NULL;

    return ESP_OK;
}

static const httpd_uri_t uri_whoami = {
    .uri       = URI_WHOAMI,
    .method    = HTTP_GET,
    .handler   = whoami_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

/* An HTTP POST handler */
static esp_err_t set_wifi_post_handler(httpd_req_t *req)
{
    char buf[256] = {0};
    int ret, remaining = req->content_len;
    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        char ret_char[10] = {0};
        if(process_wifi_config(buf)) {
            snprintf(ret_char, sizeof(ret_char), "true");
        } else  {
            snprintf(ret_char, sizeof(ret_char), "false");
        }

        memset(buf, 0, 128);

        snprintf(buf, 128, "{\"clientId\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"Success\": %s,\"firmwareVersion\":\"%d\",\"modelNumber\": \"%s\", \"ipaddress\": \"%s\",}",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                 ret_char,
                 123,
                 "ABC-123",
                 "UnKnow");

        httpd_resp_send_chunk(req, buf, strlen(buf));
        remaining -= ret;
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t uri_set_wifi = {
    .uri = URI_SET_WIFI,
    .method = HTTP_POST,
    .handler = set_wifi_post_handler,
    .user_ctx = NULL};

static esp_err_t reset_post_handler(httpd_req_t *req)
{
    char buf[256] = {0};
    int ret, remaining = req->content_len;
    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        char ret_char[10] = {0};
        bool reset = reset_handle(buf);
        if(reset) {
            snprintf(ret_char, sizeof(ret_char), "true");
        } else  {
            snprintf(ret_char, sizeof(ret_char), "false");
        }

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "{\"Success\": %s}", ret_char);
        httpd_resp_send_chunk(req, buf, strlen(buf));
        remaining -= ret;

        uint8_t is_config = 0;
        NVS_DATA_init_config_get(&is_config);
        if(reset && is_config) {
            esp_restart();
        } else {
            ESP_LOGE(TAG, "Reset failure\n");
        }
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t uri_reset = {
    .uri = URI_RESET,
    .method = HTTP_POST,
    .handler = reset_post_handler,
    .user_ctx = NULL};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp(URI_WHOAMI, req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/api/whoami URI is not available");
        return ESP_OK;
    } else if (strcmp(URI_SET_WIFI, req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/api/set_wifi URI is not available");
        return ESP_FAIL;
    }

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
        httpd_register_uri_handler(server, &uri_set_wifi);
        httpd_register_uri_handler(server, &uri_whoami);
        httpd_register_uri_handler(server, &uri_reset);
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

static bool process_wifi_config(char *data)
{
/*
    {
    "ssid": "ssid vale",
    "password": "password",
    "mqttAddress": "hostname",
    "mqttUsername": "username",
    "mqttPassword": "password"
    }
*/

    char* ssid = NULL;
    char* password = NULL;
    char* mqtt_adr = NULL;
    char* mqtt_usr = NULL;
    char* mqtt_pass = NULL;

    ESP_LOGI(TAG, "%s\n", data);

    cJSON *root = cJSON_Parse((const char*)data);
    if(root == NULL) {
        ESP_LOGE(TAG, "Json formst error\n", data);
        return false;
    }

    if (cJSON_GetObjectItem(root, "ssid"))
    {
        ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
        ESP_LOGI(TAG, "ssid=%s\n", ssid);
    }
    if (cJSON_GetObjectItem(root, "password"))
    {
        password = cJSON_GetObjectItem(root, "password")->valuestring;
        ESP_LOGI(TAG, "password=%s\n", password);
    }
    if (cJSON_GetObjectItem(root, "mqttAddress"))
    {
        mqtt_adr = cJSON_GetObjectItem(root, "mqttAddress")->valuestring;
        ESP_LOGI(TAG, "mqttAddress=%s\n", password);
    }
    if (cJSON_GetObjectItem(root, "mqttUsername"))
    {
        mqtt_usr = cJSON_GetObjectItem(root, "mqttUsername")->valuestring;
        ESP_LOGI(TAG, "mqttUsername=%s\n", password);
    }
    if (cJSON_GetObjectItem(root, "mqttPassword"))
    {
        mqtt_pass = cJSON_GetObjectItem(root, "mqttPassword")->valuestring;
        ESP_LOGI(TAG, "mqttPassword=%s\n", password);
    }
    cJSON_Delete(root);

/*
    {
    "clientId": "ec:94:cb:65:51:70",
    "Success": true,
    "firmwareVersion": "VALUE-forfirmware-version",
    "ipaddress": "IP address",
    "modelNumber": "some-static-value"
    }
*/
    
    if(ssid && password && mqtt_adr && mqtt_usr && mqtt_pass)
    {
        // WIFI password len should equal or larger 8
        if(strlen(password) < 8) {
            return false;
        }

        // Save to NVS
        NVS_DATA_ssid_set(ssid);
        NVS_DATA_password_set(password);
        NVS_DATA_mqtt_addr_set(mqtt_adr);
        NVS_DATA_mqtt_user_set(mqtt_usr);
        NVS_DATA_mqtt_pass_set(mqtt_pass);
        NVS_DATA_init_config_set(1);
        return true;
    }
    return false;
}

static bool reset_handle(char *data) {
    cJSON *root = cJSON_Parse((const char*)data);
    if(root == NULL) {
        ESP_LOGE(TAG, "Json formst error\n", data);
        return false;
    }

    bool ret = false;
    cJSON *success = cJSON_GetObjectItem(root, "Success");
    if(success) {
        if (cJSON_IsTrue(success)) {
            return true;
        }
    }
    cJSON_Delete(root);

    return ret;
}
