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

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H


#include <string>
#include <memory>

//! A general polynomial.
class Polynomial
{
public:
	//! Empty constructor.
	Polynomial();

	//! Constructor, from pointer of coefficients.
	Polynomial(double *coef,	/*!< Pointer containing the coefficients.						*/
			   const int &n		/*!< Number of coefficients (i.e. degree of the polynomial.)	*/);

	//! Constructor, from vector.
    //Polynomial(const Vector &vec	/*!< Coeficients contained in a vector.	*/);

	//! Constructor, from string.
	/*! Formating: <n> <a0> <a1> ... <ai> ... <an>.
	 *  n: Number of coefficients.
	 *  ai: ith coeficient.
	 */
	Polynomial(const std::string &cmd	/*!< String to create polynomial from.	*/);

	//! Copy constructor.
	Polynomial(const Polynomial &pol	/*!< Polynomial to copy.	*/);

	//! Destructor.
	~Polynomial();

	//! Function to evaluate the polynomial at a given point.
	/*! \return the value of the polynomial with argument x.
	 */
	double Eval(const double &x	/*!< Argument of the polynomial.	*/) const;

	//! Overloaded ().
	/*! \return \ref Eval.
	 */
    double operator()(const double &x 	/*!< Argument of the polynomial.	*/) const { return Eval(x);     }

	//! Assign operator.
	Polynomial &operator=(const Polynomial &pol		/*!< Polynomial to assign.		*/);

	//! Overloaded +=
	Polynomial &operator+=(const Polynomial &pol 	/*!< Polynomial to add.			*/);

	//! Overloaded -=
	Polynomial &operator-=(const Polynomial &pol 	/*!< Polynomial to subtract.	*/);

private:
	//! Vector to contain the coefficients.
    std::unique_ptr<double[]> m_coef;

    //! Length of the vector.
    int length;

};


#endif // POLYNOMIAL_H
