#include "Gamma.h"

#include "XIA_CFD.h"

#include <vector>
#include <algorithm>
#include <cmath>

static bool ByTimestamp(const tmp_w &a, const tmp_w &b)
{
    char fail;
    return (a.dataword.timestamp + XIA_CFD_Fraction_100MHz(a.dataword.cfddata, &fail) )
            < (b.dataword.timestamp + XIA_CFD_Fraction_100MHz(a.dataword.cfddata, &fail));
}

Gamma::Gamma(const double low[4][4], const double high[4][4], int fAddr)
    : firstAddr( fAddr )
{
    for (int i = 0 ; i < 4 ; ++i){
        for (int j = 0 ; j < 4 ; ++j){
            gate_low[i][j] = low[i][j];
            gate_high[i][j] = high[i][j];
        }
    }
}

bool Gamma::ProcessHits()
{
    if (nHits == 0)
        return false;
    std::vector<tmp_w> hits_all;
    int i, j;
    char fail;
    hits_all.assign(hits, hits+nHits);
    std::sort(hits_all.begin(), hits_all.end(), ByTimestamp);
    for (i = 0 ; i < nHits ; ++i)
        hits[i] = hits_all[i];

    int64_t start_c, diff_c;
    double start_f, diff_f, diff;
    double energy;
    for (i = 0 ; i < nHits ; ++i){

        start_c = hits[i].dataword.timestamp;
        start_f = XIA_CFD_Fraction_100MHz(hits[i].dataword.cfddata, &fail);
        energy = hits[i].energy;
        gamma[nG].timestamp = start_c;
        gamma[nG].cfdcorr = start_f;
        for (j = i+1 ; j < nHits ; ++j){
            diff_c = 10*(hits[j].dataword.timestamp - hits[i].dataword.timestamp);
            diff_f = XIA_CFD_Fraction_100MHz(hits[j].dataword.cfddata, &fail) - start_f;
            diff = diff_c + diff_f;
            if (std::abs(diff) < 250)
                energy += hits[j].energy;
            else
                i = j;
                break;
        }
        gamma[nG++].energy = energy;
    }
    return true;
}

/*
bool Gamma::ProcessHits()
{
	int64_t start_c, diff_c;
	double start_f, diff_f, diff;
	int fail;

	int start, stop;

	bool have_used[128];
	for (int i = 0 ; i < 128 ; ++i) have_used[i] = false;

    if (nHits == 0 || nHits > 128) // Check if we actually have data
		return false;

    for (int i = 0 ; i < nHits ; ++i){

        if (have_used[i]) // We have already counted this gamma, we'll go to next
			continue;

		start_c = hits[i].timestamp;
		start_f = XIA_CFD_Fraction_100MHz(hits[i].cfddata, &fail);
		word_t inCoins[128];
		double energies[128];
		int found=0;
		start = hits[i].address - firstAddr;
        energies[found] = calibE[i];
        inCoins[found++] = hits[i];
        have_used[i] = true;
        for (int j = 0 ; j < nHits ; ++j){
			stop = hits[i].address - firstAddr;
			if (i == j || have_used[j] || start == stop) // We do not count 
				continue;

			diff_c = 10*(hits[j].timestamp - start_c);
			diff_f = XIA_CFD_Fraction_100MHz(hits[j].cfddata, &fail) - start_f;
			diff = diff_c + diff_f;

			if (diff >= gate_low[start][stop] && diff <= gate_high[start][stop]){
				energies[found] = calibE[j];
				inCoins[found++] = hits[j];
				have_used[j] = true;
			}
		}
		gamma_t gam;
        gam.timestamp = 10*inCoins[0].timestamp;
        gam.cfdcorr = start_f;
        for (int j = 0 ; j < found ; ++j){
			gam.energy += energies[j];
            //gam.timestamp += inCoins[j].timestamp;
            //gam.cfdcorr += XIA_CFD_Fraction_100MHz(inCoins[j].cfdcorr, &fail);
		}
        //gam.timestamp *= 10;
        //gam.timestamp /= found;
        //gam.cfdcorr /= found;
        if (nG < 1024)
            gamma[nG++] = gam;
	}

    if (nG > 0)
        return true;
    return false;

}
*/

void Gamma::Reset()
{
    nHits = 0;
    nE = 0;
    nG = 0;
}
