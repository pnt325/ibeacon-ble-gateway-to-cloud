/*
 * gateway.c
 *
 *  Created on: May 15, 2022
 *      Author: Phat.N
 */

#include <string.h>
#include "gateway.h"
#include "app_config.h"
#include "mqtt.h"
#include "app_var.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Macro ================================================== */
#define DATA_FIELD_NAME_LEN 10
// #define DATA_FIELD_VALUE_LEN 10
#define DATA_FILED_MAX 12

#define DataTypeFloat_Str "float"
#define DataTypeString_Str "string"
#define DataTypeInt_Str "int"
#define DataTypeUInt_Str "uint"
#define DataTypeBool_str "bool"

#define GATEWAY_EVENT_BIT_BEACON_DATA       (1 << 0)
#define GATEWAY_EVENT_BIT_AUTHENTICATION    (1 << 1)

/* Type ==================================================== */
typedef enum
{
    TypeFloat,
    TypeString,
    TypeUInt,
    TypeInt,
    TypeBool,
    _TypeNum
} value_type_t;

typedef struct
{
    char key[DATA_FIELD_NAME_LEN]; // Json key
    uint8_t value_type;            // Json value format
    uint8_t data_type;             // Data code send form device.
} data_field_t __attribute__((packed));

typedef struct
{
    uint8_t addr[6]; // Device MAC address
    uint8_t data[4]; // Beacon data
    uint8_t type;    // Data type
} device_data_t __attribute__((packed));

typedef struct
{
    uint8_t uuid[16];                    // Device uuid
    data_field_t fields[DATA_FILED_MAX]; // Data field
    uint8_t field_count;                 // Data field count
    bool ready;                          // New data update
    device_data_t data;                  // Data
} device_t __attribute__((packed));

typedef struct
{
    device_t devices[APP_GATEWAY_DEVICE_MAX];
    uint8_t device_count; // number of device active on list
} gateway_t __attribute__((packed));

/* Private variable ======================================== */
static gateway_t gateway;
TaskHandle_t task_gateway;
EventGroupHandle_t gateway_event;
extern uint8_t mac[8];

/* Private function ======================================== */
static void task_gateway_handle(void *param);

void gateway_init(void)
{
    // Create event
    gateway_event = xEventGroupCreate();
    configASSERT(gateway_event);

    // Creat task
    task_gateway = xTaskCreate(task_gateway_handle, "gateway", 128 * 4, NULL, 25, NULL);
    configASSERT(task_gateway);
}

void gateway_init_device(char* data)
{
/*
{
    "knownUUIDs": [
        {
            "uuid": "72b509fcaca645608e40dd60b33ea7cc",
            "data": [
                {
                    "key": "sos",
                    "valueType": "string",
                    "dataType": 1
                },
                {
                    "key": "battery",
                    "valueType": "float",
                    "dataType": 2
                }
            ]
        },
        {
            "uuid": "b1c310c5a57510a7c29cc1a5c28fbda2",
            "data": [
                {
                    "key": "temp",
                    "valueType": "float",
                    "dataType": 1
                },
                {
                    "key": "acc",
                    "valueType": "int",
                    "dataType": 2
                },
                {
                    "key": "xyz",
                    "valueType": "int",
                    "dataType": 3
                }
            ]
        }
    ]
}
*/
    cJSON *auth_json = cJSON_Parse((const char*)data);
    if(auth_json == NULL) {
        return;
    }

    cJSON* known_uuid = cJSON_GetObjectItem(auth_json, "knownUUIDs")
    if(known_uuid == NULL) {
        return;
    }

    uint8_t array_len = cJSON_GetArraySize(known_uuid);
    if(array_len < 1) {
        return;
    }

    for(uint8_t i = 0; i < array_len; i++) 
    {
        cJSON* item = cJSON_GetArrayItem(known_uuid, i);
        if(item == NULL) {
            continue;
        }
        
        cJSON* uuid = cJSON_GetObjectItem(item);

        device_t* dev = &gateway.devices[gateway.device_count];
        if(uuid) {

        }


    }

    
    // if (cJSON_GetObjectItem(root, "password"))
    // {
    //     password = cJSON_GetObjectItem(root, "password")->valuestring;
    //     ESP_LOGI(TAG, "password=%s\n", password);
    // }
}

