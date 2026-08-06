#include "Instrument.h"

void SyrInstrument_Destroy(SyrInstrument* instrument)
{
    if (instrument == NULL)
        return;

    free(instrument);
}
