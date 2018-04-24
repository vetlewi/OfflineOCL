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

#include "UserSort.h"

#include "DefineFile.h"

#include "TDRWordBuffer.h"
#include "DetectorRead.h"
#include "Histogram1D.h"
#include "Histogram2D.h"
#include "Histograms.h"
#include "Event.h"


#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <fstream>

#include <stdint.h>
#include <stdlib.h>

#define FISSION 0

static bool set_par(Parameters& parameters, std::istream& ipar,
                    const std::string& name, int size)
{
    Parameter* p = parameters.Find(name);
    if( !p )
        return false;

    std::vector<Parameter::param_t> values;
    float tmp;
    for(int i=0; i<size; ++i) {
        tmp = 0;
        ipar >> tmp;
        values.push_back(tmp);
    }
    p->Set( values );

    return true;
}

static bool set_gainshift(Parameters& parameters, std::istream& ipar)
{
    bool p1 = set_par(parameters, ipar, "gain_labr",    NUM_LABR_DETECTORS );
    bool p2 = set_par(parameters, ipar, "shift_labr",   NUM_LABR_DETECTORS );
    bool p3 = set_par(parameters, ipar, "gain_de", NUM_SI_DE_DET );
    bool p4 = set_par(parameters, ipar, "shift_de", NUM_SI_DE_DET );
    bool p5 = set_par(parameters, ipar, "gain_e", NUM_SI_E_DET );
    bool p6 = set_par(parameters, ipar, "shift_e", NUM_SI_E_DET );
    bool p7 = set_par(parameters, ipar, "shift_time_labr", NUM_LABR_DETECTORS );
    bool p8 = set_par(parameters, ipar, "shift_time_de", NUM_SI_DE_DET );
    bool p9 = set_par(parameters, ipar, "shift_time_e", NUM_SI_E_DET );
    bool p10 = set_par(parameters, ipar, "shift_time_ppac", NUM_PPAC );
    return (p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8 && p9 && p10);
}

UserSort::UserSort()
    : gain_labr( GetParameters(), "gain_labr", NUM_LABR_DETECTORS, 1)
    , shift_labr( GetParameters(), "shift_labr", NUM_LABR_DETECTORS, 0)
    , gain_dE( GetParameters(), "gain_de", NUM_SI_DE_DET, 1)
    , shift_dE( GetParameters(), "shift_de", NUM_SI_DE_DET, 0)
    , gain_E( GetParameters(), "gain_e", NUM_SI_E_DET, 1)
    , shift_E( GetParameters(), "shift_e", NUM_SI_E_DET, 0)
    , shift_time_labr( GetParameters(), "shift_time_labr", NUM_LABR_DETECTORS, 0)
    , shift_time_de( GetParameters(), "shift_time_de", NUM_SI_DE_DET, 0)
    , shift_time_e( GetParameters(), "shift_time_e", NUM_SI_E_DET, 0)
    , shift_time_ppac( GetParameters(), "shift_time_ppac", NUM_PPAC, 0)
    , ex_from_ede    ( GetParameters(), "ex_from_ede", NUM_SI_RINGS*3)
//    , ede_rect       ( GetParameters(), "ede_rect", 4         )
    , thick_range    ( GetParameters(), "thick_range", 2      )
    , labr_time_cuts  ( GetParameters(), "labr_time_cuts", 2*2  )
    , ppac_time_cuts ( GetParameters(), "ppac_time_cuts", 2*2 )
{
}


double UserSort::CalibrateE(const word_t &w) const
{
    DetectorInfo_t info = GetDetector(w.address);
    switch ( info.type ) {

    case labr : {
        return gain_labr[info.detectorNum]*(w.adcdata + drand48() - 0.5) + shift_labr[info.detectorNum];
    }
    case deDet : {
        return gain_dE[info.detectorNum]*(w.adcdata + drand48() - 0.5) + shift_dE[info.detectorNum];
    }
    case eDet : {
        return gain_E[info.detectorNum]*(w.adcdata + drand48() - 0.5) + shift_E[info.detectorNum];
    }
    case ppac : {
        return w.adcdata;
    }
    case rfchan : {
        return w.adcdata;
    }
    default:
        return w.adcdata;
    }
}

