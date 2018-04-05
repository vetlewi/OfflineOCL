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


#ifndef TELESCOPE_H
#define TELESCOPE_H

#include <string>
#include <iosfwd>
#include <cstdint>
#include <map>
#include <cmath>

#include "Polynomial.h"

/*!
 *  \class     Telescope
 *  \brief     Class to calculate kinematics.
 *  \details   This class handles all kinematics calculations needed by the sorting routine. It is initilized with a
 *             mapping of the ADC address to the telescope pixels( (n, m) coordinates). It relies on user defined polynomials
 *             for calculating the energies and doppler corrections.
 *  \author    Vetle W. Ingeberg
 *  \version   0.8.0
 *  \date      2015-2016
 *  \copyright GNU Public License v. 3
 *  \todo      The class needs to be somewhat rewritten to be more general. As of it current state it quite spesific and needs to be
 *             more general to be used with other experiments than what the software is currently been written for.
 */
class Telescope
{
public:
	//! Constructor.
    Telescope(std::string vertFile, /*!< File with the mapping of the vertical strips.  */
              std::string horiFile /*!< File with the mapping of the horizontal strips. */);

	//! Destructor.
	~Telescope();
	
    //! Calculate excitation energy.
    /*! The excitation energy is calculated using a polynomial provided by the user, \f$E_x = a_0(n,m) + (E + dE)a_1(n,m) + (E + dE)^2a_2(n,m)\f$.
     *  \return The excitation energy.
     */
    double CalcEx(const uint16_t &xaddress,	/*!< ADC address to vertical strip.			*/
                  const uint16_t &yaddress,	/*!< ADC address to horizontal strip.		*/
                  const double &EdE			/*!< Total energy in dE and E detector.     */) const;

    //! Calculate Doppler correction.
    /*! Calculates the doppler correction using \f$E_\gamma^\prime = \gamma E_\gamma (1 - \beta\sin\theta\sin\phi)\f$.
     *  Where \f$E_\gamma^\prime\f$ is the CM gamma energy, \f$E_\gamma\f$ is the lab gamma energy, \f$\theta\f$ and \f$\phi\f$ is the
     *  spherical angles of the detector with respect to the beam axis and \f$\gamma\f$ and \f$\beta\f$ is the Lorentz factors determined by
     *  the momentum of the residual nucleus. Currently these are set manually by the user using the macro constants GAMMAD, BETAD, GAMMAP, BETAP in the implementation.
     *  These values has to be choose depending on the kinematics and the reaction. Currently only deutrons and protons are implemented by these constants.
     *  \return The doppler corrected gamma energy.
     */
    double CalcDoppler(const double &theta,			/*!< Theta angle of the gamma detector.	*/
					   const double &Egamma			/*!< Energy observed in the detector.	*/) const;

    //! Calculate Angle
    /*! The angles of each pixel with respect to the beam axis is given by:
     *  \f$\theta_{n,m} = \cos^{-1}(\frac{r + (n + 17/2)d}{\sqrt{2r^2 + 2(n^2 + m^2 - 17(n + m))d^2 + 289d^2}})\f$
     *  where \f$r\f$ is the radius of the target chamber, \f$d\f$ is the width of the strips and \f$n\f$ and \f$m\f$ is the
     *  strip number.
     *  \return The angle of the pixel.
     */
    double CalcAngle(const uint16_t &xaddress,  /*!< Address to vertical strip.     */
                     const uint16_t &yaddress   /*!< Address to horizontal strip.   */) const;

    //! Find mapping of vertical strips.
    /*! \return the n-value of strip with given adc address.
     *  If not found, will return -1.
     */
    int findVert(const uint16_t &address	/*!< ADC address of the strip.	*/) const;

    //! Find mapping of horizontal strips.
    /*! \return the m-value of strips with given adc address.
     *  If not found, will return -1.
     */
    int findHori(const uint16_t &address	/*!< ADC address of the strip.	*/) const;


    //! Set the excitation curves.
    /*! \return True if reading of the file was successful, false otherwise.
     */
    inline bool SetExcitation(const std::string filename /*! File containing the excitation curves.  */){ return extractExPoly(filename); }

    //! Set the beta factor (velocity after reaction).
    inline void SetBeta(const double b){ beta = b; gamma = 1/sqrt(1 - beta*beta); }

private:
	//! Type for mapping of detector.
	typedef std::map<uint16_t,int> AddressMap;

	//! Mapping of vertical strips.
	AddressMap vertStrips;

	//! Mapping of horizontal strips.
	AddressMap horiStrips;

	//! Array of 2nd degree polynomials for Ex calculation.
    Polynomial ExPoly[16][16];

    //! Variable to store beta factor.
    double beta;

    //! Variable to store gamma factor.
    double gamma;

	//! Function to extract the mapping of the vertical strip detectors.
    /*! This function is invoked by the line: "vertical <path-to-file>" in input file.
     *  Each line in the input file represents the vertical n-value of a given adc address.
     *  Each line is then:
     *  <vertical n> <adc address>
     *  Lines can be commented by starting with #.
     */
	void extractVertical(std::string path	/*!< Path to the file to extract mapping from.	*/);

	//! Function to extract the mapping of the horizontal strip detectors.
    /*! This function is invoked by the line: "horizontal <path-to-file>" in input file.
     *  Each line in the input file represents the horizontal m-value of a given adc address.
     *  Each line is then:
     *  <horizontal m> <adc address>
     *  Lines can be commented by starting with #.
     */
	void extractHorizontal(std::string path	/*!< Path to the file to extract mapping from.	*/); 


    //! Function to extract Ex polynomials.
    /*! Format of the input file is assumed to be:
     *  <vertical n> <horizontal m> <a0> <a1> <a2>
     *  Lines that starts with # are comments. It is assumed that a0 has units [MeV], a1 is in [MeV\f$^{-1}\f$] and a2 is in [MeV\f$^{-2}\f$].
     */
    bool extractExPoly(std::string filename	/*!< File to extract from.	*/);


    //! Read the input file containing the parameters needed to initilize the class.
    void readFile(std::string filename  /*!< The batch file to read.    */);

	//! Read a single line.
    /*! \return False if EOF, True elsewise.
     */
    bool next_line(std::istream &in,        /*!< File stream to read a new line from.       */
                   std::string &cmd_line    /*!< String to set with the line that are read. */);

};

#endif // TELESCOPE_H
