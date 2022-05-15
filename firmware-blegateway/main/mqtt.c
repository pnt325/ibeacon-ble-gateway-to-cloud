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
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>

#include "bg22_config.h"

static const char *TAG = "MQTT";

#define MQTT_EVENT_PUB_BIT  (1 << 0)    // Bit 0
#define MQTT_EVENT_SUB_BIT  (1 << 1)    // Bit 1
#define MQTT_EVENT_USUB_BIT (1 << 2)    // bit 2

/* Subscribe structure */
typedef struct mqtt_sub_handler{
    char*                topic;
    mqtt_data_callback_t callback;
    struct mqtt_sub_handler *next;
}mqtt_sub_handler_t;

static esp_mqtt_client_handle_t mqtt_client;        // mqtt client
static SemaphoreHandle_t        publish_block;      // publish block
static SemaphoreHandle_t        topic_block;        // topic block
static SemaphoreHandle_t        sub_usub_block;     // subscribe/un-subscribe block
static mqtt_event_t             mqtt_event;         // mqtt connect/disconnect callback
static EventGroupHandle_t       event_group;

static bool mqtt_connect = false;
static mqtt_sub_handler_t *sub_handler;

static void mqtt_event_callback(void) {
    if(mqtt_connect) {
        // Block topic
        xSemaphoreTake(topic_block, portMAX_DELAY);
        mqtt_sub_handler_t* handler = sub_handler;

        ESP_LOGI(TAG, "Re-subscribe topic\n");
        while(handler) {
            int msg_id = esp_mqtt_client_subscribe(mqtt_client, handler->topic, 0);
            bg22_assert(msg_id >= 0);
            ESP_LOGI(TAG, "\t- %s\n", handler->topic);
            handler = handler->next;
        }   

        // Release
        xSemaphoreGive(topic_block);
        mqtt_event.connected();
    } else {
        mqtt_event.disconnected();
    }
}

static void mqtt_sub_add_topic(const char* topic, mqtt_data_callback_t callback) {
    bg22_assert(topic);
    bg22_assert(callback);

    xSemaphoreTake(topic_block, portMAX_DELAY);

    mqtt_sub_handler_t* handler = sub_handler;
    while(handler) {
        handler = handler->next;
    }

    // add topic
    handler = (mqtt_sub_handler_t *)malloc(sizeof(mqtt_sub_handler_t));
    bg22_assert(handler);

    size_t len = strlen(topic);
    handler->topic = (char *)malloc(len + 1);
    bg22_assert(handler->topic);
    memset(handler->topic, 0, len + 1);
    memcpy(handler->topic, topic, len);
    handler->callback = callback;
    handler->next = NULL;

    xSemaphoreGive(topic_block);
}

static void mqtt_sub_remove_topic(const char* topic) {
    bg22_assert(topic);

    xSemaphoreTake(topic_block, portMAX_DELAY);

    mqtt_sub_handler_t* handler = sub_handler;
    while(handler) {
        if(memcmp(topic, handler->topic, strlen(topic)) == 0)  {
            free(handler->topic);
            free(handler);
            handler = handler->next;
            break;
        }
        handler = handler->next;
    }

    xSemaphoreGive(topic_block);
}

static mqtt_data_callback_t mqtt_get_callback(char* topic, size_t topic_len) {
    mqtt_data_callback_t callback = NULL;

    // Block sub_handler data
    // xSemaphoreTake(topic_block, portMAX_DELAY);

    mqtt_sub_handler_t* handler = sub_handler;
    while(handler) {
        if(memcmp(topic, handler->topic, topic_len) == 0) {
            callback = handler->callback;
            break;
        }
        handler = handler->next;
    }

    // Release sub_handler data
    // xSemaphoreGive(topic_block);

    return callback;
}

static void mqtt_data_handler(size_t topic_len, char* topic, char* data, size_t data_len) {
    xSemaphoreTake(topic_block, portMAX_DELAY);

    mqtt_data_callback_t callback = mqtt_get_callback(topic, topic_len);
    if(callback) {
        callback(data, data_len, topic, topic_len);
    }

    xSemaphoreGive(topic_block);
}

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
        mqtt_event_callback();
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_connect = false;
        mqtt_event_callback();
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        xEventGroupSetBits(event_group, MQTT_EVENT_SUB_BIT);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        xEventGroupSetBits(event_group, MQTT_EVENT_USUB_BIT);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        xEventGroupSetBits(event_group, MQTT_EVENT_PUB_BIT);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);
        mqtt_data_handler(event->topic_len, event->topic, event->data, event->data_len);
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
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void MQTT_init(mqtt_event_t* event, const char* host, const char* user, const char* password)
{
    //! Create semaphore
    publish_block = xSemaphoreCreateMutex();
    topic_block = xSemaphoreCreateMutex();
    event_group = xEventGroupCreate();
    sub_usub_block = xSemaphoreCreateMutex();

    bg22_assert(publish_block);
    bg22_assert(topic_block);
    bg22_assert(sub_usub_block);

    // MQTT configure
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = host,
        .username = user,
        .password = password
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    mqtt_event = *event;
}

bool MQTT_publish(const char* topic, const char* data, uint16_t len)
{
    bool retval = false;
    if(mqtt_connect == false)
    {
        // ESP_LOGW(TAG, "Publish reject: mqtt disconnected");
        return false;
    }

    if(topic == NULL || data == NULL || len == 0)
    {
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    // Block MQTT public
    xSemaphoreTake(publish_block, portMAX_DELAY);
    ESP_LOGI(TAG, "Publish: %s: %s", topic, data);

    if(esp_mqtt_client_publish(mqtt_client, topic, data, len, 0, 0) < 0) {
        goto exit;
    }

    // wait for message publish finish, timeout 500ms
    EventBits_t bit = xEventGroupWaitBits(event_group, MQTT_EVENT_PUB_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(500));
    if(bit & MQTT_EVENT_PUB_BIT) {
        retval = true;
        goto exit;
    }

    exit:
    xSemaphoreGive(publish_block);
    return retval;
}

bool MQTT_subscribe(const char* topic, mqtt_data_callback_t callback) {
    bool retval = false;

    bg22_assert(topic);
    bg22_assert(callback);

    // Block subscribte
    xSemaphoreTake(sub_usub_block, portMAX_DELAY);
    if(esp_mqtt_client_subscribe(mqtt_client, topic, 0) < 0) {
        goto exit;
    }

    // Wait for subscribe finish
    EventBits_t bit = xEventGroupWaitBits(event_group, MQTT_EVENT_SUB_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(500));
    if(bit & MQTT_EVENT_SUB_BIT) {
        retval = true;
        mqtt_sub_add_topic(topic, callback);
        goto exit;
    }

    exit:
    // Un-Block subscribe
    xSemaphoreGive(sub_usub_block);
    return retval;
}

bool MQTT_unsubscribe(const char* topic) {
    bool retval = false;
    bg22_assert(topic);

    xSemaphoreTake(sub_usub_block, portMAX_DELAY);

    if(esp_mqtt_client_unsubscribe(mqtt_client, topic) < 0) {
        goto exit;
    }

    EventBits_t bit = xEventGroupWaitBits(event_group, MQTT_EVENT_USUB_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(500));
    if(bit & MQTT_EVENT_USUB_BIT) {
        retval = true;
        mqtt_sub_remove_topic(topic);
        goto exit;
    }

    exit:
    xSemaphoreGive(sub_usub_block);
    return retval;
}

void MQTT_clear_connnect(void)
{
    mqtt_connect = false;
}
