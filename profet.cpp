#include "profet.h"

void Profet::Update(bool bOutEnabled)
{
    eLastState = eState;

    if (!pConfig->bEnabled)
    {
        pwm.Off();
        palClearLine(m_in);
        nOcCount = 0;
        eState = ProfetState::Off;
        return;
    }

    if ((*pInput) && bOutEnabled)
        eReqState = ProfetState::On;
    else
        eReqState = ProfetState::Off;

    // Set DSEL pin to select the appropriate IS channel
    // Only valid on 2 channel devices
    // Wait for DSEL changeover and ADC conversion to complete
    // DSEL changeover takes max ~60us
    if (m_model == ProfetModel::BTS7008_2EPA_CH1)
    {
        palClearLine(m_dsel);
        chThdSleepMicroseconds(60);
    }
    else if (m_model == ProfetModel::BTS7008_2EPA_CH2)
    {
        palSetLine(m_dsel);
        chThdSleepMicroseconds(60);
    }
    
    uint32_t nCNT = 0;
    uint32_t nCCR = 0;

    if (pwm.IsEnabled() && eState == ProfetState::On)
    {
        //Assign to local vars to prevent CNT rolling over and slipping past check
        //Example: 
        //CCR = 2500
        //When checking read delay CNT = 2499
        //Before getting to the CNT < CCR check the CNT has rolled over to 0
        //This will cause an incorrect reading 
        //Copying to local var freezes the CNT value
        nCCR = m_pwmDriver->tim->CCR[static_cast<uint8_t>(m_pwmChannel)];
        nCNT = m_pwmDriver->tim->CNT; 

        if((nCCR > nPwmReadDelay) &&
            (nCNT > nPwmReadDelay) && 
            (nCNT < nCCR))
        {
            nIS = GetAdcRaw(m_ain);
            nLastIS = nIS;
        }
        else
        {
            nIS = nLastIS;
        }
    }
    else
        nIS = GetAdcRaw(m_ain);


    // Calculate current at ADC, multiply by kILIS ratio to get output current
    // Analog value must be ready before reading to allow for conversion after DSEL change
    // Use the measured VDDA value to calculate volts/step
    // Current = (rawVal * (VDDA / 4095)) / 1.2k) * kILIS
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

    bInRushActive = (pConfig->nInrushTime + nInRushOnTime) > SYS_TIME;

    switch (eState)
    {
    case ProfetState::Off:
        pwm.Off();

        palClearLine(m_in);
        
        nOcCount = 0;

        // Check for turn on
        if (eReqState == ProfetState::On)
        {
            nInRushOnTime = SYS_TIME;
            eState = ProfetState::On;
        }
        break;

    case ProfetState::On:
        if (pwm.IsEnabled())
            pwm.On();
        else
            palSetLine(m_in);

        // Check for turn off
        if (eReqState == ProfetState::Off)
        {
            eState = ProfetState::Off;
        }

        // Overcurrent
        if (nCurrent > pConfig->nCurrentLimit && !bInRushActive)
        {
            nOcTriggerTime = SYS_TIME;
            nOcCount++;
            eState = ProfetState::Overcurrent;
        }

        // Inrush overcurrent
        if (nCurrent > pConfig->nInrushLimit && bInRushActive)
        {
            nOcTriggerTime = SYS_TIME;
            nOcCount++;
            eState = ProfetState::Overcurrent;
        }

        break;

    case ProfetState::Overcurrent:
        pwm.Off();

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
        if ((pConfig->nResetTime + nOcTriggerTime) < SYS_TIME)
        {
            nInRushOnTime = SYS_TIME;
            eState = ProfetState::On;
        }

        // Check for turn off
        if (eReqState == ProfetState::Off)
        {
            eState = ProfetState::Off;
        }
        break;

    case ProfetState::Fault:
        pwm.Off();

        palClearLine(m_in);
        // Fault requires power cycle, no way out
        break;
    }

    pwm.Update();

    nOutput = eState == ProfetState::On ? 1 : 0;
}

MsgCmdResult Profet::ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set output settings
    // DLC 2 = Get output settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = (rx->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_OUTPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stOutput[nIndex].bEnabled = (rx->data8[1] & 0x01);
                conf->stOutput[nIndex].nInput = rx->data8[2];
                conf->stOutput[nIndex].nCurrentLimit = rx->data8[3] * 10;
                conf->stOutput[nIndex].eResetMode = static_cast<ProfetResetMode>(rx->data8[4] & 0x0F);
                conf->stOutput[nIndex].nResetLimit = (rx->data8[4] & 0xF0) >> 4;
                conf->stOutput[nIndex].nResetTime = rx->data8[5] * 100;
                conf->stOutput[nIndex].nInrushLimit = rx->data8[6] * 10;
                conf->stOutput[nIndex].nInrushTime = rx->data8[7] * 100;
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::Outputs) + 128;
            tx->data8[1] = ((nIndex & 0x0F) << 4) + (conf->stOutput[nIndex].bEnabled & 0x01);
            tx->data8[2] = conf->stOutput[nIndex].nInput;
            tx->data8[3] = (uint8_t)(conf->stOutput[nIndex].nCurrentLimit / 10);
            tx->data8[4] = ((conf->stOutput[nIndex].nResetLimit & 0x0F) << 4) +
                           (static_cast<uint8_t>(conf->stOutput[nIndex].eResetMode) & 0x0F);
            tx->data8[5] = (uint8_t)(conf->stOutput[nIndex].nResetTime / 100);
            tx->data8[6] = (uint8_t)(conf->stOutput[nIndex].nInrushLimit / 10);
            tx->data8[7] = (uint8_t)(conf->stOutput[nIndex].nInrushTime / 100);

            if (rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}