/*
 * gateway.c
 *
 *  Created on: May 15, 2022
 *      Author: Phat.N
 */

#include <string.h>
#include "gateway.h"
#include "app_config.h"
#include "bg22_config.h"
#include "mqtt.h"
#include "app_var.h"
#include "uuid.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_log.h"

/* Macro ================================================== */
#define DATA_FIELD_NAME_LEN                 10
#define DATA_FILED_MAX                      12

#define DataTypeFloat_Str                   "float"
#define DataTypeString_Str                  "string"
#define DataTypeInt_Str                     "int"
#define DataTypeUInt_Str                    "uint"
#define DataTypeBool_str                    "bool"

#define GATEWAY_EVENT_BIT_BEACON_DATA       (1 << 0)
#define GATEWAY_EVENT_BIT_AUTHENTICATION    (1 << 1)

#define GATEWAY_TAG                         "Gateway"

/* Type ==================================================== */
typedef enum
{
    FormatFloat,
    FormatString,
    FormatUInt,
    FormatInt,
    FormatBool,
    _FormatNum
} format_t;

typedef struct __attribute__((packed))
{
    char key[DATA_FIELD_NAME_LEN]; // Json key
    uint8_t format;                // Data convert to stirng format
    uint8_t type;                  // Data received form device type
    uint8_t update;                // Device data update
    uint8_t data[4];               // device data
} field_t;

typedef struct __attribute__((packed))
{
    uint8_t uuid[16];               // Device uuid
    uint8_t mac_addr[6];            // Device MAC address
    field_t fields[DATA_FILED_MAX]; // Data field
    uint8_t field_cnt;              // Number of data field
} device_t;

typedef struct __attribute__((packed))
{
    device_t devs[APP_GATEWAY_DEVICE_MAX]; // List of device
    uint8_t dev_cnt;                       // Number of device
} gateway_t;

/* Private variable ======================================== */
static gateway_t gateway;
TaskHandle_t task_gateway;
EventGroupHandle_t gateway_event;
SemaphoreHandle_t gateway_data_lock;

/* Publish variable ======================================== */
extern uint8_t mac[8];
extern uint8_t ip_addr[4];

/* Private function ======================================== */
static void task_gateway_handle(void *param);

void gateway_init(void)
{
    // Create event
    gateway_event = xEventGroupCreate();
    configASSERT(gateway_event);

    // Create gateway mutex
    gateway_data_lock = xSemaphoreCreateMutex();
    configASSERT(gateway_data_lock);

    // Creat task
    BaseType_t ret = xTaskCreate(task_gateway_handle, "gateway", 512 * 4, NULL, 25, &task_gateway);
    configASSERT(ret == pdTRUE);

    ESP_LOGI(GATEWAY_TAG, "Init");
}

static bool device_data_get(device_t *dev, cJSON *json)
{
    uint8_t field_count = cJSON_GetArraySize(json);
    if (field_count < 1)
        return false;

    dev->field_cnt = 0;
    for (uint8_t i = 0; i < field_count; i++)
    {
        field_t *field = &dev->fields[dev->field_cnt];
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (!item)
            continue;

        // Get "key"
        cJSON *tmp = cJSON_GetObjectItem(item, "key");
        if (!tmp)
            continue;
        ESP_LOGI(GATEWAY_TAG, "\tkey: %s", tmp->valuestring);
        int len = strlen(tmp->valuestring);
        len = len > 10 ? 10 : len;
        memset(field, 0, 10);
        memcpy(field->key, tmp->valuestring, len);

        // Get "valueType"
        tmp = cJSON_GetObjectItem(item, "valueType");
        if (!tmp)
            continue;
        ESP_LOGI(GATEWAY_TAG, "\tvalueType: %s", tmp->valuestring);
        len = strlen(tmp->valuestring);
        if (len == strlen(DataTypeFloat_Str))
        {
            if (!memcmp(tmp->valuestring, DataTypeFloat_Str, len))
                field->format = FormatFloat;
            else
                continue;
        }
        else if (len == strlen(DataTypeString_Str))
        {
            if (!memcmp(tmp->valuestring, DataTypeString_Str, len))
                field->format = FormatString;
            else
                continue;
        }
        else if (len == strlen(DataTypeInt_Str))
        {
            if (!memcmp(tmp->valuestring, DataTypeInt_Str, len))
                field->format = FormatInt;
            else
                continue;
        }
        else if (len == strlen(DataTypeUInt_Str))
        {
            if (!memcmp(tmp->valuestring, DataTypeUInt_Str, len))
                field->format = FormatUInt;
            else
                continue;
        }
        else if (len == strlen(DataTypeBool_str))
        {
            if (!memcmp(tmp->valuestring, DataTypeBool_str, len))
                field->format = FormatBool;
            else
                continue;
        }
        else
            continue;

        // Get "dataType"
        tmp = cJSON_GetObjectItem(item, "dataType");
        if (!tmp)
            continue;
        ESP_LOGI(GATEWAY_TAG, "\tdataType: %d", tmp->valueint);
        if (cJSON_IsNumber(tmp))
            field->type = tmp->valueint;
        else
            continue;

        dev->field_cnt++;
    }

    return (dev->field_cnt >= 1);
}

