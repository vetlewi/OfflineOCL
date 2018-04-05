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

#ifndef TIMEGATE_H
#define TIMEGATE_H

#include <string>
#include <map>

//! Class: TimeGate
/*!
 *  This class stores a list of ADC addresses and the interval that corresponds to prompt peaks.
 *  The class reads the ADC and the interval from a file.
 */
class TimeGate
{
public:
	//! Constructor.
	TimeGate(const std::string &filename	/*!< File to read gates from.	*/);


	//! Function to evaluate the gate.
	/*! \return true if time is within the gate, false otherwise.
	 */
	bool Evaluate(const uint16_t &address,	/*!< ADC address of detector.	*/
                  const double &time_value	/*!< Time difference.			*/) const;

private:
	struct Gate_t {
        double max;	//! Maximum value of the time difference.
        double min;	//! Minimum value of the time difference.

        //! Constructor.
        Gate_t(double m=0, /*!< Max value. */
               double n=0  /*!< Min value. */)
			: max( m )
			, min( n ){ }
	};

	//! Type to map the gates to ADC address.
	typedef std::map<uint16_t, Gate_t> GateMap_t;

	//! Variable storing the gates.
	GateMap_t gateMap;

	//! Function to find the gate.
	/*! \return The gate if exists. If not
	 *  it returns a gate with max=min=0.
	 */
    Gate_t Find(const uint16_t &address /*!< Address of detector.   */) const;
};



#endif // TIMEGATE_H
