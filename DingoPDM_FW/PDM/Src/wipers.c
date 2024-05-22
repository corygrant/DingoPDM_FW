#include "wipers.h"

static void Parked(Wiper_t* wiper)
{
  //Set motor to off
  wiper->nSlowOut = 0;
  wiper->nFastOut = 0;
  wiper->eSelectedSpeed = PARK;

  switch(wiper->eMode){
  case MODE_DIG_IN:
    if(*wiper->pInterInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = INTER_ON;
      wiper->eSelectedSpeed = INTER_1;
    }

    if(*wiper->pSlowInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      wiper->eSelectedSpeed = SLOW;
    }

    if(*wiper->pFastInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      wiper->eSelectedSpeed = FAST;
    }
    break;

  case MODE_INT_IN:
    //Set on based on selected speed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      wiper->eState = PARKING;
      break;

    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = INTER_ON;
      break;
    }
    break;

  case MODE_MIX_IN:
    //Switched on
    if(*wiper->pOnSw){
      //Set on based on selected speed
      switch(wiper->eSelectedSpeed)
      {
      case PARK:
        //Do nothing
        break;

      case SLOW:
        wiper->nSlowOut = 1;
        wiper->nFastOut = 0;
        wiper->eState = SLOW_ON;
        break;

      case FAST:
        wiper->nSlowOut = 1;
        wiper->nFastOut = 1;
        wiper->eState = FAST_ON;
        break;

      case INTER_1 ... INTER_6:
        wiper->nSlowOut = 1;
        wiper->nFastOut = 0;
        wiper->eState = INTER_ON;
        break;
      }
    }
    break;
  }

  if(*wiper->pWashInput){
     wiper->eStatePreWash = PARKED;
     wiper->eState = WASH;
  }

  if(*wiper->pSwipeInput){
    wiper->eState = SWIPE;
  }

}

static void Parking(Wiper_t* wiper)
{
  //Set to last state in case park is missed
  switch(wiper->eLastState)
  {
  case PARKED:
    //Do nothing
    break;
  case PARKING:
    //Do nothing
    break;
  case SLOW_ON:
    wiper->nSlowOut = 1;
    wiper->nFastOut = 0;
    break;
  case FAST_ON:
    wiper->nSlowOut = 1;
    wiper->nFastOut = 1;
    break;
  case INTER_ON:
    wiper->nSlowOut = 1;
    wiper->nFastOut = 0;
    break;
  case INTER_PAUSE:
    //Do nothing
    break;
  case WASH:
    wiper->nSlowOut = 1;
    wiper->nFastOut = 0;
    break;
  case SWIPE:
    wiper->nSlowOut = 1;
    wiper->nFastOut = 1;
    break;
  }

  //Park detected - stop motor
  //Park high or low depending on stop level
  if( (!(*wiper->pParkSw) && !(wiper->nParkStopLevel)) ||
      ((*wiper->pParkSw) && (wiper->nParkStopLevel == 1)))
  {
    wiper->nSlowOut = 0;
    wiper->nFastOut = 0;
    wiper->eState = PARKED;
  }

  //Wash turned on
  if(*wiper->pWashInput)
  {
    wiper->eStatePreWash = PARKING;
    wiper->eState = WASH;
  }
}

static void SlowOn(Wiper_t* wiper)
{
  wiper->eLastState = wiper->eState;

  //Set motor to slow speed
  wiper->nSlowOut = 1;
  wiper->nFastOut = 0;

  switch(wiper->eMode){
  case MODE_DIG_IN:
    if(!(*wiper->pSlowInput))
      wiper->eState = PARKING;

    if(*wiper->pInterInput){
      wiper->eState = INTER_ON;
    }

    if(*wiper->pFastInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
    }

    break;

  case MODE_INT_IN:
    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      wiper->eState = PARKING;
      break;

    case SLOW:
      //Do nothing
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      wiper->eState = INTER_ON;
      break;
    }
    break;

  case MODE_MIX_IN:
    //Wipers turned off - park
    if(!(*wiper->pOnSw))
    {
      wiper->eState = PARKING;
    }

    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      //Do nothing
      break;

    case SLOW:
      //Do nothing
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      wiper->eState = INTER_ON;
      break;
    }
    break;
  }

  //Wash turned on
  if(*wiper->pWashInput)
  {
    wiper->eStatePreWash = INTER_PAUSE;
    wiper->eState = WASH;
  }

}