static bool init_device(cJSON *json)
{
    ESP_LOGI(GATEWAY_TAG, "Initialize device\n");
    uint8_t dev_num = cJSON_GetArraySize(json);
    if (!dev_num)
    {
        // Get device failure
        return false;
    }

    for (uint8_t i = 0; i < dev_num; i++)
    {
        device_t *device = &gateway.devs[gateway.dev_cnt];
        cJSON *dev_item = cJSON_GetArrayItem(json, i);
        if (!dev_item)
            continue;

        // Get device UUID
        cJSON *dev_uuid = cJSON_GetObjectItem(dev_item, "uuid");
        if (!dev_uuid)
            continue;
        ESP_LOGI(GATEWAY_TAG, "Device UUID: %s", dev_uuid->valuestring);
        uint8_t *uuid = uuid_str2byte((const char *)dev_uuid->valuestring);
        if (!uuid)
            continue;
        memcpy(device->uuid, uuid, 16);
        ESP_LOGI(GATEWAY_TAG, "Verify UUID: %s", uuid_byte2str(device->uuid));

        // Get device data
        cJSON *dev_data = cJSON_GetObjectItem(dev_item, "data");
        if (!dev_data)
            continue;

        if (device_data_get(device, dev_data))
            gateway.dev_cnt++;
        else
            continue;
    }

    return (gateway.dev_cnt >= 1);
}

void gateway_init_device(char *data)
{
    ESP_LOGI(GATEWAY_TAG, "Request init device");
    /*
    {
        "knownUUIDs": [
            {
                "uuid": "72b509fc-aca6-4560-8e40-dd60b33ea7cc",
                "data": [
                    {
                        "key": "bat",
                        "valueType": "string",
                        "dataType": 0
                    },
                    {
                        "key": "temp",
                        "valueType": "float",
                        "dataType": 1
                    }
                ]
            },
            {
                "uuid": "b1c310c5-a575-10a7-c29c-c1a5c28fbda2",
                "data": [
                    {
                        "key": "sos",
                        "valueType": "string",
                        "dataType": 0
                    },
                    {
                        "key": "acc",
                        "valueType": "int",
                        "dataType": 1
                    },
                    {
                        "key": "gyro",
                        "valueType": "int",
                        "dataType": 2
                    }
                ]
            }
        ]
    }
    */

    /* lock gateway data */
    xSemaphoreTake(gateway_data_lock, portMAX_DELAY);

    // Reset gateway device
    gateway.dev_cnt = 0;

    cJSON *json_root = cJSON_Parse((const char *)data);
    if (json_root == NULL)
        return;
    cJSON *known_uuid = cJSON_GetObjectItem(json_root, "knownUUIDs");
    if (known_uuid)
    {
        init_device(known_uuid);
    }
    cJSON_Delete(json_root);

    /* un-lock gateway data */
    xSemaphoreGive(gateway_data_lock);
}

void gateway_beacon_data_set(beacon_data_t *beacon)
{
    for (uint8_t d = 0; d < gateway.dev_cnt; d++)
    {
        device_t *dev = &gateway.devs[d];
        if (memcmp(dev->uuid, beacon->uuid, 16))
        {
            continue;
        }

        ESP_LOGI(GATEWAY_TAG, "beacon update");

        // Update mac address
        dev->mac_addr[0] = beacon->mac_addr[0];
        dev->mac_addr[1] = beacon->mac_addr[1];
        dev->mac_addr[2] = beacon->mac_addr[2];
        dev->mac_addr[3] = beacon->mac_addr[3];

        // Update device data
        for (uint8_t f = 0; f < dev->field_cnt; f++)
        {
            field_t *field = &dev->fields[f];
            if (beacon->type != field->type)
            {
                continue;
            }

            field->update = true;
            field->data[0] = beacon->data[0];
            field->data[1] = beacon->data[1];
            field->data[2] = beacon->data[2];
            field->data[3] = beacon->data[3];

            // Notify new data received to task publish to broker
            if (MQTT_connect())
            {
                xEventGroupSetBits(gateway_event, GATEWAY_EVENT_BIT_BEACON_DATA);
            }

            break;
        }
        break;
    }
}

