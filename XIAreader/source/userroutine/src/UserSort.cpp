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

void UserSort::NameTimeParameters()
 {
labr_time_cut.lower_prompt = labr_time_cuts[0];
labr_time_cut.higher_prompt = labr_time_cuts[1];
labr_time_cut.lower_bg = labr_time_cuts[2];
labr_time_cut.higher_bg = labr_time_cuts[3];

ppac_time_cut.lower_prompt = ppac_time_cuts[0];
ppac_time_cut.higher_prompt = ppac_time_cuts[1];
ppac_time_cut.lower_bg = ppac_time_cuts[2];
ppac_time_cut.higher_bg = ppac_time_cuts[3];
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
    char tmp[1024], tmp1[1024], tmp2[1024], tmp3[1024], tmp_title[1024];

    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_raw_labr_%02d", i+1);
        energy_labr_raw[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_labr_%02d", i+1);
        energy_labr[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

        sprintf(tmp, "time_labr_%02d", i+1);
        time_labr[i] = Spec(tmp, tmp, 5000, -2500, 2500, "Time t_{LaBr} - t_{#Delta E} [ns]");

    }

    sprintf(tmp, "energy_labr");
    energy_labr_all = Mat(tmp, tmp, 10000, 0, 10000, "Energy [keV]", NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr_{3}(Ce) nr.");

    sprintf(tmp, "time_labr");
    time_labr_all = Mat(tmp, tmp, 5000, -2500, 2500, "Time t_{LaBr} - t_{#Delta E} [ns]", NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr_{3}(Ce) nr.");

    for (int i = 0 ; i < NUM_SI_DE_DET ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_raw_dE_%d", i);
        energy_dE_raw[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_dE_%02d", i);
        energy_dE[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

//        sprintf(tmp, "time_de_labr_%d", i);
//        sprintf(tmp2, """t_{LaBr} - t_{dE %d} [ns]", i+1);
//        time_de_labr[i] = Mat(tmp, tmp, 5000, -250, 250, tmp2, NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr_{3}(Ce) nr.");
    }

    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_raw_E_%d", i);
        energy_E_raw[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_E_%d", i);
        energy_E[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

        sprintf(tmp, "time_e_de_%d", i);
        time_e_de[i] = Mat(tmp, tmp, 3000, -1500, 1500, "Timediff. [ns]",
                                        8,    0,   8, "dE ring");
    }

    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){
        for (int j = 0 ; j < NUM_SI_RINGS ; ++j){
            sprintf(tmp, "ede_b%d_f%d_raw", i, j);
            ede_raw[i][j] = Mat(tmp, tmp, 2000, 0, 32768, "E energy [ch]", 2000, 0, 32768, "dE energy [ch]");

            sprintf(tmp, "ede_b%d_f%d", i, j);
            ede[i][j] = Mat(tmp, tmp, 2000, 0, 20000, "E energy [keV]", 2000 , 0, 10000, "dE energy [keV]");

            sprintf(tmp, "h_ede_b%df%d", i, j);
            h_ede[i][j] = Spec( tmp, tmp, 2000, 0, 20000, "E+#DeltaE [keV]" );
        }
    }

    for(int f=0; f<NUM_SI_RINGS; ++f ) {
        sprintf(tmp, "h_ede_f%d", f);
        h_ede_r[f] = Spec(tmp, tmp, 15000, 0, 30000, "E+#DeltaE [keV]");

        sprintf(tmp, "h_edx_f%d", f);
        h_ex_r[f] =  Spec(tmp, tmp, 15000, 0, 30000, "E_{x} [keV]");
    }

    sprintf(tmp, "ede");
    ede_all = Mat(tmp, tmp, 2000, 0, 20000, "E energy [keV]", 2000, 0, 20000, "dE energy [keV]");

    sprintf(tmp, "ede_thick");
    ede_thick = Mat(tmp, tmp, 2000, 0, 20000, "E energy [keV]", 2000, 0, 20000, "dE energy [keV]");

    sprintf(tmp, "h_thick");
    h_thick = Spec(tmp, tmp, 1000, 0, 1000, "Apparent thickness [um]");

    sprintf(tmp, "h_ede");
    h_ede_all = Spec(tmp, tmp, 15000, 0, 30000, "E + dE [keV]");

    sprintf(tmp, "h_ex");
    h_ex = Spec(tmp, tmp, 15000, 0, 30000, "Excitation energy [keV]");

    for (int i = 0 ; i < NUM_PPAC ; ++i){
        sprintf(tmp, "time_ppac%d_labr", i);
        time_ppac_labr[i] = Mat(tmp, tmp, 10000, -2500, 2500, "Time [ns]", NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr nr.");

        sprintf(tmp, "time_energy_ppac%d_labr", i);
        time_energy_ppac_labr[i] = Mat(tmp, tmp, 400, -100, 100, "Time [ns]", 300, 0, 10000, "LaBr Energy [keV]");

        sprintf(tmp, "time_ppac%d_de", i);
        time_ppac_de[i] = Mat(tmp, tmp, 2500, -500, 500, "Time [ns]", NUM_SI_DE_DET, 0, NUM_SI_DE_DET, "dE nr.");

        sprintf(tmp, "time_energy_ppac%d_de", i);
        time_energy_ppac_de[i] = Mat(tmp, tmp, 400, -100, 100, "Time [ns]", 300, 0, 10000, "dE Energy [keV]");
    }

    time_energy_labr = Mat("time_energy_labr", "", 1024, 0, 32768, "Energy [ch]", 1000, -500, 500, "Time [ns]");

    sprintf(tmp, "time_energy_ppac_labr_00");
    time_energy_ppac_labr_00 = Mat(tmp, tmp, 400, -100, 100, "Time [ns]", 300, 0, 10000, "LaBr Energy [keV]");

    sprintf(tmp, "time_energy_ppac_labr_08");
    time_energy_ppac_labr_08 = Mat(tmp, tmp, 400, -100, 100, "Time [ns]", 300, 0, 10000, "LaBr Energy [keV]");

    sprintf(tmp, "time_walltime_ppac_labr_01_test");
    time_walltime_ppac_labr_01 = Mat(tmp, "Det. 01 -- Just a test -- not finished yet", 100, -20, 20, "TimeDiff PPAC - Labr [ns]", 24*3, 0, 1e14, "WallTime [ns]");

    sprintf(tmp, "time_walltime_ppac_labr_06_test");
    time_walltime_ppac_labr_06 = Mat(tmp, "Det. 06 -- Just a test -- not finished yet", 100, -20, 20, "TimeDiff PPAC - Labr [ns]", 24*3, 0, 1e14, "WallTime [ns]");


    sprintf(tmp, "alfna");
    alfna = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_bg");
    alfna_bg = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_ppac");
    sprintf(tmp_title, "alfna_ppac; not propper bg subtraction yet!");
    alfna_ppac = Mat(tmp, tmp_title, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_bg_ppac");
    sprintf(tmp_title, "alfna_ppac_bg; not all bg yet!");
    alfna_bg_ppac = Mat(tmp, tmp_title, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

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


    word_t de_words[32]; // List of dE hits from pads in front of the trigger E word.
    int n_de_words=0;

    // First fill some 'singles' spectra.
    for ( i = 0 ; i < NUM_LABR_DETECTORS ; ++i ){
        for ( j = 0 ; j < event.n_labr[i] ; ++j ){
            energy_labr_raw[i]->Fill(event.w_labr[i][j].adcdata);
            energy = CalibrateE(event.w_labr[i][j]);
            energy_labr[i]->Fill(energy);
            energy_labr_all->Fill(energy, i);
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

            tdiff = CalcTimediff(event.trigger, event.w_dEdet[i][j]);
            time_e_de[GetDetector(event.trigger.address).telNum]->Fill(tdiff, i - 8*GetDetector(event.trigger.address).telNum);

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
        unsigned int tel = GetDetector(e_word.address).telNum;

        // Fill DE - E matrices.
        ede_raw[tel][ring]->Fill(e_word.adcdata, de_word.adcdata);

        double e_energy = CalibrateE(e_word);
        double de_energy = CalibrateE(de_word);

        ede[tel][ring]->Fill(e_energy, de_energy);
        ede_all->Fill(e_energy, de_energy);

        // Fill time spectra (for alignment? Need to determine what we need later)
        tdiff = CalcTimediff(e_word, de_word);
        time_e_de[tel]->Fill(tdiff, ring);


        // Calculate 'apparent thickness'
        double thick = range.GetRange(e_energy + de_energy) - range.GetRange(e_energy);
        h_thick->Fill(thick);

        // Check if correct particle.
        if ( thick >= thick_range[0] && thick <= thick_range[1] ){

            ede_thick->Fill(e_energy, de_energy);

            // Calculate the excitation energy.
            double e_tot = e_energy + de_energy;

            // Filling 'total particle energy' spectrum.
            h_ede[tel][ring]->Fill( e_tot );
            h_ede_r[ring]->Fill( e_tot );
            h_ede_all->Fill( e_tot );


            double ex = ex_from_ede[3*ring]; // Constant part.
            ex += ex_from_ede[3*ring + 1]*(e_tot)*1-3; // Linear part.
            ex += ex_from_ede[3*ring + 2]*( e_tot*1e-3 )*( e_tot*1e-3 ); // Quadratic term.
            ex *= 1000; // Back to keV units!


            h_ex_r[ring]->Fill(ex);
            h_ex->Fill(ex);

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
            time_labr[i]->Fill(tdiff);
            time_labr_all->Fill(tdiff, i);
            time_energy_labr->Fill(energy, tdiff);

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
            time_labr[i]->Fill(tdiff);
            time_labr_all->Fill(tdiff, i);
            time_energy_labr->Fill(energy, tdiff);

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


            for (int k = 0 ; k < NUM_PPAC ; ++k){
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
            }
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
