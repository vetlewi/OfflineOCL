#include "Event.h"

/*subevt_t::subevt_t(const word_t &w)
	: address( w.address )
	, adcdata( w.adcdata )
	, cfddata( w.cfddata )
{
	switch (GetSamplingFrequency(address)) {
		case f100MHz:
            cfdcorr = XIA_CFD_Fraction_100MHz(cfddata, &cfdfail);
            timestamp = 10*w.timestamp;
            break;
        case f250MHz:
            cfdcorr = XIA_CFD_Fraction_250MHz(cfddata, &cfdfail);
            timestamp = 8*w.timestamp;
            break;
        case f500MHz:
            cfdcorr = XIA_CFD_Fraction_500MHz(cfddata, &cfdfail);
            timestamp = 10*w.timestamp;
            break;
        default:
            break;
    }
}*/
