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

#include "TelescopeGate.h"

#include <sstream>

//! Remove witespace at beginning and end.
/*! \return The text without leading and trailing whitespace.
 */
static std::string trim_whitespace( const std::string& text /*!< Text to be clean. */)
{
    size_t start = text.find_first_not_of(" \t");
    if ( start == std::string::npos )
        return "";

    size_t end = text.find_last_not_of(" \t");
    if ( end == std::string::npos || end<=start )
        end = text.size();
    return text.substr(start, end-start+1);
}

TelescopeGate::TelescopeGate(const std::string &cmd)
{
    std::stringstream icmd(cmd.c_str());

    std::string str;
    int ncoef;
    double *coef;
    icmd >> str;
    if (str == "uppper" || str == "UPPER"){
        icmd >> ncoef;
        coef = new double[ncoef];
        for (int i = 0 ; i < ncoef ; ++i)
            icmd >> coef[i];
        polH = Polynomial(coef, ncoef);
    }
}



void TelescopeGate::ExtractPolynomial(std::stringstream &icmd)
{
    std::string str;
    int ncoef;
    double *coef;
    icmd >> str;
    icmd >> ncoef;
    coef = new double[ncoef];
    for (int i = 0 ; i < ncoef ; ++i)
        icmd >> coef[i];
    str = trim_whitespace(str);
    if (str == "UPPER" || str == "upper")
        polH = Polynomial(coef, ncoef);
    else if (str == "LOWER" || str == "lower")
        polL = Polynomial(coef, ncoef);
    delete[] coef;
}
