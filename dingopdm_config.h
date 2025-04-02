#pragma once

#define MAJOR_VERSION 0
#define MINOR_VERSION 4
#define BUILD 14

#define TX_SETTINGS_ID_OFFSET 30
#define TX_MSG_ID_OFFSET 31

#define CAN_TX_CYCLIC_MSG_DELAY 100 //ms
#define CAN_TX_MSG_SPLIT 30 //us

#define USB_TX_MSG_SPLIT 30 //us

#define BATT_LOW_VOLT 10.0f
#define BATT_HIGH_VOLT 16.0f

#define BOARD_TEMP_WARN 55 //deg C
#define BOARD_TEMP_CRIT 80 //deg C

#define MAX_COUNTER_VAL 8