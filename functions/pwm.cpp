#include "pwm.h"
#include "dbc.h"

void Pwm::Update()
{
    bChannelEnabled = (bool)(m_pwm->enabled & (1 << 0));

    if (!pConfig->bEnabled || !bChannelEnabled) {
        bLastChannelEnabled = bChannelEnabled;
        bSoftStartComplete = false;
        nDutyCycle = 0;
    }

    if (!pConfig->bSoftStart) {
        nDutyCycle = GetTargetDutyCycle();
    }
    else
    {
        // Initialize soft start
        if (bChannelEnabled != bLastChannelEnabled) {
            InitSoftStart();
        }
        
        // Update soft start
        if (!bSoftStartComplete) {
            UpdateSoftStart();
        }
        // After soft start completes, continue updating duty cycle if variable
        else if (pConfig->bVariableDutyCycle) {
            nDutyCycle = GetTargetDutyCycle();
        }
    }

    UpdateFrequency();
    
    bLastChannelEnabled = bChannelEnabled;
}

uint8_t Pwm::GetTargetDutyCycle() {
    if (pConfig->bVariableDutyCycle && pConfig->nDutyCycleInputDenom > 0) {
        return (uint8_t)((*pInput) / pConfig->nDutyCycleInputDenom);
    }
    return pConfig->nFixedDutyCycle;
}

void Pwm::InitSoftStart() {
    fSoftStartStep = GetTargetDutyCycle() / (float)pConfig->nSoftStartRampTime;
    bSoftStartComplete = false;
    nSoftStartTime = SYS_TIME;
}

void Pwm::UpdateSoftStart() {
    uint8_t targetDuty = GetTargetDutyCycle();
    nDutyCycle = (uint8_t)(fSoftStartStep * (SYS_TIME - nSoftStartTime));
    
    if (nDutyCycle >= targetDuty) {
        nDutyCycle = targetDuty;
        bSoftStartComplete = true;
    }
}

void Pwm::UpdateFrequency()
{
    // Frequency within range and
    // (Frequency setting has changed or
    // PWM driver frequency != Frequency setting)
    if (((pConfig->nFreq <= 400) && (pConfig->nFreq > 0)) &&
        ((pConfig->nFreq != nLastFreq) ||
        (m_pwm->period != (m_pwmCfg->frequency / pConfig->nFreq))))
    {
        pwmChangePeriod(m_pwm, m_pwmCfg->frequency / pConfig->nFreq);
    }

    nLastFreq = pConfig->nFreq;
}

msg_t Pwm::Init()
{
    msg_t ret;
    ret = pwmStart(m_pwm, m_pwmCfg);
    if (ret != HAL_RET_SUCCESS)
        return ret;

    return HAL_RET_SUCCESS;
}

void Pwm::On()
{
    // PWM duty cycle is 0-10000
    //  100% = 10000
    //  50% = 5000
    //  0% = 0
    pwmEnableChannel(m_pwm, static_cast<uint8_t>(m_pwmCh), PWM_PERCENTAGE_TO_WIDTH(m_pwm, nDutyCycle * 100));
    pwmEnablePeriodicNotification(m_pwm);
    pwmEnableChannelNotification(m_pwm, static_cast<uint8_t>(m_pwmCh));
}

void Pwm::Off()
{
    pwmDisablePeriodicNotification(m_pwm);
    pwmDisableChannel(m_pwm, static_cast<uint8_t>(m_pwmCh));
}

MsgCmdResult Pwm::ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set output pwm settings
    // DLC 2 = Get output pwm settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 12, 4);
        if (nIndex < PDM_NUM_OUTPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stOutput[nIndex].stPwm.bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
                conf->stOutput[nIndex].stPwm.bSoftStart = Dbc::DecodeInt(rx->data8, 9, 1);
                conf->stOutput[nIndex].stPwm.bVariableDutyCycle = Dbc::DecodeInt(rx->data8, 10, 1);
                conf->stOutput[nIndex].stPwm.nDutyCycleInput = Dbc::DecodeInt(rx->data8, 16, 8);
                conf->stOutput[nIndex].stPwm.nFreq = Dbc::DecodeInt(rx->data8, 24, 9);
                conf->stOutput[nIndex].stPwm.nFixedDutyCycle = Dbc::DecodeInt(rx->data8, 33, 7);
                conf->stOutput[nIndex].stPwm.nSoftStartRampTime = Dbc::DecodeInt(rx->data8, 40, 16);
                conf->stOutput[nIndex].stPwm.nDutyCycleInputDenom = Dbc::DecodeInt(rx->data8, 56, 8);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::OutputsPwm) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.bEnabled, 8, 1);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.bSoftStart, 9, 1);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.bVariableDutyCycle, 10, 1);
            Dbc::EncodeInt(tx->data8, nIndex, 12, 4);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.nDutyCycleInput, 16, 8);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.nFreq, 24, 9);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.nFixedDutyCycle, 33, 7);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.nSoftStartRampTime, 40, 16);
            Dbc::EncodeInt(tx->data8, conf->stOutput[nIndex].stPwm.nDutyCycleInputDenom, 56, 8);

            if (rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}

void Pwm::SetDefaultConfig(Config_PwmOutput *config)
{
    config->bEnabled = false;
    config->bSoftStart = false;
    config->bVariableDutyCycle = false;
    config->nDutyCycleInput = 0;
    config->nFixedDutyCycle = 50;
    config->nFreq = 100;
    config->nSoftStartRampTime = 1000;
    config->nDutyCycleInputDenom = 100;
}