#include "digital_output.h"

void Digital_Output::Update()
{
    if(!pConfig->bEnabled)
    {
        fVal = 0;
        palWriteLine(m_line, 0);
        return;
    }

    palWriteLine(m_line, *pInput);
    fVal = *pInput;
}