void gateway_beacon_data_set(beacon_data_t *beacon)
{
    for (uint8_t i = 0; i < gateway.device_count; i++)
    {
        device_t *dev = &gateway.devices[i];

        // FIXME Compare between byte array and char array
        if (memcmp(dev->uuid, data->uuid))
        {
            continue;
        }

        // Get data
        dev->data.addr[0] = beacon->addr[0];
        dev->data.addr[1] = beacon->addr[1];
        dev->data.addr[2] = beacon->addr[2];
        dev->data.addr[3] = beacon->addr[3];
        dev->data.addr[4] = beacon->addr[4];
        dev->data.addr[5] = beacon->addr[5];

        dev->data.data[0] = beacon->data[0];
        dev->data.data[1] = beacon->data[1];
        dev->data.data[2] = beacon->data[2];
        dev->data.data[3] = beacon->data[3];

        dev->data.type = beacon->rssi;

        // New data ready
        dev->ready = true;

        // Notify the task new data update
        if(MQTT_connect()) {
            xEventGroupSetBits(gateway_event, GATEWAY_EVENT_BIT_BEACON_DATA);
        }
        
        break;
    }
}

/**********************************************************************
 * Implement private function.
 *********************************************************************/
static void task_gateway_handle(void *param)
{
    (void)param;

    char topic[64];
    char data[512];
    char val[16];
    app_var_t var;
    EventBits_t bit;

    // Wait for authentication sucecss
    xEventGroupWaitBits(gateway_event, GATEWAY_EVENT_BIT_AUTHENTICATION, pdTRUE, pdTRUE, portMAX_DELAY);
    
    while (true)
    {
        bit = xEventGroupWaitBits(gateway_event, GATEWAY_EVENT_BIT_BEACON_DATA, pdTRUE, pdTRUE, portMAX_DELAY);
        if ((bit & GATEWAY_EVENT_BIT_BEACON_DATA) == 0)
        {
            continue;
        }

        // Publish update data
        for (uint8_t i = 0; i < gateway.device_count; i++)
        {
            device_t *dev = &gateway.devices[i];
            if (dev->ready == false)
            {
                continue;
            }
            dev->ready = false;

            // Check the data type matched
            uint8_t f;
            for (f = 0; f < dev->field_count; f++)
            {
                if (dev->data.type == dev->fileds[j].data_type)
                {
                    break;
                }
            }
            if (f >= dev->dev->field_count)
            {
                continue;
            }

            // Topic
            snprintf(topic, sizeof(topic), APP_GATEWAY_PUBLISH_TOPIC, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            // Json data
            /*
                {
                "macAddress": "ec:94:cb:65:51:70",
                "clientMac": "ab:a4:cb:50:ab:72",
                "data": "data_name",
                "value": "valuexxx"
                }
            */

            memset(data, 0, sizeof(data));
            memset(val, 0, sizeof(val));

            // Parse data
            var.buf[0] = dev->data.data[0];
            var.buf[1] = dev->data.data[1];
            var.buf[2] = dev->data.data[2];
            var.buf[3] = dev->data.data[3];
            switch (dev->fields[f].value_type)
            {
            case TypeFloat:
                snprintf(val, sizeof(val), "%f", var.single);
                break;
            case TypeString:
                snprintf(val, sizeof(val), "%s", dev->fields[f].key);
                break;
            case TypeUInt:
                snprintf(val, sizeof(val), "%u", var.u32);
                break;
            case TypeInt:
                snprintf(val, sizeof(val), "%u", var.i32);
                break;
            case TypeBool:
                snprintf(val, sizeof(val), "%s", var.buf[0] ? "true" : "false");
                break;
            default:
                break;
            }

            snprintf(data, sizeof(data), "{\"macAddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"clientMac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"data\":\"%s\",\"value\":\"%s\"}",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                     dev->data.addr[0], dev->data.addr[1], dev->data.addr[2], dev->data.addr[3], dev->data.addr[4], dev->data.addr[5],
                     dev->fileds[f].key,
                     val);

            MQTT_publish((const char*)topic, data, strlen(data));
        }
    }
}
