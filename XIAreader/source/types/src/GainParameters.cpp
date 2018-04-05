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

#include "GainParameters.h"

#include <fstream>
#include <sstream>

#define TEST 0

GainParameter GainParameters::Find(const uint16_t &address){
    params_t::iterator it = m_params.find(address);
    if ( it != m_params.end() )
        return it->second;
    else
        return GainParameter();
}

void GainParameters::Add(const uint16_t &address, GainParameter param)
{
    m_params[address] = param;
}

void GainParameters::Remove(const uint16_t &address)
{
    params_t::iterator it = m_params.find(address);
    if (it != m_params.end())
        m_params.erase( it );
}

bool GainParameters::setAll(const std::string &filename)
{
    std::ifstream input_file(filename.c_str());

    std::string line;

    while ( next_line(input_file, line) ){
        if (line.size() == 0 || line[0] == '#') continue;
        std::istringstream icmd(line.c_str());
        uint16_t addr;
        double energy[MAX_E_PARAM];
        for (int i = 0 ; i < MAX_E_PARAM ; ++i) energy[i] = 0.0;
        icmd >> addr;
        for (int i = 0 ; i < MAX_E_PARAM ; i++) icmd >> energy[i];
        Add(addr, GainParameter(EnergyParameter(energy,MAX_E_PARAM),TimeParameter()));
    }
    return true;
}

bool GainParameters::next_line(std::istream &in, std::string &cmd_line)
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

#if TEST
#include <iostream>

int main()
{
    GainParameters *params = new GainParameters("/Users/vetlewi/Dropbox/Master/Sorting/input_files/FineTune/gain_Si_pairs_Eu_w1.list");

    std::cout << params->Find(64).Eeval(131.0) << std::endl;
}
#endif // TEST
