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

#include "TimeGate.h"

#include <sstream>
#include <fstream>
#include <iostream>

static bool next_line(std::istream &in, std::string &cmd_line)
{
	cmd_line = "";
	std::string line;
	while ( getline(in, line) ){
		unsigned long ls = line.size();
		if ( ls == 0 )
			break;
		else if ( line[ls - 1] != '\\' ){
			cmd_line += line;
			break;
		} else {
			cmd_line += line.substr(0, ls-1);
		}
	}
	return in || !cmd_line.empty();
}

TimeGate::TimeGate(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	std::string line;
	uint16_t address;
    double max, min;

    if (!file.is_open()){
        std::cerr << "TimeGate: Could not open file '" << filename << "'." << std::endl;
    }

	while ( next_line(file, line) ){
		if ( line.size() == 0 || line[0] == '#' )
			continue;
		std::stringstream icmd(line.c_str());
		icmd >> address;
		icmd >> min;
		icmd >> max;
		gateMap[address] = Gate_t(max, min);
	}
	file.close();
}

TimeGate::Gate_t TimeGate::Find(const uint16_t &address) const
{
	GateMap_t::const_iterator it = gateMap.find(address);
	if ( it != gateMap.end() )
		return it->second;
	else
		return Gate_t(0,0);
}

bool TimeGate::Evaluate(const uint16_t &address, const double &time_value) const
{
    Gate_t gate = Find(address);
    bool result;
    if ( time_value >= gate.min && time_value <= gate.max )
        result = true;
    else
        result = false;
    return result;
}
