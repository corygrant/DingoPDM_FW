#pragma once

#define MAJOR_VERSION 0
#define MINOR_VERSION 5
#define BUILD 5

#define CAN_TX_CYCLIC_MSG_DELAY 100 //ms
#define CAN_TX_MSG_SPLIT 30 //us

#define USB_TX_MSG_SPLIT 30 //us

#define KEYPAD_TX_MSG_DELAY 100 //ms
#define KEYPAD_TX_MSG_SPLIT 5 //ms

#define BATT_LOW_VOLT 10.0f
#define BATT_HIGH_VOLT 16.0f

#define BOARD_TEMP_WARN 55 //deg C
#define BOARD_TEMP_CRIT 80 //deg C

#define MAX_COUNTER_VAL 8

#define CONFIG_TX_OFFSET 0 //To dingoConfig
#define CONFIG_RX_OFFSET 1 //From dingoConfig
#define CYCLIC_TX_OFFSET 2