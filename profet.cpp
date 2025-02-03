#include "profet.h"

void Profet::Update(bool bOutEnabled)
{
    eLastState = eState;

    if (!pConfig->bEnabled)
    {
        palClearLine(m_in);
        nOcCount = 0;
        eState = ProfetState::Off;
        return;
    }

    if((*pInput) && bOutEnabled)
        eReqState = ProfetState::On;
    else
        eReqState = ProfetState::Off;
        

    //Set DSEL pin to select the appropriate IS channel
    //Only valid on 2 channel devices
    if(m_model == ProfetModel::BTS7008_2EPA_CH1)
    {
        palClearLine(m_dsel);
        //Wait for DSEL changeover (up to 60us)
        chThdSleepMicroseconds(100);
    }
    else if(m_model == ProfetModel::BTS7008_2EPA_CH2)
    {
        palSetLine(m_dsel);
        //Wait for DSEL changeover (up to 60us)
        chThdSleepMicroseconds(100);
    }

    // Calculate current at ADC, multiply by kILIS ratio to get output current
    // Analog value must be ready before reading to allow for conversion after DSEL change
    // Use the measured VDDA value to calculate volts/step
    // Current = (rawVal * (VDDA / 4095)) / 1.2k) * kILIS
    nIS = GetAdcRaw(m_ain);
    nCurrent = (uint16_t)((((float)nIS * (GetVDDA() / 4095)) / 1200) * fKILIS);

    // Ignore current less than or equal to 0.2A
    // Not capable of measuring that low
    // Noise causes small blips in current when output is off
    if (nCurrent <= 2)
        nCurrent = 0;

    // Check for fault (device overcurrent/overtemp/short)
    // Raw ADC current reading will be very high
    if (nIS > 30000)
    {
        eState = ProfetState::Fault;
    }

    bInRushActive = (pConfig->nInrushTime + nInRushOnTime) > chVTGetSystemTimeX();

    switch (eState)
    {
    case ProfetState::Off:
        palClearLine(m_in);
        nOcCount = 0;

        // Check for turn on
        if (eReqState == ProfetState::On)
        {
            nInRushOnTime = chVTGetSystemTimeX();
            eState = ProfetState::On;
        }
        break;

    case ProfetState::On:
        palSetLine(m_in);

        // Check for turn off
        if (eReqState == ProfetState::Off)
        {
            eState = ProfetState::Off;
        }

        // Overcurrent
        if (nCurrent > pConfig->nCurrentLimit && !bInRushActive)
        {
            nOcTriggerTime = chVTGetSystemTimeX();
            nOcCount++;
            eState = ProfetState::Overcurrent;
        }

        // Inrush overcurrent
        if (nCurrent > pConfig->nInrushLimit && bInRushActive)
        {
            nOcTriggerTime = chVTGetSystemTimeX();
            nOcCount++;
            eState = ProfetState::Overcurrent;
        }

        break;

    case ProfetState::Overcurrent:
        palClearLine(m_in);

        // No reset, straight to fault
        if (pConfig->eResetMode == ProfetResetMode::None)
        {
            eState = ProfetState::Fault;
        }

        // Overcurrent count exceeded
        if (nOcCount >= pConfig->nResetLimit && pConfig->eResetMode == ProfetResetMode::Count)
        {
            eState = ProfetState::Fault;
        }

        // Overcurrent reset time exceeded
        // ResetEndless or ResetCount
        if ((pConfig->nResetTime + nOcTriggerTime) < chVTGetSystemTimeX())
        {
            nInRushOnTime = chVTGetSystemTimeX();
            eState = ProfetState::On;
        }

        // Check for turn off
        if (eReqState == ProfetState::Off)
        {
            eState = ProfetState::Off;
        }
        break;

    case ProfetState::Fault:
        palClearLine(m_in);
        // Fault requires power cycle, no way out
        break;
    }

    nOutput = eState == ProfetState::On ? 1 : 0;

}