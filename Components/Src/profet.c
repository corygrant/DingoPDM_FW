/*
 * profet.c
 *
 *  Created on: Nov 9, 2020
 *      Author: coryg
 */

#include "profet.h"

uint32_t GetTripTime(ProfetModelTypeDef eModel, uint16_t nIL, uint16_t nMaxIL);

//Transient state
static void TurningOff(volatile ProfetTypeDef *profet)
{
  *profet->nIN_Port &= ~profet->nIN_Pin;
  profet->eState = OFF;
}

static void Off(volatile ProfetTypeDef *profet)
{
  profet->cState = 'O';

  //Short circuit to battery check
  //TODO: Collapsing field will trigger this
  if (profet->nIL > 0.1) {
    //profet->eState = SHORT_CIRCUITING;
  }

  //Check for turn on
  if (profet->eReqState == ON) {
    profet->eState = TURNING_ON;
  }
}

//Transient state
static void InRushing(volatile ProfetTypeDef *profet)
{
  *profet->nIN_Port |= profet->nIN_Pin;
  profet->nIL_On_Time = HAL_GetTick();
  profet->eState  = IN_RUSH;
}

static void InRush(volatile ProfetTypeDef *profet)
{
  if (profet->nIL > profet->nIL_InRush_Limit) {
    profet->eState = OVERCURRENTING;
  }
  if((HAL_GetTick() - profet->nIL_On_Time) > profet->nIL_InRush_Time){
    profet->eState = TURNING_ON;
  }
  //Check for turn off
  if (profet->eReqState == OFF) {
    profet->eState = TURNING_OFF;
  }
}

//Transient state
static void TurningOn(volatile ProfetTypeDef *profet)
{
  *profet->nIN_Port |= profet->nIN_Pin;
  profet->eState = ON;
}

static void On(volatile ProfetTypeDef *profet)
{
  profet->cState = '|';

  //TODO: Dead short vs open load
  //Dead short will register no current
  //How to differentiate between open load?
  if (profet->nIL == 0) {
    //profet->nIN_Port->ODR &= ~profet->nIN_Pin;
    //profet->eState = SHORT_CIRCUIT;
  }

  //Check for fault (device overcurrent/overtemp/short)
  //IL will be very high
  //TODO: Calculate value from datasheet
  if (profet->nIS_Avg > 30000) {
    profet->eState = FAULTING;
  }

  //Check for turn off
  if (profet->eReqState == OFF) {
    profet->eState = TURNING_OFF;
  }

  if ((profet->nIL > profet->nIL_Limit) && (profet->nOC_Detected == 0)){
    profet->nIL_On_Time = HAL_GetTick();
    profet->nOC_Detected = 1;
  }

  if ((profet->nIL < profet->nIL_Limit) && (profet->nOC_Detected > 0)){
    profet->nOC_Detected = 0;
  }

  if(profet->nOC_Detected > 0){
    if((HAL_GetTick() - profet->nIL_On_Time) > GetTripTime(profet->eModel, profet->nIL, profet->nIL_Limit)){
      profet->eState = OVERCURRENTING;
    }
  }
}

//Transient state
static void Overcurrenting(volatile ProfetTypeDef *profet)
{
  profet->nValStore = profet->nIL;
  *profet->nIN_Port &= ~profet->nIN_Pin;
  profet->nOC_TriggerTime = HAL_GetTick();
  profet->nOC_ResetCount++;
  profet->eState = OVERCURRENT;
}

static void Overcurrent(volatile ProfetTypeDef *profet)
{
  profet->cState = 'C';
  *profet->nIN_Port &= ~profet->nIN_Pin;
  if(profet->nOC_ResetCount <= profet->nOC_ResetLimit){
    if((HAL_GetTick() - profet->nOC_TriggerTime) > profet->nOC_ResetTime){
      profet->eState = IN_RUSHING;
    }
  }
  else{
    profet->eState = SUSPENDING;
  }

  //Check for turn off
  if (profet->eReqState == OFF) {
    profet->nOC_ResetCount = 0;
    profet->eState = TURNING_OFF;
  }
}

//Transient state
static void ShortCircuiting(volatile ProfetTypeDef *profet)
{
  profet->nValStore = profet->nIL;
  profet->eState = SHORT_CIRCUIT;
}

static void ShortCircuit(volatile ProfetTypeDef *profet)
{
  profet->cState = 'S';
  *profet->nIN_Port &= ~profet->nIN_Pin;
}

//Transient state
static void Suspending(volatile ProfetTypeDef *profet)
{
  *profet->nIN_Port &= ~profet->nIN_Pin;
  profet->eState = SUSPENDED;
}

