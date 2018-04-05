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

#include "Telescope.h"

#include "DefineFile.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>


/*!
 * \file Telescope.cpp
 * \brief Implementation of the Telescope class.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */

//! The constant \f$\pi\f$.
//static const double PI = acos(-1);


Telescope::Telescope(std::string vertStrips, std::string horiStrips)
{
    extractVertical(vertStrips);
    extractHorizontal(horiStrips);
}

Telescope::~Telescope()
{
}

double Telescope::CalcEx(const uint16_t &xaddress, const uint16_t &yaddress, const double &EdE) const
{
	int n = findVert(xaddress);
    int m = findHori(yaddress);
	if (n < 0 || m < 0){
		return 0;
	}
    double result = ExPoly[n-1][m-1](EdE);
	return result;
}

double Telescope::CalcDoppler(const double &theta, const double &Egamma) const
{
    return Egamma*gamma*(1 - beta*cos(theta));
}

double Telescope::CalcAngle(const uint16_t &xaddress, const uint16_t &yaddress) const
{
    int n = findVert(xaddress);
    int m = findHori(yaddress);

    if (n < 0 || m < 0)
        return 0;

    double xaxis = 6.0 - (n - 17./2.)*0.3;
    double yaxis = 6.0 + (n - 17./2.)*0.3;
    double zaxis = (m - 17./2.)*0.3;
    return acos(yaxis/sqrt(xaxis*xaxis + yaxis*yaxis + zaxis*zaxis))*180./acos(-1);
}


int Telescope::findVert(const uint16_t &address) const
{
	AddressMap::const_iterator it = vertStrips.find(address);
	if ( it != vertStrips.end() )
		return it->second;
	else
		return -1;
}

int Telescope::findHori(const uint16_t &address) const
{
	AddressMap::const_iterator it = horiStrips.find(address);
	if ( it != horiStrips.end() )
		return it->second;
	else
		return -1;
}

void Telescope::extractVertical(std::string filename)
{
	std::ifstream file(filename.c_str());
	std::string line;
	int i;
	uint16_t n;
	while( next_line(file, line) ){
		if ( line.size() == 0 || line[0] == '#' )
			continue;
		std::stringstream icmd(line.c_str());
		icmd >> i;
		icmd >> n;
		vertStrips[n] = i;
    }
	file.close();
}

void Telescope::extractHorizontal(std::string filename)
{
	std::ifstream file(filename.c_str());
	std::string line;
	int j;
	uint16_t m;
	while( next_line(file, line) ){
		if ( line.size() == 0 || line[0] == '#' )
			continue;
		std::stringstream icmd(line.c_str());
		icmd >> j;
		icmd >> m;
		horiStrips[m] = j;
	}
	file.close();
}

bool Telescope::extractExPoly(std::string filename)
{
	std::ifstream file(filename.c_str());
	std::string line;
	int i, j;
	double a0, a1, a2;
    if (!file.is_open())
        return false;
	while ( next_line(file, line) ){
		if ( line.size() == 0 || line[0] == '#' )
			continue;
		std::stringstream icmd(line.c_str());
		icmd >> i;
		icmd >> j;
		icmd >> a0;
		icmd >> a1;
		icmd >> a2;
        double *poly = new double[3];
        poly[0] = a0; poly[1] = a1; poly[2] = a2;
        ExPoly[i-1][j-1] = Polynomial(poly, 3); // Test to see if this gives a more appropriate Ex - Egamma (no gammas above the Ex). Currently a0 + 0.5 gives good result. If needed try changing it up and down depending on what fits the data best.
        delete[] poly;
	}
	file.close();
    return true;
}

bool Telescope::next_line(std::istream &in, std::string &cmd_line)
{
	cmd_line = "";
	std::string line;
	while ( getline(in, line) ){
		size_t ls = line.size();
		if ( ls == 0 ){
			break;
		} else if ( line[ls - 1] != '\\' ){
			cmd_line += line;
			break;
		} else {
			cmd_line += line.substr(0, ls-1);
		}
	}
	return in || !cmd_line.empty();
}