static void FastOn(Wiper_t* wiper)
{
  wiper->eLastState = wiper->eState;

  //Set motor to fast speed
  wiper->nSlowOut = 1;
  wiper->nFastOut = 1;

  switch(wiper->eMode){
  case MODE_DIG_IN:
    if(!(*wiper->pFastInput))
      wiper->eState = PARKING;

    if(*wiper->pInterInput){
      wiper->eState = INTER_ON;
    }

    if(*wiper->pSlowInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
    }
    break;

  case MODE_INT_IN:
    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      wiper->eState = PARKING;
      break;

    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      //Do nothing
      break;

    case INTER_1 ... INTER_6:
      wiper->eState = INTER_ON;
      break;
    }
    break;

  case MODE_MIX_IN:
    //Wipers turned off - park
    if(!(*wiper->pOnSw))
    {
      wiper->eState = PARKING;
    }

    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      //Do nothing
      break;
    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      //Do nothing
      break;

    case INTER_1 ... INTER_6:
      wiper->eState = INTER_ON;
      break;
    }
    break;
  }

  //Wash turned on
  if(*wiper->pWashInput)
  {
    wiper->eStatePreWash = INTER_PAUSE;
    wiper->eState = WASH;
  }
}

static void InterOn(Wiper_t* wiper)
{
  wiper->eLastState = wiper->eState;

  //Set motor to slow speed
  wiper->nSlowOut = 1;
  wiper->nFastOut = 0;

  switch(wiper->eMode){
  case MODE_DIG_IN:
    if(!(*wiper->pInterInput)){
      wiper->eState = PARKING;
    }

    if(*wiper->pSlowInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
    }

    if(*wiper->pFastInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
    }

    break;

  case MODE_INT_IN:
    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      wiper->eState = PARKING;
      break;

    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      //Do nothing
      break;
    }
    break;

  case MODE_MIX_IN:
    //Wipers turned off - park
    if(!(*wiper->pOnSw))
    {
      wiper->eState = PARKING;
    }

    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      //Do nothing
      break;
    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      //Do nothing
      break;
    }
    break;
  }

  //Park detected
  //Stop motor
  //Save time - pause for set time
  if(!(*wiper->pParkSw))
  {
    wiper->nSlowOut = 0;
    wiper->nFastOut = 0;
    wiper->nInterPauseStartTime = HAL_GetTick();
    wiper->eState = INTER_PAUSE;
  }

  //Wash turned on
  if(*wiper->pWashInput)
  {
    wiper->eStatePreWash = INTER_ON;
    wiper->eState = WASH;
  }
}

