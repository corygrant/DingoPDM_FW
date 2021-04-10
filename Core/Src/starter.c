/*
 * starter.c
 *
 *  Created on: Jan 6, 2021
 *      Author: coryg
 */

#include "starter.h"

void EvaluateStarter(PdmConfig_Starter_t *pStarter, uint8_t nIndex, uint16_t* pResult)
{
    if(!pStarter->nEnabled)
      *pResult = 1;
    else
      *pResult = !(pStarter->nDisableOut[nIndex] && *pStarter->pInput);
}
