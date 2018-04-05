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

#include "Polynomial.h"

#include <sstream>
#include <cmath>

Polynomial::Polynomial(){ }

Polynomial::Polynomial(double *coef, const int &n)
    : m_coef( new double[n] )
    , length( n )
{
    for (int i = 0 ; i < length ; ++i)
        m_coef[i] = coef[i];
}

Polynomial::Polynomial(const std::string &cmd)
{
	std::stringstream icmd(cmd.c_str());

	int n;
	icmd >> n;
    length = n;
    m_coef.reset(new double[n]);
	double coef;
    for (int i = 0 ; i < length ; ++i){
		icmd >> coef;
		m_coef[i] = coef;
	}
}

Polynomial::Polynomial(const Polynomial &pol)
    : m_coef( new double[pol.length] )
    , length( pol.length )
{
    for (int i = 0 ; i < length ; ++i)
        m_coef[i] = pol.m_coef[i];
}

Polynomial::~Polynomial(){ }

double Polynomial::Eval(const double &x) const
{
	double result = 0;
    for (int i = 0 ; i < length ; ++i)
		result += m_coef[i]*pow(x, i);
	return result;
}

Polynomial &Polynomial::operator=(const Polynomial &pol)
{
    this->m_coef.reset(new double[pol.length]);
    this->length = pol.length;
    for (int i = 0 ; i < this->length ; ++i){
        this->m_coef[i] = pol.m_coef[i];
    }
    return *this;
}

Polynomial &Polynomial::operator+=(const Polynomial &pol)
{
    if (this->length > pol.length){
        double *coef = new double[this->length];
        for (int i = 0 ; i < pol.length ; ++i){
            coef[i] = this->m_coef[i] + pol.m_coef[i];
        }
        for (int i = pol.length ; i < this->length ; ++i){
            coef[i] = this->m_coef[i];
        }
        this->m_coef.reset(coef);
    } else if (this->length < pol.length){
        double *coef = new double[pol.length];
        for (int i = 0 ; i < this->length ; ++i){
            coef[i] = this->m_coef[i] + pol.m_coef[i];
        }
        for (int i = this->length ; i < pol.length ; ++i){
            coef[i] = pol.m_coef[i];
        }
        this->m_coef.reset(coef);
        this->length = pol.length;
    } else if (this->length == pol.length){
        for (int i = 0 ; i < this->length ; ++i){
            this->m_coef[i] += pol.m_coef[i];
        }
    }

    return *this;
}

Polynomial &Polynomial::operator-=(const Polynomial &pol)
{
    if (this->length > pol.length){
        double *coef = new double[this->length];
        for (int i = 0 ; i < pol.length ; ++i){
            coef[i] = this->m_coef[i] - pol.m_coef[i];
        }
        for (int i = pol.length ; i < this->length ; ++i){
            coef[i] = this->m_coef[i];
        }
        this->m_coef.reset(coef);
    } else if (this->length < pol.length){
        double *coef = new double[pol.length];
        for (int i = 0 ; i < this->length ; ++i){
            coef[i] = this->m_coef[i] - pol.m_coef[i];
        }
        for (int i = this->length ; i < pol.length ; ++i){
            coef[i] = -pol.m_coef[i];
        }
        this->m_coef.reset(coef);
        this->length = pol.length;
    } else if (this->length == pol.length){
        for (int i = 0 ; i < this->length ; ++i){
            this->m_coef[i] -= pol.m_coef[i];
        }
    }

    return *this;
}
