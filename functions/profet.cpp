#include "profet.h"
#include "dbc.h"

#if NUM_OUTPUTS > 0
void Profet::Update(bool bOutEnabled)
{
    eLastState = eState;

    if (!pConfig->bEnabled)
    {
        pwm.Off();
        palClearLine(m_in);
        nOcCount = 0;
        eState = ProfetState::Off;
        fOutput = 0;
        return;
    }

    HandleDsel();

    // Follower device — mirror primary
    if (pPrimary != nullptr)
    {
        FollowerUpdate();
        return;
    }

    if ((*pInput) && bOutEnabled)
        eReqState = ProfetState::On;
    else
        eReqState = ProfetState::Off;

    MeasureCurrent();

    // Sum follower current into primary's reported current
    if (pFollower != nullptr && pFollower->eState != ProfetState::Fault)
        fCurrent += pFollower->fCurrent;

    // Check for fault (device overcurrent/overtemp/short)
    // Raw ADC current reading will be very high
    if (nIS > 30000)
    {
        eState = ProfetState::Fault;
    }

    // Follower hardware fault is permanent — bring down the primary too
    if (pFollower != nullptr && pFollower->eState == ProfetState::Fault)
        eState = ProfetState::Fault;

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
        if (fCurrent > pConfig->fCurrentLimit && !bInRushActive)
        {
            nOcTriggerTime = SYS_TIME;
            nOcCount++;
            eState = ProfetState::Overcurrent;
        }

        // Inrush overcurrent
        if (fCurrent > pConfig->fInrushLimit && bInRushActive)
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
    fOutput = eState == ProfetState::On ? 1 : 0;
    fOvercurrent = eState == ProfetState::Overcurrent ? 1 : 0;
    fFault = eState == ProfetState::Fault ? 1 : 0;

    palClearLine(LINE_E2);
}

void Profet::FollowerUpdate()
{
    MeasureCurrent();

    // Hardware fault — independent, permanent
    if (nIS > 30000)
        eState = ProfetState::Fault;

    if (eState == ProfetState::Fault)
    {
        pwm.Off();
        palClearLine(m_in);
        fOutput = 0; 
        fOvercurrent = 0; 
        fFault = 1;
        return;
    }

    // Mirror primary state
    if (pPrimary->eState == ProfetState::On)
    {
        if (pPrimary->IsPwmEnabled())
        {
            pwm.EnsureStarted();
            pwm.OverrideFrequency(pPrimary->GetFrequency());
            pwm.SetDutyCycle(pPrimary->GetDutyCycle());
            // Align follower timer phase to primary on first enable so both switches
            // transition simultaneously. Both timers share the same APB clock so
            // there is no ongoing drift — only the initial phase offset needs fixing.
            if (eLastState != ProfetState::On)
                m_pwmDriver->tim->CNT = pPrimary->m_pwmDriver->tim->CNT;
            pwm.On();
        }
        else
        {
            pwm.Off();
            palSetLine(m_in);
        }
        eState = ProfetState::On;
    }
    else
    {
        pwm.Off();
        palClearLine(m_in);
        eState = pPrimary->eState;
    }

    fOutput = eState == ProfetState::On ? 1 : 0;
    fOvercurrent = eState == ProfetState::Overcurrent ? 1 : 0;
    fFault = 0;
}

void Profet::HandleDsel()
{
    // Select the appropriate IS channel on dual-channel devices
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
}

void Profet::MeasureCurrent()
{
    if (pwm.IsEnabled() && eState == ProfetState::On)
    {
        // Assign to local vars to prevent CNT rolling over and slipping past check
        // Example:
        // CCR = 2500
        // When checking read delay CNT = 2499
        // Before getting to the CNT < CCR check the CNT has rolled over to 0
        // This will cause an incorrect reading
        // Copying to local var freezes the CNT value
        uint32_t nCCR = m_pwmDriver->tim->CCR[static_cast<uint8_t>(m_pwmChannel)];
        uint32_t nCNT = m_pwmDriver->tim->CNT;

        // Delay for the first 1/3 of the PWM period to allow current to stabilize after switching, or a fixed min delay if the period is very short. 
        // This ensures we don't read during the switching transient
        uint32_t nDelay = (nCCR / 3 > nPwmReadDelay) ? nCCR / 3 : nPwmReadDelay;

        if ((nCCR > nDelay) &&
            (nCNT > nDelay) &&
            (nCNT < nCCR))
        {
            //==============================================================================
            //Trigger E2 on channel 1 for debug purposes to verify timing
            if (m_num == 1)
                palSetLine(LINE_E2);
            //==============================================================================

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

    CalculateCurrent();
}

void Profet::CalculateCurrent()
{
    // Current = (rawVal * (VDDA / 4095)) / 1.2k) * kILIS
    fCurrent = (((float)nIS * (GetVDDA() / 4095)) / 1200) * fKILIS;

    // Noise floor — ignore current below measurable threshold
    switch (m_model)
    {
    case ProfetModel::BTS7002_1EPP:
        if (fCurrent <= 0.5f)
            fCurrent = 0;
        break;
    case ProfetModel::BTS7008_2EPA_CH1:
    case ProfetModel::BTS7008_2EPA_CH2:
        if (fCurrent <= 0.2f)
            fCurrent = 0;
        break;
    case ProfetModel::BTS70012_1ESP:
        if (fCurrent <= 1.0f)
            fCurrent = 0;
        break;
    }
}

#endif
