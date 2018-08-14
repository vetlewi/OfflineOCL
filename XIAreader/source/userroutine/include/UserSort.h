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

    // Enum describing the 'prompt' status of a gamma-ray
    typedef enum {
        is_prompt,          //!< To indicate that the hit is within the 'prompt' window.
        is_background,      //!< To indicate that the hit is within the 'background' window.
        ignore              //!< To indicate that the hit is neither in the 'prompt' nor the 'background window.
    } prompt_status_t;

    // Method to check if the time is within a gate.
    prompt_status_t CheckTimeStatus(const double &time,         /*!< Time of the hit        */
                                    const Parameter &paramter   /*!< Gates of the detector  */) const;

    // Method for calibrating energy of a detector.
    double CalibrateE(const word_t &w) const;

    // Method for getting time difference between two words.
    double CalcTimediff(const word_t &start, const word_t &stop) const;

    // Method for analyzing and checking conincident gamma events.
    void AnalyzeGamma(const word_t &de_word,    /*!< We need the de_word for the start time         */
                      const double &excitation, /*!< We need the reconstructed excitation energy    */
                      const Event &event        /*!< Event structure.                               */);

    // Method for analyzing and checking coinicident gamma-ppac events.
    void AnalyzeGammaPPAC(const word_t &de_word,    /*!< We need the de_word for the start time         */
                          const double &excitation, /*!< We need the reconstructed excitation energy    */
                          const Event &event        /*!< Event structure.                               */);

    // SINGLES histograms.
    Histogram1Dp energy_labr_raw[NUM_LABR_DETECTORS], energy_labr[NUM_LABR_DETECTORS];
    Histogram1Dp energy_dE_raw[NUM_SI_DE_DET], energy_dE[NUM_SI_DE_DET];
    Histogram1Dp energy_E_raw[NUM_SI_E_DET], energy_E[NUM_SI_E_DET];

    // Time spectra.
    Histogram2Dp e_de_time[NUM_SI_E_DET]; // This will be a time : dE ring spectrum.
    Histogram2Dp de_align_time;  // LaBr 0 as start? and dE as start. For aligning the dE detectors. Axis is x: time, y: dE detector nr.
    Histogram2Dp labr_align_time;  // LaBr as stop and dE as start. For aligning the LaBr detectors. Axis is x: time, y: LaBr detector nr.
    Histogram2Dp ppac_align_time;   // LaBr 0 as start and PPAC as stop. For aligning the PPACs.
    Histogram2Dp energy_time_labr[NUM_LABR_DETECTORS], energy_time_labr_above[NUM_LABR_DETECTORS];
    Histogram2Dp excitation_time_ppac[NUM_PPAC];
    Histogram2Dp energy_time_ppac[NUM_PPAC];
    Histogram2Dp energy_time_labr_all;  // Energy vs. time in labr for all labr detectors.


    // dE vs E
    Histogram2Dp ede_raw[NUM_SI_E_DET][NUM_SI_RINGS], ede[NUM_SI_E_DET][NUM_SI_RINGS];
    Histogram2Dp ede_all, ede_gate;

    // Misc. dE/E coincidence spectra stuff.
    Histogram1Dp h_thick;   // "Apparent" thickness spectra.
    Histogram1Dp h_ede[NUM_SI_E_DET][NUM_SI_RINGS], h_ede_all; // Total energy deposited after particle gate.
    Histogram1Dp h_ex[NUM_SI_E_DET][NUM_SI_RINGS], h_ex_all; // Excitation energy.


    // Particle - gamma-ray coincidence matrix
    Histogram2Dp exgam, exgam_bg;
    Histogram2Dp exgam_ppac, exgam_ppac_bg;
    Histogram2Dp exgam_veto_ppac, exgam_veto_ppac_bg;

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


    // Time gates for the LaBr detectors, e.g. for making the exgam matrices
    Parameter labr_time_cuts;

    // Time gates for the ppacs.
    Parameter ppac_time_cuts;


    int n_fail_de, n_fail_e;

    int n_tot_e, n_tot_de;
    int tot;


};

#endif // USERROUTINE2_H
