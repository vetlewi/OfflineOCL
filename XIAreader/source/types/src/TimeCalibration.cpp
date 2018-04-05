/*******************************************************************************
 * Copyright (C) 2017 Vetle W. Ingeberg                                        *
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

#include "TimeCalibration.h"

#include <iostream>
#include <fstream>
#include <sstream>

double ZEROS[MAX_T_PARAM] = {0.0, 0.0, 0.0};

TimeCalibration::TimeCalibration(double cf[], int params)
{
	if (params > MAX_T_PARAM){
		std::cerr << "TimeCalibration: Number of coefficients are larger than the maximum number: ";
		std::cerr << MAX_T_PARAM << ". Keeping only the first " << MAX_T_PARAM;
		std::cerr << ". Please increase the limit if more coefficients are needed." << std::endl;
		for (int i = 0 ; i < MAX_T_PARAM ; ++i){
			coef[i] = cf[i];
		}
	} else {
		for (int i = 0 ; i < MAX_T_PARAM ; ++i){
			if (i < params)
				coef[i] = cf[i];
			else
				coef[i] = 0;
		}
	}
}

TimeCalibration TimeCalibrations::Find(const uint16_t &address)
{
    if (!isSet) return TimeCalibration(ZEROS, MAX_T_PARAM);
	calib_t::iterator it = m_calib.find(address);
	if ( it != m_calib.end() )
		return it->second;
	else
		return TimeCalibration();
}

void TimeCalibrations::Add(const uint16_t &address, TimeCalibration param)
{
	m_calib[address] = param;
}

void TimeCalibrations::Remove(const uint16_t &address)
{
	calib_t::iterator it = m_calib.find(address);
	if ( it != m_calib.end() )
		m_calib.erase( it );
}

bool TimeCalibrations::setAll(const std::string &filename)
{
	std::ifstream input_file(filename.c_str());

	std::string line;
	while ( next_line(input_file, line) ){
		if (line.size() == 0 || line[0] == '#') continue;
		std::istringstream icmd(line.c_str());
		uint16_t addr;
		double coef[MAX_T_PARAM];
		for (int i = 0 ; i < MAX_T_PARAM ; ++i) coef[i] = 0.0;
		icmd >> addr;
        for (int i = 0 ; i < MAX_T_PARAM ; ++i) icmd >> coef[i];
		Add(addr, TimeCalibration(coef, MAX_T_PARAM));
	}
    isSet = true;
    return true;
}

bool TimeCalibrations::next_line(std::istream &in, std::string &cmd_line)
{
    cmd_line = "";
    std::string line;
    while ( getline(in, line) ){
        size_t ls = line.size();
        if (ls == 0)
            break;
        else if ( line[ls-1] != '\\' ){
            cmd_line += line;
            break;
        } else
            cmd_line += line.substr(0, ls-1);
    }
    return in || !cmd_line.empty();
}
