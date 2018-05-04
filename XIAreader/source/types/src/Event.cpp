#include "Event.h"

#include "WordBuffer.h"
#include "experimentsetup.h"

#include <iostream>

void Event::PackEvent(const WordBuffer *buffer, int start, int stop)
{
    length = stop - start;
    DetectorInfo_t dinfo;
    for (int i = start ; i < stop ; ++i){
        dinfo = GetDetector((*buffer)[i].address);

        switch (dinfo.type) {
        case labr: {
            if ( n_labr[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_LABR_DETECTORS){
                w_labr[dinfo.detectorNum][n_labr[dinfo.detectorNum]++] = (*buffer)[i];
                ++tot_labr;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate LaBr word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case deDet: {
            if ( n_dEdet[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_SI_DE_DET){
                w_dEdet[dinfo.detectorNum][n_dEdet[dinfo.detectorNum]++] = (*buffer)[i];
                ++tot_dEdet;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate dEdet word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case eDet: {
            if ( n_Edet[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_SI_E_DET){
                w_Edet[dinfo.detectorNum][n_Edet[dinfo.detectorNum]++] = (*buffer)[i];
                ++tot_Edet;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate Edet word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case eGuard: {
            if ( n_Eguard[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_SI_E_GUARD){
                w_Eguard[dinfo.detectorNum][n_Eguard[dinfo.detectorNum]++] = (*buffer)[i];
                ++tot_Eguard;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate eGuard word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case ppac: {
            if ( n_ppac[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_PPAC){
                w_ppac[dinfo.detectorNum][n_ppac[dinfo.detectorNum]++] = (*buffer)[i];
                ++tot_ppac;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate PPAC word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case rfchan: {
            if ( n_RFpulse < MAX_WORDS_PER_DET )
                w_RFpulse[n_RFpulse++] = (*buffer)[i];
            else
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate RF word, run debugger with appropriate break point for more details" << std::endl;
            break;
        }
        default:
            break;
        }
    }
    return;
}
