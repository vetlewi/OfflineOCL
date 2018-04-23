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
    // Method to give names to some paramters
    void NameTimeParameters();

    // Method for calibrating energy of a detector.
    double CalibrateE(const word_t &w) const;

    // Method for getting time difference between two words.
    double CalcTimediff(const word_t &start, const word_t &stop) const;

    Histogram1Dp energy_labr_raw[NUM_LABR_DETECTORS], energy_labr[NUM_LABR_DETECTORS];
    Histogram1Dp energy_dE_raw[NUM_SI_DE_DET], energy_dE[NUM_SI_DE_DET];
    Histogram1Dp energy_E_raw[NUM_SI_E_DET], energy_E[NUM_SI_E_DET];
    Histogram2Dp time_e_de[NUM_SI_E_DET];

    Histogram1Dp time_labr[NUM_LABR_DETECTORS];
    Histogram2Dp energy_labr_all, time_labr_all;
    // Histogram2Dp time_ppac_labr[NUM_PPAC], time_de_labr[NUM_SI_DE_DET];
    Histogram2Dp time_ppac_labr[NUM_PPAC];

    Histogram2Dp time_energy_labr;
    Histogram2Dp time_energy_ppac_labr[NUM_PPAC]; // ppac_gated
    Histogram2Dp time_energy_ppac_labr_00, time_energy_ppac_labr_08; // ppac_gated

    Histogram2Dp time_walltime_ppac_labr_01, time_walltime_ppac_labr_06;


    Histogram2Dp time_ppac_de[NUM_PPAC];
    Histogram2Dp time_energy_ppac_de[NUM_PPAC];


    // DE - E spectra (everything in same...)
    Histogram2Dp ede_all, ede_raw[NUM_SI_E_DET][NUM_SI_RINGS], ede[NUM_SI_E_DET][NUM_SI_RINGS];
    Histogram2Dp ede_thick; // gated on the apparent thickness
    Histogram1Dp h_ede[NUM_SI_E_DET][NUM_SI_RINGS], h_ede_r[NUM_SI_RINGS], h_ex_r[NUM_SI_RINGS];
    Histogram1Dp h_ede_all, h_thick;

    // Particle gated E-DE
    Histogram2Dp ede_gate;

    // Excitation energy figure
    Histogram1Dp h_ex;

    // Particle - gamma-ray coincidence matrix
    Histogram2Dp alfna, alfna_bg;
    Histogram2Dp alfna_ppac, alfna_bg_ppac; // gated on ppacs

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

    // Coefficients of 2nd order Polynomial to calculate excitation energy from SiRi energy (E+dE).
    Parameter ex_from_ede;

    //   // Two rectangles to cut away SiRi noise/electrons
    //     Parameter ede_rect;

    // Apparent thickness gate SiRi
    Parameter thick_range;

    // Struct to hold the time gates
    struct TimeGate
    {
    double lower_prompt;
    double higher_prompt;
    double lower_bg;
    double higher_bg;
    };

    // Time gates for the NaI detectors, e.g. for making the ALFNA matrices
    Parameter labr_time_cuts;
    TimeGate labr_time_cut;

    // Time gates for the ppacs.
    Parameter ppac_time_cuts;
    TimeGate ppac_time_cut;


    int n_fail_de, n_fail_e;

    int n_tot_e, n_tot_de;
    int tot;


};

#endif // USERROUTINE2_H
