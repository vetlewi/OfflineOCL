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

//#include <printf.h>
#include <stdint.h>
#include <stdlib.h>

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
    char tmp[1024], tmp1[1024], tmp2[1024], tmp3[1024];

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

        sprintf(tmp, "energy_dE_%d", i);
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
        time_e_de[i] = Mat(tmp, tmp, 5000, -250, 250, "Timediff. [ns]",
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

    sprintf(tmp, "alfna");
    alfna = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_bg");
    alfna_bg = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_ppac");
    alfna_ppac = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    sprintf(tmp, "alfna_bg_ppac");
    alfna_bg_ppac = Mat(tmp, tmp, 1500, 0, 15000, "LaBr [keV]", 1100, -1000, 15000, "Ex [keV]");

    n_fail_e = 0;
    n_fail_de = 0;
}


bool UserSort::Sort(const Event &event)
{
    int i, j;
    double energy;
    double tdiff, tdiff_ppac_labr, tdiff_ppac_de;
    unsigned int e_seg=0;
    unsigned int de_seg=0;
    word_t e_word, de_word, ppac_word;

    NameTimeParameters();

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
            de_seg = i;
            de_word = event.w_dEdet[i][j];
            if (de_word.cfdfail > 0)
                ++n_fail_de;
        }
    }

    for ( i = 0 ; i < NUM_SI_E_DET ; ++i ){
        for ( j = 0 ; j < event.n_Edet[i] ; ++j ){
            energy_E_raw[i]->Fill(event.w_Edet[i][j].adcdata);
            energy = CalibrateE(event.w_Edet[i][j]);
            energy_E[i]->Fill(energy);
            e_seg = i;
            e_word = event.w_Edet[i][j];
            if (e_word.cfdfail > 0)
                ++n_fail_e;
        }
    }

//    for (i = 0 ; i < NUM_PPAC ; ++i){
//        for (int j = 0 ; j < event.n_ppac[i] ; ++j){
//            for (int n = 0 ; n < NUM_LABR_DETECTORS ; ++n){
//                for ( int m = 0 ; m < event.n_labr[n] ; ++m){
//                    tdiff = CalcTimediff(event.w_ppac[i][j], event.w_labr[n][m]);
//                    time_ppac_labr[i]->Fill(tdiff, n);
//                }
//            }
//        }
//    }


    // Check if only one SiRi combination fired
    if (event.tot_dEdet == 1 && event.tot_Edet == 1 // Check if one and only one E & dE has fired
            && GetDetector(de_word.address).telNum == GetDetector(e_word.address).telNum // And(!) require that dE and E are on the same pad
            && de_word.cfdfail == 0){ // Note: don't take event if cfd correction failed

        // Note: We can take the word assigned above -- if several E or dE detectors had fired, several
        //       words would be assigned. However, we anyhow demand that only one detectors had fired

        int de_ring = GetDetector(de_word.address).detectorNum % NUM_SI_RINGS; // dE ring number

        ede_raw[e_seg][de_ring]->Fill(e_word.adcdata, de_word.adcdata);

        double e_energy = CalibrateE(e_word);
        double de_energy = CalibrateE(de_word);

        ede_all->Fill(e_energy, de_energy);
        ede[e_seg][de_ring]->Fill(e_energy, de_energy);


        // Time diff.
        tdiff = CalcTimediff(e_word, de_word);
        time_e_de[e_seg]->Fill(tdiff, de_ring);


        double thick = range.GetRange(e_energy+de_energy) - range.GetRange(e_energy);
        h_thick->Fill(thick);

        // particle tickness gate
        if (thick >= thick_range[0] && thick <= thick_range[1]){
            ede_thick->Fill(e_energy, de_energy);

            double ex = ex_from_ede[3*de_ring+0] + ex_from_ede[3*de_ring+1]*(e_energy+de_energy)/1000. + ex_from_ede[3*de_ring+2]*pow((e_energy+de_energy)/1000.,2);
            ex *= 1000; // MeV -> keV

            h_ede[e_seg][de_ring]->Fill(e_energy + de_energy);
            h_ede_r[de_ring]->Fill(e_energy + de_energy);
            h_ede_all->Fill(e_energy + de_energy);

            h_ex_r[de_ring]->Fill(ex);
            h_ex->Fill(ex);

            // Loop over gamma-rays
            for (i = 0 ; i < NUM_LABR_DETECTORS ; ++i){ // Loop over detectors
                for (int j = 0 ; j < event.n_labr[i] ; ++j){ // Loop over events for each detector

                    double labr_energy = CalibrateE(event.w_labr[i][j]);
                    tdiff = CalcTimediff(de_word, event.w_labr[i][j]);

                    time_labr[i]->Fill(tdiff);
                    time_labr_all->Fill(tdiff,i);
                    time_energy_labr->Fill(labr_energy, tdiff);
                    //   time_de_labr[ GetDetector(de_word.address).detectorNum ]->Fill(tdiff, i);

                    if (tdiff > labr_time_cut.lower_prompt && tdiff < labr_time_cut.higher_prompt){
                        alfna->Fill(labr_energy, ex);
                    }
                    else if (tdiff > labr_time_cut.lower_bg && tdiff < labr_time_cut.higher_bg){
                        alfna->Fill(labr_energy, ex, -1);
                        alfna_bg->Fill(labr_energy, ex, 1);
                    }

                    // Fission related sorting

                    // TODO(!!):
                    // Copy from old sorting code:
                    //// keep this outside the if defined statement, such that if no PPACs are used, all events are considered
                    //// fiss = 0 -> by default, an event is not considered as a fission event; changed only if recognized in the for loop below
                    // int fiss = 0;
                    ////        if ( na_t_f>190 && na_t_f<220 && na_e_f>1195 && na_e_f<1225 ) fiss = 1;
                    // // Fabio: don't want energy requirement at the moment
                    ////        if ( CheckPPACpromptGate(ppac_t_c) &&  fission_excitation_energy_min[0] < e )   fiss = 1; // select fission blob in tPPAC vs E_SiRi gate
                    ////        if ( CheckPPACbgGate(ppac_t_c)     &&  fission_excitation_energy_min[0] < e )   fiss = 2; // added these to also see background fissions

                    for (int k = 0 ; k < NUM_PPAC ; ++k){
                        for (int l = 0 ; l < event.n_ppac[k] ; ++l){
                            ppac_word = event.w_ppac[k][l];

                            tdiff_ppac_labr = CalcTimediff(ppac_word, event.w_labr[i][j]);
                            time_ppac_labr[k]->Fill(tdiff_ppac_labr, i);
                            time_energy_ppac_labr[k]->Fill(tdiff_ppac_labr, labr_energy);

                            tdiff_ppac_de = CalcTimediff(ppac_word, de_word);
                            time_ppac_de[k]->Fill(tdiff_ppac_de, GetDetector(de_word.address).detectorNum);
                            time_energy_ppac_de[k]->Fill(tdiff_ppac_de, de_energy);

                            if (tdiff_ppac_labr > labr_time_cut.lower_prompt && tdiff_ppac_labr < labr_time_cut.higher_prompt){
                                alfna_ppac->Fill(labr_energy, ex);
                            }
                            else if (tdiff_ppac_labr > labr_time_cut.lower_bg && tdiff_ppac_labr < labr_time_cut.higher_bg){
                                alfna_ppac->Fill(labr_energy, ex, -1);
                                alfna_bg_ppac->Fill(labr_energy, ex, 1);
                            }

                        }
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
