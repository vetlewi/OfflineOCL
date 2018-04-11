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

#include <printf.h>
#include <stdint.h>

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
    char tmp[1024], tmp1[1024], tmp2[1024], tmp3[1024];

    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_labr_%02d", i+1);
        energy_labr[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_cal_labr_%02d", i+1);
        energy_labr_cal[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

        sprintf(tmp, "time_labr_%02d", i+1);
        time_labr[i] = Spec(tmp, tmp, 5000, -2500, 2500, "Time t_{LaBr} - t_{#Delta E} [ns]");

    }

    for (int i = 0 ; i < NUM_SI_DE_DET ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_dE_%02d", i+1);
        energy_dE[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_cal_dE_%02d", i+1);
        energy_dE_cal[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

        sprintf(tmp, "time_de_labr_%02d", i+1);
        sprintf(tmp2, """t_{LaBr} - t_{dE %d} [ns]", i+1);
        time_de_labr[i] = Mat(tmp, tmp, 5000, -250, 250, tmp2, NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr_{3}(Ce) nr.");
    }

    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){

        // Create energy spectra
        sprintf(tmp, "energy_E_%02d", i+1);
        energy_E[i] = Spec(tmp, tmp, 32768, 0, 32768, "Energy [ch]");

        sprintf(tmp, "energy_cal_E_%02d", i+1);
        energy_E_cal[i] = Spec(tmp, tmp, 10000, 0, 10000, "Energy [keV]");

        sprintf(tmp, "time_e_de_%02d", i+1);
        time_e_de[i] = Mat(tmp, tmp, 5000, -250, 250, "Timediff. [ns]",
                                        8,    0,   8, "dE ring");
    }

    for (int i = 0 ; i < 8 ; ++i){
        for (int j = 0 ; j < 8 ; ++j){
            sprintf(tmp, "ede_b%02d_f%02d", i+1, j+1);
            ede[i][j] = Mat(tmp, tmp, 2000, 0, 20000, "E energy [ch]", 2000 , 0, 10000, "dE energy [ch]");
        }
    }

    sprintf(tmp, "ede_mat");
    ede_all = Mat(tmp, tmp, 32768 / 4, 0, 32768, "E energy [ch]", 32768 / 4, 0, 32768, "dE energy [ch]");

    sprintf(tmp, "h_part");
    h_particle = Spec(tmp, tmp, 1000, 0, 1000, "Apparent thickness [um]");

    sprintf(tmp, "h_ex");
    h_ex = Spec(tmp, tmp, 15000, 0, 30000, "Excitation energy [keV]");

    sprintf(tmp, "alfna_labr1");
    alfna_labr_1 = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [ch]", 1100, -1000, 15000, "Ex [keV]");

    for (int i = 0 ; i < 4 ; ++i){
        sprintf(tmp, "time_ppac_labr_%02d", i);
        time_ppac_labr[i] = Mat(tmp, tmp, 10000, -2500, 2500, "Time [ns]", NUM_LABR_DETECTORS, 0, NUM_LABR_DETECTORS, "LaBr nr.");
    }

    time_energy_labr1 = Mat("time_energy_labr1", "", 1024, 0, 32768, "Energy [ch]", 1000, -500, 500, "Time [ns]");

    n_fail_e = 0;
    n_fail_de = 0;
}


bool UserSort::Sort(const Event &event)
{
    int i, j;
    double energy;
    double tdiff;
    unsigned int e_seg=0;
    unsigned int de_seg=0;
    word_t e_word, de_word;
    for ( i = 0 ; i < NUM_LABR_DETECTORS ; ++i ){
        for ( j = 0 ; j < event.n_labr[i] ; ++j ){
            energy_labr[i]->Fill(event.w_labr[i][j].adcdata);
            energy = CalibrateE(event.w_labr[i][j]);
            energy_labr_cal[i]->Fill(energy);
        }
    }

    for ( i = 0 ; i < NUM_SI_DE_DET ; ++i ){
        for ( j = 0 ; j < event.n_dEdet[i] ; ++j ){
            energy_dE[i]->Fill(event.w_dEdet[i][j].adcdata);
            energy = CalibrateE(event.w_dEdet[i][j]);
            energy_dE_cal[i]->Fill(energy);
            de_seg = i;
            de_word = event.w_dEdet[i][j];
            if (de_word.cfdfail > 0)
                ++n_fail_de;
        }
    }

    for ( i = 0 ; i < NUM_SI_E_DET ; ++i ){
        for ( j = 0 ; j < event.n_Edet[i] ; ++j ){
            energy_E[i]->Fill(event.w_Edet[i][j].adcdata);
            energy = CalibrateE(event.w_Edet[i][j]);
            energy_E_cal[i]->Fill(energy);
            e_seg = i;
            e_word = event.w_Edet[i][j];
            if (e_word.cfdfail > 0)
                ++n_fail_e;
        }
    }

    for (i = 0 ; i < NUM_PPAC ; ++i){
        for (int j = 0 ; j < event.n_ppac[i] ; ++j){
            for (int n = 0 ; n < NUM_LABR_DETECTORS ; ++n){
                for ( int m = 0 ; m < event.n_labr[n] ; ++m){
                    tdiff = CalcTimediff(event.w_ppac[i][j], event.w_labr[n][m]);
                    time_ppac_labr[i]->Fill(tdiff, n);
                }
            }
        }
    }

    // Check if one and only one e & de has fired
    if (event.tot_dEdet == 1 && event.tot_Edet == 1 && GetDetector(de_word.address).telNum == GetDetector(e_word.address).telNum
            && de_word.cfdfail == 0){
        // Make ede_mat

        double e_energy = CalibrateE(e_word);
        double de_energy = CalibrateE(de_word);

        ede_all->Fill(e_word.adcdata, de_word.adcdata);

        ede[e_seg][de_seg-8*e_seg]->Fill(e_energy, de_energy);


        // Time diff.
        tdiff = CalcTimediff(e_word, de_word);
        time_e_de[e_seg]->Fill(tdiff, de_seg - 8 * e_seg);


        if (gain_E[e_seg] != 1 && gain_dE[de_seg] != 1 && std::abs(tdiff) <= 10){
            double thick = range.GetRange(e_energy+de_energy) - range.GetRange(e_energy);
            h_particle->Fill(thick);
            if (thick >= 100 && thick <= 160){
                double ex = 14.5333 - 1.030173*(e_energy+de_energy)/1000. - 0.001239*pow((e_energy+de_energy)/1000.,2);
                h_ex->Fill(e_energy + de_energy);

                // If we have arrived at this point we might as well do timing between
                // labr and dE.

                if (event.n_labr[0] > 0){
                    for (int j = 0 ; j < event.n_labr[0] ; ++j){
                        tdiff = CalcTimediff(de_word, event.w_labr[i][j]);
                        if (tdiff > -1 && tdiff < 10)
                            alfna_labr_1->Fill(CalibrateE(event.w_labr[i][j]), ex*1000);
                        else if (tdiff > 59 && tdiff < 70)
                            alfna_labr_1->Fill(CalibrateE(event.w_labr[i][j]), ex*1000, -1);
                    }
                }

                for (i = 0 ; i < NUM_LABR_DETECTORS ; ++i){
                    for (j = 0 ; j < event.n_labr[i] ; ++j){
                        tdiff = CalcTimediff(de_word, event.w_labr[i][j]);
                        time_labr[i]->Fill(tdiff);
                        time_de_labr[ GetDetector(de_word.address).detectorNum ]->Fill(tdiff, i);
                        if (i == 0)
                            time_energy_labr1->Fill(event.w_labr[i][j].adcdata, tdiff);
                    }
                }

            }
        }

    }


    return true;
}

bool UserSort::End()
{
    std::cout << "CFD fails in E - detectors: " << n_fail_e << std::endl;
    std::cout << "CFD fails in dE - detectors: " << n_fail_de << std::endl;
    return true;
}
