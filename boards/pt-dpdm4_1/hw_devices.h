#pragma once

#include "port.h"
#include "profet.h"
#include "digital_input.h"
#include "analog_input.h"
#include "led.h"
#include "hardware/mcp9808.h"
#include "neopixels.h"

extern Profet pf[NUM_OUTPUTS];
extern Digital_Input digIn[NUM_DIG_INPUTS];
extern Analog_Input analogIn[NUM_ANALOG_INPUTS];
extern Led statusLed;
extern Led errorLed;
extern MCP9808 tempSensor;
extern NeoPixels intNeoPixels;