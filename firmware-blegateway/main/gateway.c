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

#define DATA_FIELD_NAME_LEN  10
// #define DATA_FIELD_VALUE_LEN 10
#define DATA_FILED_MAX       12

#define DataTypeFloat_Str  "float"
#define DataTypeString_Str "string" 
#define DataTypeInt_Str    "int"
#define DataTypeUInt_Str   "uint"
#define DataTypeBool_str   "bool"

/* Type =================================================== */
typedef enum {
    TypeFloat,
    TypeString,
    TypeUInt,
    TypeInt,
    TypeBool,
    _TypeNum
} data_type_t;

typedef struct  {
    char name[DATA_FIELD_NAME_LEN];  // Json key
    data_type_t type;
} data_field_t;

typedef struct  {
    uint8_t addr[6];    // Device MAC address
    uint8_t data[4];    // Beacon data
    uint8_t prop;       // Properties
} device_data_t;

typedef struct {
    uint8_t       uuid[16];
    data_field_t  fields[DATA_FILED_MAX];
    uint8_t       enabled;
    uint8_t       ready;
    device_data_t data;
} gateway_device_t;

/* Private variable ======================================== */
static gateway_device_t devices[APP_GATEWAY_DEVICE_MAX];

/* Private function ======================================== */
static void gateway_task_handle(void* param);

void gateway_init(void) {
    // TODO create task and wait for authentication finish.
}

void gateway_device_data_set(beacon_data_t* data) {
    // Check the uuid.
    for(uint8_t i = 0;i < APP_GATEWAY_DEVICE_MAX; i++) {
        gateway_device_t *dev = &devices[i];

        // TODO Use lock to safe share resource.

        // Device are enabled to received data on the gateway
        if(dev->enabled == 0) {
            continue;
        }

        if(memcmp(data->uuid, dev->uuid, 16)) {
            continue;
        }

        // Get device IP
        dev->data.addr[0] = data->addr[0];
        dev->data.addr[1] = data->addr[1];
        dev->data.addr[2] = data->addr[2];
        dev->data.addr[3] = data->addr[3];
        dev->data.addr[4] = data->addr[4];
        dev->data.addr[5] = data->addr[5];

        // Get data
        dev->data.data[0] = data->udata[0];
        dev->data.data[1] = data->udata[1];
        dev->data.data[2] = data->udata[2];
        dev->data.data[3] = data->udata[3];

        dev->data.prop    = data->rssi;
        ddv->ready        = 1;

        // TODO notify new data ready to send
        break;
    }
}

/**********************************************************************
 * Implement private function.
 *********************************************************************/
static void gateway_task_handle(void* param) {
    (void)param;

    while(true) {
        for(uint8_t i  = 0; i < APP_GATEWAY_DEVICE_MAX; i++) {
            gateway_device_t *dev = &devices[i];
            if(dev->ready) {
                dev->ready = 0;

                // Copy device data
                device_data_t data = dev->data;

                // TODO Convert data to json message



                //TODO handle and send data to MQTT
            }
        }
    }
}