static void InterPause(Wiper_t* wiper)
{
  wiper->eLastState = wiper->eState;

  //Stop motor
  wiper->nSlowOut = 0;
  wiper->nFastOut = 0;

  switch(wiper->eMode){
  case MODE_DIG_IN:
    if(!(*wiper->pInterInput)){
      wiper->eState = PARKING;
    }

    if(*wiper->pSlowInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
    }

    if(*wiper->pFastInput){
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
    }

    break;

  case MODE_INT_IN:
    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      wiper->eState = PARKING;
      break;

    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      //Do nothing
      break;
    }
    break;

  case MODE_MIX_IN:
    //Wipers turned off - park (should already be parked)
    if(!(*wiper->pOnSw))
    {
      wiper->eState = PARKING;
    }

    //Speed changed
    switch(wiper->eSelectedSpeed)
    {
    case PARK:
      //Do nothing
      break;
    case SLOW:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 0;
      wiper->eState = SLOW_ON;
      break;

    case FAST:
      wiper->nSlowOut = 1;
      wiper->nFastOut = 1;
      wiper->eState = FAST_ON;
      break;

    case INTER_1 ... INTER_6:
      //Do nothing
      break;
    }
    break;
  }

  //Copy inter delay to local variable
  uint16_t nDelay = 0;
  switch(wiper->eSelectedSpeed)
  {
  case PARK:
    //DO nothing
    break;
  case SLOW:
    //Do nothing
    break;
  case FAST:
    //Do nothing
    break;
  case INTER_1:
    nDelay = wiper->nInterDelays[0];
    break;
  case INTER_2:
    nDelay = wiper->nInterDelays[1];
    break;
  case INTER_3:
    nDelay = wiper->nInterDelays[2];
    break;
  case INTER_4:
    nDelay = wiper->nInterDelays[3];
    break;
  case INTER_5:
    nDelay = wiper->nInterDelays[4];
    break;
  case INTER_6:
    nDelay = wiper->nInterDelays[5];
    break;
  }

  //Pause for inter delay
  if((HAL_GetTick() - wiper->nInterPauseStartTime) > nDelay)
  {
    wiper->nSlowOut = 1;
    wiper->nFastOut = 0;
    wiper->eState = INTER_ON;
  }

  //Wash turned on
  if(*wiper->pWashInput)
  {
    wiper->eStatePreWash = INTER_PAUSE;
    wiper->eState = WASH;
  }

}

static void Wash(Wiper_t* wiper)
{
  wiper->eLastState = wiper->eState;

  if(wiper->eStatePreWash == FAST_ON){
    wiper->nSlowOut = 1;
    wiper->nFastOut = 1;
  }else{
    wiper->nSlowOut = 1;
    wiper->nFastOut = 0;
  }

  if(*wiper->pWashInput){
    wiper->nWashWipeCount = 0;
  }
  else{
    if(!(*wiper->pParkSw) && (*wiper->pParkSw != wiper->nLastParkSw))
    {
      wiper->nWashWipeCount++;
    }
    if(wiper->nWashWipeCount >= wiper->nWashWipeCycles){
      if(wiper->eStatePreWash == PARKED || wiper->eStatePreWash == PARKING)
        wiper->eState = PARKING;
      if(wiper->eStatePreWash == INTER_PAUSE || wiper->eStatePreWash == INTER_ON)
        wiper->eState = INTER_ON;
      if(wiper->eStatePreWash == SLOW_ON)
        wiper->eState = SLOW_ON;
      if(wiper->eStatePreWash == FAST_ON)
        wiper->eState = FAST_ON;
    }
  }
  wiper->nLastParkSw = *wiper->pParkSw;
}

static void Swipe(Wiper_t* wiper)
{
  wiper->eLastState = wiper->eState;

  //Swipe fast
  wiper->nSlowOut = 1;
  wiper->nFastOut = 1;

  //Park switch high
  //Moved past park position
  if(*wiper->pParkSw){
    wiper->eState = PARKING;
  }


}

void WiperSM(Wiper_t* wiper)
{
  if(!(wiper->nEnabled == 1)) return;

  if((wiper->eMode == MODE_MIX_IN) ||
      (wiper->eMode == MODE_INT_IN)){
    wiper->eSelectedSpeed = wiper->eSpeedMap[*wiper->pSpeedInput];
  }

  switch(wiper->eState)
  {
  case PARKED:
    Parked(wiper);
    break;

  case PARKING:
    Parking(wiper);
    break;

  case SLOW_ON:
    SlowOn(wiper);
    break;

  case FAST_ON:
    FastOn(wiper);
    break;

  case INTER_ON:
    InterOn(wiper);
    break;

  case INTER_PAUSE:
    InterPause(wiper);
    break;

  case WASH:
    Wash(wiper);
    break;

  case SWIPE:
    Swipe(wiper);
    break;
  }
}
