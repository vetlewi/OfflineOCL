/*******************************************************************************
 * Copyright (C) 2016 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, v.w.ingeberg@fys.uio.no                      *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

#ifndef USERSORT_H
#define USERSORT_H

#include "TDRRoutine.h"
#include "Event.h"

class UserSort : public TDRRoutine
{
public:
    UserSort();
    ~UserSort() { }

    bool Sort(const Event& event);

    bool End();

    //! We have no user commands that needs to be set.
    /*! \return true allways.
     */
    bool UserCommand(const std::string &cmd);

protected:
    void CreateSpectra();

private:

    // Method for calibrating energy of a detector.
    double CalibrateE(const word_t &w) const;

    // Method for getting time difference between two words.
    double CalcTimediff(const word_t &start, const word_t &stop) const;

    Histogram1Dp energy_labr[NUM_LABR_DETECTORS], energy_labr_cal[NUM_LABR_DETECTORS];
    Histogram1Dp energy_dE[NUM_SI_DE_DET], energy_dE_cal[NUM_SI_DE_DET];
    Histogram1Dp energy_E[NUM_SI_E_DET], energy_E_cal[NUM_SI_E_DET];
    Histogram2Dp time_e_de[NUM_SI_E_DET];

    Histogram1Dp time_labr[NUM_LABR_DETECTORS];
    Histogram2Dp time_ppac_labr[NUM_PPAC], time_de_labr[NUM_SI_DE_DET];

    Histogram2Dp time_energy_labr1;

    // DE - E spectra (everything in same...)
    Histogram2Dp ede_all, ede[NUM_SI_RINGS][NUM_SI_E_DET], ede_cal[NUM_SI_RINGS][NUM_SI_E_DET];

    // Particle gated E-DE
    Histogram2Dp ede_gate, alfna_labr_1;

    // Excitation energy figure
    Histogram1Dp h_ex, h_particle;

    // Gain labr
    Parameter gain_labr;

    // Shift labr
    Parameter shift_labr;

    // Gain dE
    Parameter gain_dE;

    // Shift dE
    Parameter shift_dE;

    // Gain E
    Parameter gain_E;

    // Shift E
    Parameter shift_E;

    // Time alignment LaBr
    Parameter shift_time_labr;

    // Time alignment dE
    Parameter shift_time_de;

    // Time alignment E
    Parameter shift_time_e;

    // Time alignment PPACs
    Parameter shift_time_ppac;

    int n_fail_de, n_fail_e;


};

#endif // USERSORT_H