double UserSort::CalcTimediff(const word_t &start, const word_t &stop) const
{
    // First we fetch the correct shift parameters.
    double start_shift, stop_shift;

    DetectorInfo_t info_start = GetDetector(start.address);
    DetectorInfo_t info_stop = GetDetector(stop.address);

    switch ( info_start.type ){
        case labr : {
            start_shift = shift_time_labr[info_start.detectorNum];
            break;
        }
        case deDet : {
            start_shift = shift_time_de[info_start.detectorNum];
            break;
        }
        case eDet : {
            start_shift = shift_time_e[info_start.detectorNum];
            break;
        }
        case ppac : {
            start_shift = shift_time_ppac[info_start.detectorNum];
            break;
        }
        default : {
            start_shift = 0;
            break;
        }
    }

    switch ( info_stop.type ){
        case labr : {
            stop_shift = shift_time_labr[info_stop.detectorNum];
            break;
        }
        case deDet : {
            stop_shift = shift_time_de[info_stop.detectorNum];
            break;
        }
        case eDet : {
            stop_shift = shift_time_e[info_stop.detectorNum];
            break;
        }
        case ppac : {
            stop_shift = shift_time_ppac[info_stop.detectorNum];
            break;
        }
        default : {
            stop_shift = 0;
            break;
        }
    }

    // 'Coarse' time difference.
    int64_t diff_coarse = stop.timestamp - start.timestamp;

    // 'Fine' time difference
    double diff_fine = stop.cfdcorr -start.cfdcorr;

    // Actual time difference.
    double diff = diff_coarse + diff_fine;

    // Now we add shift of stop time and subtract shift of start time.
    diff += (stop_shift - start_shift);

    return diff;

}


bool UserSort::UserCommand(const std::string &cmd)
{
    std::istringstream icmd(cmd.c_str());

    std::string name, tmp;
    icmd >> name;

    if ( name == "Gain" ) {
        icmd >> tmp;
        if ( tmp == "file" ) {
            std::string filename;
            icmd >> filename;
            std::ifstream gainfile(filename.c_str());
            if ( !set_gainshift(GetParameters(), gainfile) ){
                std::cerr << __PRETTY_FUNCTION__ << ", gain file: error reading '" << filename << "'" << std::endl;
                return false;
            }
        } else if ( tmp=="data" ){
            if ( !set_gainshift(GetParameters(), icmd) ){
                std::cerr << __PRETTY_FUNCTION__ << ", gain data: error reading calibration data" << std::endl;
                return false;
            }
        } else {
            std::cerr << __PRETTY_FUNCTION__ << ", gain: Expected 'file' or 'data', not '" << tmp << "'" << std::endl;
        }
    } else if ( name == "parameter" ){
        return GetParameters().SetAll(icmd);
    } else {
        return false;
    }
    return true;
}


