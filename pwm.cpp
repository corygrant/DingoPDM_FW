#include "pwm.h"

void Pwm::Update()
{
    bChannelEnabled = (bool)(m_pwm->enabled & (1 << 0));

    if (!pConfig->bEnabled || !bChannelEnabled) {
        bLastChannelEnabled = bChannelEnabled;
        bSoftStartComplete = false;
        nDutyCycle = 0;
    }

    if (!pConfig->bSoftStart) {
        bSoftStartComplete = true;
        nDutyCycle = GetTargetDutyCycle();
    }

    // Initialize soft start
    if (bChannelEnabled != bLastChannelEnabled) {
        InitSoftStart();
    }

    // Update soft start
    if (!bSoftStartComplete) {
        UpdateSoftStart();
    }

    UpdateFrequency();
    
    bLastChannelEnabled = bChannelEnabled;
}

uint8_t Pwm::GetTargetDutyCycle() {
    if (pConfig->bVariableDutyCycle && pConfig->nDutyCycleInputDenom > 0) {
        return (*pInput) / pConfig->nDutyCycleInputDenom;
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
        uint8_t nIndex = (rx->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_OUTPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stOutput[nIndex].stPwm.bEnabled = (rx->data8[1] & 0x01);
                conf->stOutput[nIndex].stPwm.bSoftStart = (rx->data8[1] & 0x02) >> 1;
                conf->stOutput[nIndex].stPwm.bVariableDutyCycle = (rx->data8[1] & 0x04) >> 2;
                conf->stOutput[nIndex].stPwm.nDutyCycleInput = rx->data8[2];
                conf->stOutput[nIndex].stPwm.nFreq = (rx->data8[3] << 1) + (rx->data8[4] & 0x01);
                conf->stOutput[nIndex].stPwm.nFixedDutyCycle = (rx->data8[4] & 0xFE) >> 1;
                conf->stOutput[nIndex].stPwm.nSoftStartRampTime = (rx->data8[5] << 8) + rx->data8[6];
                conf->stOutput[nIndex].stPwm.nDutyCycleInputDenom = rx->data8[7];
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::OutputsPwm) + 128;
            tx->data8[1] = ((nIndex & 0x0F) << 4) + (conf->stOutput[nIndex].stPwm.bVariableDutyCycle << 2) +
                           (conf->stOutput[nIndex].stPwm.bSoftStart << 1) + conf->stOutput[nIndex].stPwm.bEnabled;
            tx->data8[2] = conf->stOutput[nIndex].stPwm.nDutyCycleInput;
            tx->data8[3] = (conf->stOutput[nIndex].stPwm.nFreq >> 1) & 0xFF;
            tx->data8[4] = ((conf->stOutput[nIndex].stPwm.nFixedDutyCycle & 0x7F) << 1) + (conf->stOutput[nIndex].stPwm.nFreq & 0x01);
            tx->data8[5] = (conf->stOutput[nIndex].stPwm.nSoftStartRampTime >> 8) & 0xFF;
            tx->data8[6] = conf->stOutput[nIndex].stPwm.nSoftStartRampTime & 0xFF;
            tx->data8[7] = conf->stOutput[nIndex].stPwm.nDutyCycleInputDenom;

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