void gateway_get_known_uuid(void)
{
    /*
    {
    "macAddress": "ec:94:cb:65:51:70",
    "ipaddress": "288.23.43.34",
    "firmwareVersion": "VALUE-forfirmware-version",
    "data": "NET_ID"
    }
    */
    char *t_buf = (char *)malloc(64);
    char *d_buf = (char *)malloc(256);
    bg22_assert(t_buf);
    bg22_assert(d_buf);
    memset(t_buf, 0, 64);
    memset(d_buf, 0, 256);

    snprintf(t_buf, 64, APP_GATEWAY_PUBLISH_TOPIC, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(d_buf, 256, "{\"macAddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"ipaddress\":\"%d:%d:%d:%d\",\"firmwareVersion\":\"%d.%d.%d\",\"data\":\"NET_ID\"}",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
             ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3],
             FW_MAJOR, FW_MINOR, FW_PATCH);

    MQTT_publish(t_buf, d_buf, strlen(d_buf));

    free(t_buf);
    free(d_buf);
}

/**********************************************************************
 * Implement private function.
 *********************************************************************/
static char topic_buf[64];
static char data_buf[512];
static char value_buf[16];
static void task_gateway_handle(void *param)
{
    (void)param;
    app_var_t var;
    EventBits_t bit;

    // Wait for authentication sucecss
    xEventGroupWaitBits(gateway_event, GATEWAY_EVENT_BIT_AUTHENTICATION, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(GATEWAY_TAG, "Device init sucecss!!!\n");

    while (true)
    {
        bit = xEventGroupWaitBits(gateway_event, GATEWAY_EVENT_BIT_BEACON_DATA, pdTRUE, pdTRUE, portMAX_DELAY);
        if ((bit & GATEWAY_EVENT_BIT_BEACON_DATA) == 0)
            continue;
        ESP_LOGI(GATEWAY_TAG, "Beacon update!!!\n");


        // Publish update data

        /* lock gateway device data */
        xSemaphoreTake(gateway_data_lock, portMAX_DELAY);

        for (uint8_t i = 0; i < gateway.dev_cnt; i++)
        {
            device_t *dev = &gateway.devs[i];
            // Check data field update
            for (uint8_t f = 0; f < dev->field_cnt; f++)
            {
                field_t *field = &dev->fields[f];
                if (!field->update)
                    continue;
                field->update = 0;

                memset(topic_buf, 0, sizeof(topic_buf));
                snprintf(topic_buf, sizeof(topic_buf), APP_GATEWAY_PUBLISH_TOPIC, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                // Json data
                /*
                    {
                    "macAddress": "ec:94:cb:65:51:70",
                    "clientMac": "ab:a4:cb:50:ab:72",
                    "data": "data_name",
                    "value": "valuexxx"
                    }
                */

                memset(data_buf, 0, sizeof(data_buf));
                memset(value_buf, 0, sizeof(value_buf));
                // Parse data
                var.buf[0] = field->data[0];
                var.buf[1] = field->data[1];
                var.buf[2] = field->data[2];
                var.buf[3] = field->data[3];

                switch (field->format)
                {
                case FormatFloat:
                    snprintf(value_buf, sizeof(value_buf), "%f", var.single);
                    break;
                case FormatString:
                    snprintf(value_buf, sizeof(value_buf), "%s", dev->fields[f].key);
                    break;
                case FormatUInt:
                    snprintf(value_buf, sizeof(value_buf), "%u", var.u32);
                    break;
                case FormatInt:
                    snprintf(value_buf, sizeof(value_buf), "%u", var.i32);
                    break;
                case FormatBool:
                    snprintf(value_buf, sizeof(value_buf), "%s", var.buf[0] ? "true" : "false");
                    break;
                default:
                    break;
                }

                snprintf(data_buf, sizeof(data_buf), "{\"macAddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"clientMac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"data\":\"%s\",\"value\":\"%s\"}",
                         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                         dev->mac_addr[0], dev->mac_addr[1], dev->mac_addr[2], dev->mac_addr[3], dev->mac_addr[4], dev->mac_addr[5],
                         dev->fields[f].key,
                         value_buf);
                         
                MQTT_publish((const char *)topic_buf, data_buf, strlen(data_buf));
                break;
            }

            /* un-lock gateway data */
            xSemaphoreGive(gateway_data_lock);
        }
    }
}