static void Suspended(volatile ProfetTypeDef *profet)
{
  profet->cState = 'X';
  //TODO: replace with a reset
  if (profet->eReqState == OFF){
    profet->nOC_ResetCount = 0;
    profet->eState = TURNING_OFF;
  }
}

//Transient state
static void Faulting(volatile ProfetTypeDef *profet)
{
  *profet->nIN_Port &= ~profet->nIN_Pin;
  profet->eState = FAULT;
}

static void Fault(volatile ProfetTypeDef *profet)
{
  profet->cState = 'F';
  *profet->nIN_Port &= ~profet->nIN_Pin;

  //TODO: Reset from fault requires DEN cycle
}

void Profet_SM(volatile ProfetTypeDef *profet) {

  switch (profet->eState) {
  case TURNING_OFF:
    TurningOff(profet);
    break;

  case OFF:
    Off(profet);
    break;

  case IN_RUSHING:
    InRushing(profet);
    break;

  case IN_RUSH:
    InRush(profet);
    break;

  case TURNING_ON:
    TurningOn(profet);
    break;

  case ON:
    On(profet);
    break;

  case SHORT_CIRCUITING:
    ShortCircuiting(profet);
    break;

  case SHORT_CIRCUIT:
    ShortCircuit(profet);
    break;

  case OVERCURRENTING:
    Overcurrenting(profet);
    break;

  case OVERCURRENT:
    Overcurrent(profet);
    break;

  case FAULTING:
    Faulting(profet);
    break;

  case FAULT:
    Fault(profet);
    break;

  case SUSPENDING:
    Suspending(profet);
    break;

  case SUSPENDED:
    Suspended(profet);
    break;

  }
}

void Profet_UpdateIS(volatile ProfetTypeDef *profet, uint16_t newVal)
{
  //Moving average without array or dividing
  //Store the new val, incase we need a non-filtered val elsewhere
  profet->nIS = newVal;
  //Add new value to old sum
  profet->nIS_Sum += profet->nIS;
  //Shift sum by 1 which is equal to dividing by 2
  profet->nIS_Avg = profet->nIS_Sum >> 1;
  //Remove the average from the sum, otherwise sum always goes up never down
  profet->nIS_Sum -= profet->nIS_Avg;

  //Convert IS to IL (actual current)
  profet->nIL = (uint16_t)(((float)profet->nIS_Avg * profet->fKilis) / 10.0);
}

