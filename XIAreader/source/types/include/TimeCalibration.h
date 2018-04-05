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

#ifndef TIMECALIBRATION_H
#define TIMECALIBRATION_H

#include <map>
#include <string>
#include <cstdint>
#include <iosfwd>
#include <cmath>

#include "Parameters.h"

#define MAX_T_PARAM 3

class TimeCalibration 
{
public:
	//! Constructor
    TimeCalibration(void)
	{
		for (int i = 0 ; i < MAX_T_PARAM ; ++i) coef[i] = 0;
	}

	//! Constructor
    TimeCalibration(double cf[], int params);

	//! Destructor
	~TimeCalibration() { }

	//! Calculate the correct time.
	/*! \return The calibrated time.
	 */
    double Eval(const double t,	/*!< Current time (uncalibrated)	*/
                const double x	/*!< Energy in [keV]				*/) const
	{
		double res = t;
        double corr = coef[0]/std::pow(x, coef[1]);
        res -= corr;
		res -= coef[2];
		return res;
	}

	//! Inline operator() for easy access to Eval().
	inline double operator()(const double t, const double x) const { return Eval(t, x); }

private:
	//! Function to keep the coefficients.
	double coef[MAX_T_PARAM];
};

class TimeCalibrations
{
public:
	//! Constructor.
    TimeCalibrations() : isSet( false ) { }

	//! Set mapping from file.
    TimeCalibrations(const std::string &filename) { isSet = setAll(filename); }

	//! Find parameter for a detector from adc address.
    /*! \return the parameter for detector with the given adc address.
     */
    TimeCalibration Find(const uint16_t &address);

    //! Add a parameter to the mapping.
    void Add(const uint16_t &address, TimeCalibration param);

    //! Remove a parameter entry.
    void Remove(const uint16_t &address);

    //! Set one or more parameter from file.
    bool setAll(const std::string &filename);

private:
    //! Helper function
    bool next_line(std::istream &in, std::string &cmd_line);

	//! Mapping type used by the class.
	typedef std::map<uint16_t, TimeCalibration> calib_t;

	//! Variable storing the entries.
	calib_t m_calib;

    //! If the class has been initilized propperly or not.
    bool isSet;
};

#endif // TIMECALIBRATION_H
