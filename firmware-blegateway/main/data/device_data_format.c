typedef struct
{
    uint8_t type : 4; // 16 type
    uint8_t reserved : 4
} data_type_t;

typedef struct
{
    uint8_t uuid[16];
    uint8_t data[4];
    uint8_t rssi;       // Use to save data_type_t;
} beacon_data_t;