uint32_t GetTripTime(ProfetModelTypeDef eModel, uint16_t nIL, uint16_t nMaxIL)
{
  //Calculate overcurrent multiplier
  //Multiply by 10 to include first decimal point
  // 25A / 5A = 5
  // 5 * 10 = 50 nOCMult
  //Subtract 10 to start at index 0
  // nOCMult = 40
  uint8_t nOCMult = (uint8_t)(((float)nIL / (float)nMaxIL) * 10.0);
  nOCMult -= 10; //Subtract 10 to start at index 0

  if(nOCMult < 0)
    nOCMult = 0;
  if(nOCMult > 91)
    nOCMult = 91;

  static const uint16_t fTripTimeLookupTable[91] =
  {
      /*
      0xF2AEU,0xC43AU,0xA1A1U,0x8737U,0x729FU,0x6248U,0x551CU,0x4A5AU,0x4175U,0x3A06U,0x33C1U,
      0x2E6BU,0x29D9U,0x25E6U,0x2278U,0x1F78U,0x1CD6U,0x1A82U,0x1871U,0x169BU,0x14F5U,0x137BU,
      0x1226U,0x10F2U,0x0FDBU,0x0EDDU,0x0DF5U,0x0D21U,0x0C5FU,0x0BADU,0x0B09U,0x0A72U,0x09E6U,
      0x0964U,0x08ECU,0x087DU,0x0815U,0x07B4U,0x0759U,0x0705U,0x06B6U,0x066BU,0x0626U,0x05E4U,
      0x05A7U,0x056DU,0x0536U,0x0502U,0x04D2U,0x04A4U,0x0478U,0x044EU,0x0427U,0x0402U,0x03DEU,
      0x03BDU,0x039DU,0x037EU,0x0361U,0x0345U,0x032BU,0x0312U,0x02FAU,0x02E3U,0x02CCU,0x02B7U,
      0x02A3U,0x0290U,0x027DU,0x026BU,0x025AU,0x024AU,0x023AU,0x022BU,0x021CU,0x020EU,0x0200U,
      0x01F3U,0x01E7U,0x01DBU,0x01CFU,0x01C4U,0x01B9U,0x01AEU,0x01A4U,0x019AU,0x0191U,0x0188U,
      0x017FU,0x0176U,0x016EU
      */
      0xFFFFU,0xCEF7U,0xA97FU,0x8D0DU,0x76FDU,0x658FU,0x5794U,0x4C33U,0x42D5U,0x3B08U,0x3479U,
      0x2EEAU,0x2A2AU,0x2613U,0x2288U,0x1F71U,0x1CBCU,0x1A59U,0x183DU,0x165DU,0x14B0U,0x1330U,
      0x11D7U,0x10A0U,0x0F86U,0x0E86U,0x0D9DU,0x0CC9U,0x0C06U,0x0B54U,0x0AB0U,0x0A1AU,0x098EU,
      0x090EU,0x0897U,0x0828U,0x07C1U,0x0762U,0x0708U,0x06B5U,0x0667U,0x061EU,0x05DAU,0x059AU,
      0x055EU,0x0525U,0x04F0U,0x04BDU,0x048EU,0x0461U,0x0437U,0x040EU,0x03E8U,0x03C4U,0x03A2U,
      0x0381U,0x0363U,0x0345U,0x0329U,0x030EU,0x02F5U,0x02DDU,0x02C6U,0x02AFU,0x029AU,0x0286U,
      0x0273U,0x0260U,0x024EU,0x023DU,0x022DU,0x021DU,0x020EU,0x0200U,0x01F2U,0x01E5U,0x01D8U,
      0x01CBU,0x01C0U,0x01B4U,0x01A9U,0x019EU,0x0194U,0x018AU,0x0181U,0x0177U,0x016EU,0x0166U,
      0x015DU,0x0155U,0x014EU
  };

  uint16_t nTripTimeRaw = fTripTimeLookupTable[nOCMult];

  static const float fTripTimeMult_7002[200] =
  {
      0.604,0.608,0.612,0.616,0.62,0.624,0.628,0.632,0.636,0.64,0.644,0.648,0.652,0.656,
      0.66,0.664,0.668,0.672,0.676,0.68,0.684,0.688,0.692,0.696,0.7,0.704,0.708,0.712,
      0.716,0.72,0.724,0.728,0.732,0.736,0.74,0.744,0.748,0.752,0.756,0.76,0.764,0.768,
      0.772,0.776,0.78,0.784,0.788,0.792,0.796,0.8,0.804,0.808,0.812,0.816,0.82,0.824,
      0.828,0.832,0.836,0.84,0.844,0.848,0.852,0.856,0.86,0.864,0.868,0.872,0.876,0.88,
      0.884,0.888,0.892,0.896,0.9,0.904,0.908,0.912,0.916,0.92,0.924,0.928,0.932,0.936,
      0.94,0.944,0.948,0.952,0.956,0.96,0.964,0.968,0.972,0.976,0.98,0.984,0.988,0.992,
      0.996,1,1.004,1.008,1.012,1.016,1.02,1.024,1.028,1.032,1.036,1.04,1.044,1.048,1.052,
      1.056,1.06,1.064,1.068,1.072,1.076,1.08,1.084,1.088,1.092,1.096,1.1,1.104,1.108,1.112,
      1.116,1.12,1.124,1.128,1.132,1.136,1.14,1.144,1.148,1.152,1.156,1.16,1.164,1.168,
      1.172,1.176,1.18,1.184,1.188,1.192,1.196,1.2,1.204,1.208,1.212,1.216,1.22,1.224,1.228,
      1.232,1.236,1.24,1.244,1.248,1.252,1.256,1.26,1.264,1.268,1.272,1.276,1.28,1.284,1.288,
      1.292,1.296,1.3,1.304,1.308,1.312,1.316,1.32,1.324,1.328,1.332,1.336,1.34,1.344,1.348,
      1.352,1.356,1.36,1.364,1.368,1.372,1.376,1.38,1.384,1.388,1.392,1.396,1.4
  };

  static const float fTripTimeMult_7008[80] =
   {
       0.61,0.62,0.63,0.64,0.65,0.66,0.67,0.68,0.69,0.7,0.71,0.72,0.73,0.74,0.75,0.76,
       0.77,0.78,0.79,0.8,0.81,0.82,0.83,0.84,0.85,0.86,0.87,0.88,0.89,0.9,0.91,0.92,
       0.93,0.94,0.95,0.96,0.97,0.98,0.99,1,1.01,1.02,1.03,1.04,1.05,1.06,1.07,1.08,
       1.09,1.1,1.11,1.12,1.13,1.14,1.15,1.16,1.17,1.18,1.19,1.2,1.21,1.22,1.23,1.24,
       1.25,1.26,1.27,1.28,1.29,1.3,1.31,1.32,1.33,1.34,1.35,1.36,1.37,1.38,1.39,1.40
   };

  uint32_t nTripTime = 0;
  float fTripTimeMult = 0.0;

  switch(eModel){
  case BTS7002_1EPP:
    if(nMaxIL < 200)
      fTripTimeMult = fTripTimeMult_7002[nMaxIL];
    else
      fTripTimeMult = fTripTimeMult_7002[199];
    break;

  case BTS7008_2EPA_CH1:
  case BTS7008_2EPA_CH2:
    if(nMaxIL < 80)
      fTripTimeMult = fTripTimeMult_7008[nMaxIL];
    else
      fTripTimeMult = fTripTimeMult_7008[79];
    break;
  }

  nTripTime = (uint32_t)(nTripTimeRaw * fTripTimeMult);

  return nTripTime;

}

