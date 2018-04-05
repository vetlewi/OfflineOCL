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

#ifndef TELESCOPEGATE_H
#define TELESCOPEGATE_H

#include "Polynomial.h"

//! Class defining the gating done in the telescopes.
class TelescopeGate
{
public:
    //! Empty constructor.
    TelescopeGate(){ }

    //! Construct from polynomials.
    TelescopeGate(const Polynomial &_upper, /*!< Polynomial for the upper limit.    */
                  const Polynomial &_lower  /*!< Polynomial for the lower limit.    */) : polH( _upper ), polL( _lower ){ }

    //! Construct from string.
    TelescopeGate(const std::string &cmd    /*!< String to read.    */);

    //! Check if passes the gate.
    /*! \return true if pass, false otherwise.
     */
    bool Eval(const double &de, /*!< Delta E detector value.    */
              const double &e   /*!< E detector value.          */) const;

    //! Overload of () operator.
    /*! \return \ref Eval.
     */
    inline bool operator()(const double &de,    /*!< Delta E detector value.    */
                           const double &e      /*!< E detector value.          */) const { return Eval(de, e); }

private:
    //! Upper limit.
    Polynomial polH;

    //! Lower limit.
    Polynomial polL;

    //! Helper function to extract data.
    void ExtractPolynomial(std::stringstream &icmd);
};

inline bool TelescopeGate::Eval(const double &de, const double &e) const
{
    if (de < polH(e) && de > polL(e))
        return true;
    else
        return false;
}

#endif // TELESCOPEGATE_H
