#include "profet.h"
#include "dbc.h"

void Profet::Update(bool bOutEnabled)
{
    eLastState = eState;

    if (!pConfig->bEnabled)
    {
        pwm.Off();
        palClearLine(m_in);
        nOcCount = 0;
        eState = ProfetState::Off;
        nOutput = 0;
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

    static uint32_t nCNT = 0;
    static uint32_t nCCR = 0;

    if (pwm.IsEnabled() && eState == ProfetState::On)
    {
        // Assign to local vars to prevent CNT rolling over and slipping past check
        // Example:
        // CCR = 2500
        // When checking read delay CNT = 2499
        // Before getting to the CNT < CCR check the CNT has rolled over to 0
        // This will cause an incorrect reading
        // Copying to local var freezes the CNT value
        nCCR = m_pwmDriver->tim->CCR[static_cast<uint8_t>(m_pwmChannel)];
        nCNT = m_pwmDriver->tim->CNT;

        if ((nCCR > nPwmReadDelay) &&
            (nCNT > nPwmReadDelay) &&
            (nCNT < nCCR))
        {
            palSetLine(LINE_E2);
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
    // NOTE: IS is converted to nCurrent = A * 10
    // Example: 0.1A = 1, 1.0A = 10, 10A = 100
    nCurrent = (uint16_t)((((float)nIS * (GetVDDA() / 4095)) / 1200) * fKILIS);

    // Ignore current less than a low value
    // Not capable of measuring that low anyways
    // Depending on the model the value changes
    switch (m_model)
    {
    case ProfetModel::BTS7002_1EPP:
        if (nCurrent <= 5) // 0.5A
            nCurrent = 0;
        break;
    case ProfetModel::BTS7008_2EPA_CH1:
    case ProfetModel::BTS7008_2EPA_CH2:
        if (nCurrent <= 2) // 0.2A
            nCurrent = 0;
        break;
    case ProfetModel::BTS70012_1ESP:
        if (nCurrent <= 10) // 1.0A
            nCurrent = 0;
        break;
    }

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

    // Set var map values
    nOutput = eState == ProfetState::On ? 1 : 0;
    nOvercurrent = eState == ProfetState::Overcurrent ? 1 : 0;
    nFault = eState == ProfetState::Fault ? 1 : 0;

    palClearLine(LINE_E2);
}

MsgCmdResult Profet::ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set output settings
    // DLC 2 = Get output settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 12, 4);
        if (nIndex < PDM_NUM_OUTPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stOutput[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
                conf->stOutput[nIndex].nInput = Dbc::DecodeInt(rx->data8, 16, 8);
                conf->stOutput[nIndex].nCurrentLimit = Dbc::DecodeInt(rx->data8, 24, 8, 10.0f);
                conf->stOutput[nIndex].eResetMode = static_cast<ProfetResetMode>(Dbc::DecodeInt(rx->data8, 32, 4));
                conf->stOutput[nIndex].nResetLimit = Dbc::DecodeInt(rx->data8, 36, 4);
                conf->stOutput[nIndex].nResetTime = Dbc::DecodeInt(rx->data8, 40, 8, 100.0f);
                conf->stOutput[nIndex].nInrushLimit = Dbc::DecodeInt(rx->data8, 48, 8, 10.0f);
                conf->stOutput[nIndex].nInrushTime = Dbc::DecodeInt(rx->data8, 56, 8, 100.0f);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::Outputs) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].bEnabled, 8, 1);
            Dbc::EncodeInt(tx->data8, nIndex, 12, 4);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].nInput, 16, 8);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].nCurrentLimit, 24, 8, 10.0f);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stOutput[nIndex].eResetMode), 32, 4);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].nResetLimit, 36, 4);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].nResetTime, 40, 8, 100.0f);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].nInrushLimit, 48, 8, 10.0f);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].nInrushTime, 56, 8, 100.0f);

            if (rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}

void Profet::SetDefaultConfig(Config_Output *config)
{
    config->bEnabled = false;
    config->nInput = 0;
    config->nCurrentLimit = 20;
    config->nInrushLimit = 30;
    config->nInrushTime = 1000;
    config->eResetMode = ProfetResetMode::None;
    config->nResetTime = 1000;
    config->nResetLimit = 0;
    
    Pwm::SetDefaultConfig(&config->stPwm);
}