void Profet_Default_Init(volatile ProfetTypeDef pf[], uint16_t *pfGpioBank1, uint16_t *pfGpioBank2){

  pf[0].eModel = BTS7002_1EPP;
  pf[0].nNum = 0;
  pf[0].nIN_Port = pfGpioBank1;
  pf[0].nIN_Pin = 0x0080;
  pf[0].fKilis = 2.286;

  pf[1].eModel = BTS7002_1EPP;
  pf[1].nNum = 1;
  pf[1].nIN_Port = pfGpioBank1;
  pf[1].nIN_Pin = 0x0002;
  pf[1].fKilis = 2.286;

  pf[2].eModel = BTS7008_2EPA_CH1;
  pf[2].nNum = 2;
  pf[2].nIN_Port = pfGpioBank1;
  pf[2].nIN_Pin = 0x8000;
  pf[2].fKilis = 0.554;

  pf[3].eModel = BTS7008_2EPA_CH2;
  pf[3].eState = OFF;
  pf[3].nNum = 3;
  pf[3].nIN_Port = pfGpioBank1;
  pf[3].nIN_Pin = 0x1000;
  pf[3].fKilis = 0.554;

  pf[4].eModel = BTS7008_2EPA_CH1;
  pf[4].eState = OFF;
  pf[4].nNum = 4;
  pf[4].nIN_Port = pfGpioBank1;
  pf[4].nIN_Pin = 0x0800;
  pf[4].fKilis = 0.554;

  pf[5].eModel = BTS7008_2EPA_CH2;
  pf[5].eState = OFF;
  pf[5].nNum = 5;
  pf[5].nIN_Port = pfGpioBank1;
  pf[5].nIN_Pin = 0x0100;
  pf[5].fKilis = 0.554;

  pf[6].eModel = BTS7002_1EPP;
  pf[6].eState = OFF;
  pf[6].nNum = 6;
  pf[6].nIN_Port = pfGpioBank2;
  pf[6].nIN_Pin = 0x0002;
  pf[6].fKilis = 2.286;

  pf[7].eModel = BTS7002_1EPP;
  pf[7].eState = OFF;
  pf[7].nNum = 7;
  pf[7].nIN_Port = pfGpioBank2;
  pf[7].nIN_Pin = 0x0008;
  pf[7].fKilis = 2.286;

  pf[8].eModel = BTS7008_2EPA_CH1;
  pf[8].eState = OFF;
  pf[8].nNum = 8;
  pf[8].nIN_Port = pfGpioBank2;
  pf[8].nIN_Pin = 0x0010;
  pf[8].fKilis = 0.554;

  pf[9].eModel = BTS7008_2EPA_CH2;
  pf[9].eState = OFF;
  pf[9].nNum = 9;
  pf[9].nIN_Port = pfGpioBank2;
  pf[9].nIN_Pin = 0x0080;
  pf[9].fKilis = 0.554;

  pf[10].eModel = BTS7008_2EPA_CH1;
  pf[10].eState = OFF;
  pf[10].nNum = 10;
  pf[10].nIN_Port = pfGpioBank2;
  pf[10].nIN_Pin = 0x0100;
  pf[10].fKilis = 0.554;

  pf[11].eModel = BTS7008_2EPA_CH2;
  pf[11].eState = OFF;
  pf[11].nNum = 11;
  pf[11].nIN_Port = pfGpioBank2;
  pf[11].nIN_Pin = 0x0800;
  pf[11].fKilis = 0.554;
}
