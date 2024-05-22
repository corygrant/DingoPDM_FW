#ifndef COMPONENTS_INC_WIPERS_H_
#define COMPONENTS_INC_WIPERS_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"

//Wiper modes
//===All modes:
//Digital input for wash/swipe
//Integer input for selected inter time (unused = always first inter time)

// MODE_DIG_IN
// Digital inputs for slow/fast/inter

// MODE_INT_IN
// Integer input to select slow/fast/inter
// One integer value must be PARK

// MODE_MIX_IN
// Integer input to select slow/fast/inter
// Digital input for run/park


typedef enum
{
  MODE_DIG_IN,
  MODE_INT_IN,
  MODE_MIX_IN
} WiperMode_t;

typedef enum
{
  PARKED,
  PARKING,
  SLOW_ON,
  FAST_ON,
  INTER_PAUSE,
  INTER_ON,
  WASH,
  SWIPE
} WiperState_t;

typedef enum
{
  PARK=0,
  SLOW,
  FAST,
  INTER_1,
  INTER_2,
  INTER_3,
  INTER_4,
  INTER_5,
  INTER_6
} WiperSpeed_t;

typedef struct
{
  uint8_t nEnabled;

  WiperMode_t eMode;
  WiperState_t eState;

  //All Modes
  uint16_t nSlowOut;
  uint16_t nFastOut;
  uint16_t* pParkSw;
  uint8_t nParkStopLevel;
  uint16_t nInterDelays[6];
  uint16_t* pSwipeInput;
  uint16_t* pWashInput;
  uint8_t nWashWipeCycles;
  uint8_t nWashWipeCount;

  //MODE_DIG_IN
  uint16_t* pSlowInput;
  uint16_t* pFastInput;
  uint16_t* pInterInput;

  //MODE_INT_IN
  uint16_t* pSpeedInput;
  WiperSpeed_t eSpeedMap[8];
  WiperSpeed_t eSelectedSpeed;

  //MODE_MIX_IN
  uint16_t* pOnSw;

  //Internal
  uint32_t nInterPauseStartTime;
  WiperState_t eLastState;
  uint16_t nLastParkSw;
  WiperState_t eStatePreWash;
} Wiper_t;

void WiperSM(Wiper_t* wiper);

#endif /* COMPONENTS_INC_WIPERS_H_ */
