#ifndef GAMMA_H
#define GAMMA_H


#include "TDRWordBuffer.h"
#include <vector>

struct gamma_t
{
	double energy;		//!< Energy of gamma
	int64_t timestamp;	//!< Time of arrival (in ns)
	double cfdcorr;		//!< Correction from CFD to timestamp (in ns)
};

struct tmp_w
{
    word_t dataword;
    double energy;
};

class Gamma
{

public:

	//! Constructor
    Gamma(const double low[4][4], const double high[4][4], int firstAddr);

    //! Set first address
    inline void SetFirstAddress(const int &fAddr){ firstAddr = fAddr; }


	//! Get number of gammas found
    inline int GetNgamma() const { return nG; }


	//! Set a new hit
	inline void SetHit(const word_t &newhit, const double &calE)
		{
            if (nHits == nE && nE < 1024){
                hits[nHits].dataword = newhit;
                hits[nHits++].energy = calE;
                calibE[nE++] = calE;
            }
		}

	//! Process all hits
	bool ProcessHits();


	//! Get gamma N
    inline gamma_t GetGamma(const int &n) const
        { return (n >= 0 && n < nG) ? gamma[n] : gamma_t(); }

    void Reset();

private:

	//! Vector storing all hits.
    tmp_w hits[1024];
    int nHits;

	//! Vector storing all calibrated energies.
    double calibE[1024];
    int nE;

	//! Vector storing all the gamma events
    gamma_t gamma[1024];
    int nG;

	//! Time gates where first index is the start and the second is the stop.
	double gate_low[4][4];

	//! Time gates where first index is the start and the second is the stop.
	double gate_high[4][4];

	//! First address of the Clover detector
	int firstAddr;

};



#endif // GAMMA_H