void UserSort::CreateSpectra()
{
    char tmp[1024], tmp2[1024];;

    // Allocating the LaBr 'singles' spectra
    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_raw_labr_%02d", i+1);
        energy_labr_raw[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_labr_%02d", i+1);
        energy_labr[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

        sprintf(tmp, "energy_time_labr_%02d", i+1);
        energy_time_labr[i] = Mat(tmp, tmp, 1000, 0, 16000, "LaBr energy [keV]", 2000, -100, 100, "Time difference [ns]");
    }

    // Allocating the dE 'singles' spectra
    for (int i = 0 ; i < NUM_SI_DE_DET ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_raw_dE_%02d", i);
        energy_dE_raw[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_dE_%02d", i);
        energy_dE[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");
    }

    // Allocating the LaBr 'singles' spectra
    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_raw_E_%02d", i);
        energy_E_raw[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_E_%02d", i);
        energy_E[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");
    }

    // Making all spectra that are indexed [NUM_SI_E_DET]
    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){

        // e_de_time spectrum
        sprintf(tmp, "e_de_time_%02d", i);
        e_de_time[i] = Mat(tmp, tmp, 3000, -1500, 1500, "Time t_{dE} - t_{E} [ns]", NUM_SI_RINGS, 0, NUM_SI_RINGS, "Ring number");

        // Making all spectra indexed [NUM_SI_E_DET][NUM_SI_RINGS].
        for (int j = 0 ; j < NUM_SI_RINGS ; ++j){

            // Make the 'raw' ede spectrum.
            sprintf(tmp, "ede_raw_b%d_f%d", i, j);
            sprintf(tmp2, "E : DE raw, pad %d, ring %d", i, j);
            ede_raw[i][j] = Mat(tmp, tmp2, 2048, 0, 32768, "Back energy [ch]", 2048, 0, 32768, "Front energy [ch]");

            // Make 'calibrated' ede spectrum.
            sprintf(tmp, "ede_b%d_f%d", i, j);
            sprintf(tmp2, "E : DE calibrated, pad %d, ring %d", i, j);
            ede[i][j] = Mat(tmp, tmp2, 2000, 0, 20000, "Back energy [keV]", 500, 0, 5000, "Front energy [keV]");

            // Make total energy spectra.
            sprintf(tmp, "h_ede_b%d_f%d", i, j);
            sprintf(tmp2, "Total energy deposited, pad %d, ring %d", i, j);
            h_ede[i][j] = Spec(tmp, tmp2, 20000, 0, 20000, "Total energy deposited [keV]");


            // Make excitation spectra.
            sprintf(tmp, "h_ex_b%d_f%d", i, j);
            sprintf(tmp2, "Singles excitation spectrum, pad %d, ring %d", i, j);
            h_ex[i][j] = Spec(tmp, tmp2, 20000, 0, 20000, "Excitation energy [keV]");
        }
    }

    // Time spectra (except those 'listed')
    sprintf(tmp, "de_align_time");
    sprintf(tmp2, "t_{dE} - t_{LaBr nr. 1}");
    de_align_time = Mat(tmp, tmp2, 3000, -1500, 1500, "t_{dE} - t_{LaBr nr. 1} [ns]", NUM_SI_DE_DET, 0, NUM_SI_DE_DET, "#Delta E detector id.");

    sprintf(tmp, "labr_align_time");
    sprintf(tmp2, "t_{LaBr} - t_{dE ANY}");
    labr_align_time = Mat(tmp, tmp2, 3000, -1500, 1500, "t_{LaBr} - t_{dE ANY} [ns]", NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr detector id.");

    sprintf(tmp, "energy_time_labr_all");
    sprintf(tmp2, "E_{LaBr} : t_{LaBr} - t_{dE ANY}, all");
    energy_time_labr_all = Mat(tmp, tmp2, 2000, 0, 16000, "Energy LaBr [keV]", 2000, -50, 50, "t_{LaBr} - t_{DE} [ns]");

    sprintf(tmp, "ede_all");
    sprintf(tmp2, "E : DE, all");
    ede_all = Mat(tmp, tmp2, 4000, 0, 20000, "Back energy [keV]", 1000, 0, 5000, "Front energy [keV]");

    sprintf(tmp, "ede_gate");
    sprintf(tmp2, "E : DE, after particle gate");
    ede_gate = Mat(tmp, tmp2, 1000, 0, 20000, "Back energy [keV]", 250, 0, 5000, "Front energy [keV]");

    sprintf(tmp, "h_thick");
    sprintf(tmp2, "Apparent thickness of #Delta E");
    h_thick = Spec(tmp, tmp2, 3000, 0, 3000, "Apparent thickness [#mu m]");

    sprintf(tmp, "h_ede_all");
    sprintf(tmp2, "Total particle energy, all");
    h_ede_all = Spec(tmp, tmp2, 20000, 0, 20000, "Particle energy [keV]");

    sprintf(tmp, "h_ex_all");
    sprintf(tmp2, "Excitation energy, all");
    h_ex_all = Spec(tmp, tmp2, 20000, 0, 20000, "Excitation energy [keV]");

    sprintf(tmp, "alfna");
    alfna = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1600, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_bg");
    alfna_bg = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1600, -1000, 15000, "Ex [keV]");

    n_fail_e = 0;
    n_fail_de = 0;
    n_tot_e = 0;
    n_tot_de = 0;
    tot = 0;
}


bool UserSort::Sort(const Event &event)
{
    int i, j;
    double energy;
    double tdiff;

    n_tot_e += event.tot_Edet;
    n_tot_de += event.tot_dEdet;
    tot += 1;


    word_t de_words[256]; // List of dE hits from pads in front of the trigger E word.
    int n_de_words=0;

    // First fill some 'singles' spectra.
    for ( i = 0 ; i < NUM_LABR_DETECTORS ; ++i ){
        for ( j = 0 ; j < event.n_labr[i] ; ++j ){
            energy_labr_raw[i]->Fill(event.w_labr[i][j].adcdata);
            energy = CalibrateE(event.w_labr[i][j]);
            energy_labr[i]->Fill(energy);
        }
    }

    for ( i = 0 ; i < NUM_SI_DE_DET ; ++i ){
        for ( j = 0 ; j < event.n_dEdet[i] ; ++j ){
            energy_dE_raw[i]->Fill(event.w_dEdet[i][j].adcdata);
            energy = CalibrateE(event.w_dEdet[i][j]);
            energy_dE[i]->Fill(energy);
            if (event.w_dEdet[i][j].cfdfail > 0) // For 'statistical' purposes!
                ++n_fail_de;
        }
    }

    for ( i = 0 ; i < NUM_SI_E_DET ; ++i ){
        for ( j = 0 ; j < event.n_Edet[i] ; ++j ){
            energy_E_raw[i]->Fill(event.w_Edet[i][j].adcdata);
            energy = CalibrateE(event.w_Edet[i][j]);
            energy_E[i]->Fill(energy);
            if (event.w_Edet[i][j].cfdfail > 0) // For 'statistical' purposes!
                ++n_fail_e;
        }
    }

    // We know that DE addresses should be as following:
    // 0 - 7: With E address 0.
    // 8 - 15: With E address 1.
    // 16 - 23: With E address 2.
    // 24 - 31: With E address 3.
    // 32 - 39: With E address 4.
    // 40 - 47: With E address 5.
    // 48 - 55: With E address 6.
    // 56 - 63: With E address 7.

    for (i = 8*GetDetector(event.trigger.address).telNum ; i < 8*(GetDetector(event.trigger.address).telNum+1) ; ++i){
        for (j = 0 ; j < event.n_dEdet[i] ; ++j){

            if (n_de_words < 256)
                de_words[n_de_words++] = event.w_dEdet[i][j];
        }

    }


    // Check if only one dE detector of the trapezoidal has fired.
    if ( n_de_words == 1){

        word_t e_word = event.trigger;
        word_t de_word = de_words[0];

        // The ring number and telescope number.
        unsigned int ring = GetDetector(de_word.address).detectorNum % 8; // Later we should define what we divide by somewhere else...
        int tel = GetDetector(e_word.address).telNum;

        tdiff = CalcTimediff(e_word, de_word);
        e_de_time[tel]->Fill(tdiff, ring);

        // Align the dE times...
        if ( event.n_labr[0] == 1){
            tdiff = CalcTimediff(event.w_labr[0][0], de_word);
            de_align_time->Fill(tdiff, GetDetector(de_word.address).detectorNum);
        }

        // Fill DE - E matrices.
        ede_raw[tel][ring]->Fill(e_word.adcdata, de_word.adcdata);

        double e_energy = CalibrateE(e_word);
        double de_energy = CalibrateE(de_word);

        ede[tel][ring]->Fill(e_energy, de_energy);

        // Seems like we may have some issues with the dE rings 6 & 7 (0-7). We will end our
        // sorting here if we have either 6 or 7.
        if (ring == 6 || ring == 7)
            return true;

        ede_all->Fill(e_energy, de_energy);

        // Calculate 'apparent thickness'
        double thick = range.GetRange(e_energy + de_energy) - range.GetRange(e_energy);
        h_thick->Fill(thick);

        // Check if correct particle.
        if ( thick >= thick_range[0] && thick <= thick_range[1] ){

            ede_gate->Fill(e_energy, de_energy);

            // Calculate the excitation energy.
            double e_tot = e_energy + de_energy;

            // Filling 'total particle energy' spectrum.
            h_ede[tel][ring]->Fill( e_tot );
            h_ede[tel][ring]->Fill( e_tot );
            h_ede_all->Fill( e_tot );


            double ex = ex_from_ede[3*ring]; // Constant part.
            ex += ex_from_ede[3*ring + 1]*(e_tot*1e-3); // Linear part.
            ex += ex_from_ede[3*ring + 2]*( e_tot*1e-3 )*( e_tot*1e-3 ); // Quadratic term.
            ex *= 1000; // Back to keV units!


            h_ex[tel][ring]->Fill(ex);
            h_ex_all->Fill(ex);

            // Analyze gamma rays.
        #if FISSION
            AnalyzeGammaPPAC(de_word, ex, event);
        #else
            AnalyzeGamma(de_word, ex, event);
        #endif // FISSION

        }
    }

    return true;

}


void UserSort::AnalyzeGamma(const word_t &de_word, const double &excitation,const Event &event)
{

    // We will loop over all gamma-rays.
    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){
        for (int j = 0 ; j < event.n_labr[i] ; ++j){

            // Get energy and time of the gamma-ray.

            double energy = CalibrateE(event.w_labr[i][j]);
            double tdiff = CalcTimediff(de_word, event.w_labr[i][j]);

            // Fill time spectra.
            labr_align_time->Fill(tdiff, i);
            energy_time_labr[i]->Fill(energy, tdiff);
            energy_time_labr_all->Fill(energy, tdiff);

            // Check time gate.
            switch ( CheckTimeStatus(tdiff, labr_time_cuts) ) {
                case is_prompt : {
                    alfna->Fill(energy, excitation);
                    break;
                }
                case is_background : {
                    alfna->Fill(energy, excitation, -1);
                    alfna_bg->Fill(energy, excitation);
                    break;
                }
                case ignore : {
                    break;
                }
            }
        }
    }
}

void UserSort::AnalyzeGammaPPAC(const word_t &de_word, const double &excitation, const Event &event)
{

    // Things should be implemented here...
    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){
        for (int j = 0 ; j < event.n_labr[i] ; ++j){

            // Get energy and time of the gamma-ray.

            double energy = CalibrateE(event.w_labr[i][j]);
            double tdiff = CalcTimediff(de_word, event.w_labr[i][j]);

            // Fill time spectra.
            labr_align_time->Fill(tdiff, i);

            // Check time gate.
            switch ( CheckTimeStatus(tdiff, labr_time_cuts) ) {
                case is_prompt : {
                    alfna->Fill(energy, excitation);
                    break;
                }
                case is_background : {
                    alfna_bg->Fill(energy, excitation);
                    break;
                }
                case ignore : {
                    break;
                }
            }


            /*for (int k = 0 ; k < NUM_PPAC ; ++k){
                for (int l = 0 ; l < event.n_ppac[k] ; ++l){
                    word_t ppac_word = event.w_ppac[k][l];

                    double tdiff_ppac_labr = CalcTimediff(ppac_word, event.w_labr[i][j]);
                    time_ppac_labr[k]->Fill(tdiff_ppac_labr, i);
                    time_energy_ppac_labr[k]->Fill(tdiff_ppac_labr, energy);

                    // Test: Try to detect whether there is a timewalk or synchornization error
                    // NOT WORKING YET
                    // Currently, it takes the "walltime" from the listtime timestamp. The time stamps there
                    // are currently reset every time one clicks "start" (/stop) in the aquisition software.
                    // Thus, during the 240Pu experiment, we had several times a resynchronization, which means
                    // in turn a potential misalignment for the next file
                    // We'll work on some method to correct for this.
                    if(i==0)
                        time_walltime_ppac_labr_01->Fill(tdiff_ppac_labr,ppac_word.timestamp);
                    if(i==5)
                        time_walltime_ppac_labr_06->Fill(tdiff_ppac_labr,ppac_word.timestamp);



                    if (tdiff_ppac_labr > ppac_time_cut.lower_prompt && tdiff_ppac_labr < ppac_time_cut.higher_prompt){
                        if(i==0)
                            time_energy_ppac_labr_00->Fill(tdiff_ppac_labr, energy);
                        if(i==8)
                            time_energy_ppac_labr_08->Fill(tdiff_ppac_labr, energy);
                    }


                    double tdiff_ppac_de = CalcTimediff(ppac_word, de_word);
                    time_ppac_de[k]->Fill(tdiff_ppac_de, GetDetector(de_word.address).detectorNum);
                    time_energy_ppac_de[k]->Fill(tdiff_ppac_de, CalibrateE(de_word));

                    // Gate on the prompt fission gamma rays
                    // Important: Not yet propperly bg subtracted
                    if (tdiff_ppac_labr > ppac_time_cut.lower_prompt && tdiff_ppac_labr < ppac_time_cut.higher_prompt){
                        alfna_ppac->Fill(energy, excitation);
                    }
                    else if (tdiff_ppac_labr > ppac_time_cut.lower_bg && tdiff_ppac_labr < ppac_time_cut.higher_bg){
                        alfna_ppac->Fill(energy, excitation, -1);
                        alfna_bg_ppac->Fill(energy, excitation, 1);
                    }
                }
            }*/
        }
    }
}




UserSort::prompt_status_t UserSort::CheckTimeStatus(const double &time, const Parameter &parameter) const
{
    if ( time > parameter[0] && time < parameter[1])
        return is_prompt;
    else if (time > parameter[2] && time < parameter[3])
        return is_background;
    else
        return ignore;
}

bool UserSort::End()
{
    std::cout << "Stats info: " << std::endl;
    std::cout << "CFD fails in E - detectors: " << n_fail_e << std::endl;
    std::cout << "CFD fails in dE - detectors: " << n_fail_de << std::endl;
    std::cout << "Average number of dE words: " << n_tot_de/double(tot) << std::endl;
    std::cout << "Average number of E words: " << n_tot_e/double(tot) << std::endl;
    return true;
}
