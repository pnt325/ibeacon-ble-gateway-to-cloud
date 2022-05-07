/*
 * mqtt.c
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#include "mqtt.h"


#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t mqtt_client;
static SemaphoreHandle_t publish_block;
static mqtt_event_cb_t mqtt_event;

static bool mqtt_connect = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    // esp_mqtt_client_handle_t client = event->client;

    // int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_connect = true;
        if(mqtt_event)
        {
            mqtt_event(mqtt_connect);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_connect = false;
        if(mqtt_event)
        {
            mqtt_event(mqtt_connect);
        }
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }

        if(mqtt_event)
        {
            mqtt_event(0);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void MQTT_init(mqtt_event_cb_t event)
{
    //! Create semaphore
    publish_block = xSemaphoreCreateMutex();

    //Template: "mqtts://flavio.com:8883"
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://192.168.0.36:1883",
        .username = "uwt32",
        .password = "wt32"
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    mqtt_event = event;
}

bool MQTT_publish(const char* topic, const char* data, uint16_t len)
{
    if(mqtt_connect == false)
    {
        ESP_LOGW(TAG, "Publish reject: mqtt disconnected");
        return false;
    }

    if(topic == NULL || data == NULL || len == 0)
    {
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    xSemaphoreTake(publish_block, portMAX_DELAY);
    ESP_LOGI(TAG, "Publish: %s: %s", topic, data);
    int ret = esp_mqtt_client_publish(mqtt_client, topic, data, len, 0, 0);
    xSemaphoreGive(publish_block);

    return (ret == 0);
}

void MQTT_clear_connnect(void)
{
    mqtt_connect = false;
}
