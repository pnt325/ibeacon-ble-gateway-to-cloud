/*
 * gateway.c
 *
 *  Created on: May 15, 2022
 *      Author: Phat.N
 */

#include "gateway.h"

// TODO limit of data field to 12.

#define DATA_FIELD_NAME_LEN  10
// #define DATA_FIELD_VALUE_LEN 10
#define DATA_FILED_MAX       12

#define DataTypeFloat_Str  "float"
#define DataTypeString_Str "string" 
#define DataTypeInt_Str    "int"
#define DataTypeUInt_Str   "uint"
#define DataTypeBool_str   "bool"

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

typedef struct {    
    char uuid[16];
    data_field_t fields[DATA_FILED_MAX];
}


void gateway_init(void) {

}

void gateway_device_data_set(beacon_data_t* data) {

}