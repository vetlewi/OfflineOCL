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

#ifndef GAINPARAMETERS_H
#define GAINPARAMETERS_H

#include "Parameters.h"

#include <map>
#include <string>
#include <cstdint>
#include <iosfwd>

//! Class to contain all data needed for energy calibration
/*! The class are able to contain up to 4th degree polynomials.
 */
#define MAX_E_PARAM 5
class EnergyParameter
{
public:
    //! Constructor
    EnergyParameter(void)
    {
        for (int i = 0 ; i < MAX_E_PARAM ; i++) coef[i] = 0.0;
        coef[1] = 1.0;
    }

    //! Constructor
    EnergyParameter(double cf[], int params)
        : EnergyParameter()
    {
        if (params <= MAX_E_PARAM)
            for (int i = 0 ; i < params ; i++) coef[i] = cf[i];
    }

    //! Copy constructor.
    EnergyParameter(const EnergyParameter &ep)
        : EnergyParameter()
    {
        for (int i = 0 ; i < MAX_E_PARAM ; i++) coef[i] = ep.coef[i];
    }

    //! Destructor
    ~EnergyParameter(){ }

    //! Perform evaluation of the polynomial.
    inline double Eval(const double x) const
    {
        double xpot = 1.0, result=0.0;
        for (int i = 0 ; i < MAX_E_PARAM ; i++){
            result += xpot*coef[i];
            xpot *= x;
        }
        return result;
    }

    //! Inline function for easy access to Eval().
    inline double operator()(const double x) const { return Eval(x); }
private:
    //! Function to keep the coefficients.
    double coef[MAX_E_PARAM];

};

class TimeParameter
{
public:
    //! Constructor.
    TimeParameter(){ }

    //! Destructor.
    //~TimeParameter(){ }

private:

};

class GainParameter
{
public:
    //! Constructor
    GainParameter(){ }

    //! Constructor
    GainParameter(EnergyParameter _epar, TimeParameter _tpar)
        : epar( _epar )
        , tpar( _tpar ){ }

    //! Copy constructor
    GainParameter(const GainParameter &par)
        : epar( par.epar )
        , tpar( par.tpar ){ }

    //! Evaluate energy.
    inline double Eeval(const double x) const { return epar(x); }

    //! Evaluate time.
    inline double Teval() const { return 0; }
private:
    //! Variable to keep energy parameters.
    EnergyParameter epar;

    //! Variable to keep time parameters.
    TimeParameter tpar;
};

class GainParameters
{
public:
    //! Constructor
    GainParameters() { }

    //! Set mapping from file.
    GainParameters(const std::string &filename) { setAll(filename); }

    //! Find parameter for a detector from adc address.
    /*! \return the parameter for detector with the given adc address.
     */
    GainParameter Find(const uint16_t &address);

    //! Add a parameter to the mapping.
    void Add(const uint16_t &address, GainParameter param);

    //! Remove a parameter entry.
    void Remove(const uint16_t &address);

    //! Set one or more parameter from file.
    bool setAll(const std::string &filename);

private:
    //! Helper function for the reading from file.
    bool next_line(std::istream &in, std::string &cmd_line);

    //! Mapping type used by the class.
    typedef std::map<uint16_t, GainParameter> params_t;

    //! Variable to store the mapping.
    params_t m_params;
};

#endif // GAINPARAMETERS_H
