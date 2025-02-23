#include "profet.h"

//BTS7002_1EPP and BTS70012_1ESP current sense settle time = 3 * (tON + tsIS_ON)
//3 * (210us + 40us) = 750us
//BTS7008_2EPA current sense settle time = tON + tsIS_ON
//110us + 20us = 130us
//ADC conversion time = 60us
//Total time 1 channel = 750us + 60us = 810us
//Total time 2 channels = 130us + 60us = 190us

//100Hz = 10ms
//Min DC 1 channel = 810us / 10ms = 8.1%
//Min DC 2 channels = 190us / 10ms = 1.9%

//400Hz = 2.5ms
//Min DC 1 channel = 810us / 2.5ms = 32.4%
//Min DC 2 channels = 190us / 2.5ms = 7.6%
#define TIM_RISE_DELAY 300
#define TIM_FALL_DELAY 50

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

    if ((*pInput) && bOutEnabled)
        eReqState = ProfetState::On;
    else
        eReqState = ProfetState::Off;

    // Set DSEL pin to select the appropriate IS channel
    // Only valid on 2 channel devices
    // Wait for DSEL changeover and ADC conversion to complete
    // DSEL changeover takes ~60us
    // ADC conversion takes ~60us 
    // Round up to 200us to ensure changeover/conversion finishes
    if (m_model == ProfetModel::BTS7008_2EPA_CH1)
    {
        palClearLine(m_dsel);

        chThdSleepMicroseconds(200);
    }
    else if (m_model == ProfetModel::BTS7008_2EPA_CH2)
    {
        palSetLine(m_dsel);
        // Wait for DSEL changeover (up to 60us)
        chThdSleepMicroseconds(200);
    }
    
    if (pConfig->bPwmEnabled && eState == ProfetState::On)
    {
        uint32_t nCCR = m_pwmDriver->tim->CCR[static_cast<uint8_t>(m_pwmChannel)];

        if((nCCR > (TIM_RISE_DELAY + TIM_FALL_DELAY)) &&
            (m_pwmDriver->tim->CNT > TIM_RISE_DELAY) && (m_pwmDriver->tim->CNT < (nCCR - TIM_FALL_DELAY)))
            nIS = GetAdcRaw(m_ain);
        else
        {
            nIS = nLastIS;
        }
    }
    else
        nIS = GetAdcRaw(m_ain);

    nLastIS = nIS;

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
        if(pConfig->bPwmEnabled)
            pwmDisableChannel(m_pwmDriver, static_cast<uint8_t>(m_pwmChannel));

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
        if (pConfig->bPwmEnabled)
        {
            pwmEnableChannel(m_pwmDriver, static_cast<uint8_t>(m_pwmChannel), PWM_PERCENTAGE_TO_WIDTH(m_pwmDriver, nDutyCycle));
            pwmEnableChannelNotification(m_pwmDriver, static_cast<uint8_t>(m_pwmChannel));
        }
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
        if(pConfig->bPwmEnabled)
            pwmDisableChannel(m_pwmDriver, static_cast<uint8_t>(m_pwmChannel));

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
        if(pConfig->bPwmEnabled)
            pwmDisableChannel(m_pwmDriver, static_cast<uint8_t>(m_pwmChannel));

        palClearLine(m_in);
        // Fault requires power cycle, no way out
        break;
    }

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