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

/*!
 * \file DetectorRead.cpp
 * \brief Implementation of Dtype, Dinfo, Detectors.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \copyright GNU Public Licence v. 3
 */

#include "DetectorRead.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <ostream>


#define TEST_DETECTORREAD 0 // Set to 1 for testing.

std::ostream& operator<< (std::ostream& _os, const Dinfo _e)
{
    _os << "Name: " << _e.getName() << " Type: " << _e.getType()
        << " Theta: " << _e.getTheta() << " Phi: " << _e.getPhi()
        << " Telescope: " << _e.getTelescope();
    return _os;
}


Dtype::Dtype(void) :
	m_enum(none),
	m_string("none"),
	m_value(0) {}

// #######################################################

Dtype::Dtype(Enum _e) :
	m_enum(_e),
	m_string(toString(_e)),
	m_value(toValue(_e)) {}

// #######################################################

Dtype::Dtype(const std::string& _s) :
	m_enum(fromString(_s)),
	m_string(_s),
	m_value(toValue(m_enum)) {}

// #######################################################

Dtype& Dtype::operator= (const Dtype& _d)
{
	m_string = _d.m_string;
	m_enum = _d.m_enum;
	m_value = _d.m_value;
	return *this;
}

// #######################################################

Dtype& Dtype::operator= (const std::string& _s)
{
	m_string = _s;
	m_enum = fromString(_s);
	m_value = toValue(m_enum);
	return *this;
}

// #######################################################

Dtype& Dtype::operator= (Enum _e)
{
	m_enum = _e;
	m_string = toString(_e);
	m_value = toValue(_e);
	return *this;
}

// #######################################################

bool Dtype::operator< (const Dtype& _d) const
{
	return (m_value < _d.m_value);
}

// #######################################################

bool Dtype::operator< (Enum _e) const 
{
	return (m_value < toValue(_e));
}

// #######################################################

bool Dtype::operator<= (const Dtype& _d) const
{
	return (m_value <= _d.m_value);
}

// #######################################################

bool Dtype::operator<= (Enum _e) const
{
	return (m_value <= toValue(_e));
}

// #######################################################

bool Dtype::operator> (const Dtype& _d) const
{
	return (m_value > _d.m_value);
}

// #######################################################

bool Dtype::operator> (Enum _e) const
{
	return (m_value > toValue(_e));
}

// #######################################################

bool Dtype::operator>= (const Dtype& _d) const
{
	return (m_value >= _d.m_value);
}

// #######################################################

bool Dtype::operator>= (Enum _e) const
{
	return (m_value >= toValue(_e));
}

// #######################################################

bool Dtype::operator== (const Dtype& _d) const
{
	return (m_value == _d.m_value);
}

// #######################################################

bool Dtype::operator== (const std::string& _s) const
{
	// Case insensitive - make all upper case
	std::string comp = make_uppercase(_s);
	return (m_string == comp);
}

// #######################################################

bool Dtype::operator== (const Enum _e) const
{
	return (m_enum == _e);
}

// #######################################################

bool Dtype::operator!= (const Dtype& _d) const
{
	return (m_value != _d.m_value);
}

// #######################################################

bool Dtype::operator!= (const std::string& _s) const
{
	// Case insensitive - make all upper case
	std::string comp = make_uppercase(_s);
	return (m_string != comp);
}

// #######################################################

bool Dtype::operator!= (const Enum _e) const
{
	return (m_enum != _e);
}

// #######################################################

Dtype::Enum Dtype::fromString(const std::string _s)
{
	// Case insensitive - make all upper case
	std::string comp = make_uppercase(_s);
	if (comp == "NONE")			return none;
	else if (comp == "GAMMA")	return gamma;
	else if (comp == "SILICON")	return silicon;
	else if (comp == "CFD")		return CFD;
	else if (comp == "UNKNOWN")	return unknown;

	//throw std::range_error("Not a valid detector type value: " + _s);
	return unknown;
}

// #######################################################

std::string Dtype::toString(Dtype::Enum _e)
{
	switch (_e)
	{
    case none:		{ return "NONE";	}
    case gamma:		{ return "GAMMA";	}
    case silicon:	{ return "SILICON";	}
    case CFD:		{ return "CFD";		}
    case unknown:   { return "UNKNOWN"; }
    }
	return "UNKNOWN";
}

// #######################################################

int Dtype::toValue(Dtype::Enum _e)
{
	switch (_e)
	{
    case none:		{ return 0;	}
    case gamma:		{ return 1; }
    case silicon:	{ return 2;	}
    case CFD:		{ return 3; }
    case unknown:   { return 4; }

	}
	return 4; // unknown
}

// #######################################################

std::string make_uppercase(const std::string& _s)
{
	std::string newString(_s);
	for (unsigned int l = 0 ; l < _s.length() ; l++)
	{
		newString[l] = std::toupper(_s[l]);
	}
	return newString;
}

// #######################################################

Dinfo::Dinfo(void)
	: m_name("")
	, m_type(Dtype())
    , m_theta( 0 )
    , m_phi( 0 )
	, m_telescope("") {}

// #######################################################

Dinfo::Dinfo(std::string _n, Dtype _t, std::string _d, double _h, double _p)
{
	setName(_n);
	setType(_t);
    setTheta(_h);
    setPhi(_p);
    setTelescope(_d);
}

// #######################################################

Dinfo::Dinfo(const Dinfo& _d)
    : m_name(_d.m_name)
    , m_type(_d.m_type)
    , m_theta(_d.m_theta)
    , m_phi(_d.m_phi)
    , m_telescope(_d.m_telescope) {}

// #######################################################

Dinfo& Dinfo::operator=(const Dinfo& _d)
{
	this->m_name = _d.m_name;
	this->m_type = _d.m_type;
    this->m_theta = _d.m_theta;
    this->m_phi = _d.m_phi;
	this->m_telescope = _d.m_telescope;
	return *this;
}

// #######################################################

int Dinfo::operator==(const Dinfo& _d) const
{
    if ((this->m_name == _d.m_name)&&(this->m_type == _d.m_type)&&
        (this->m_theta == _d.m_theta)&&(this->m_phi == _d.m_phi)&&
            (this->m_telescope == _d.m_telescope)) return 1;
	return 0;
}

// #######################################################

int Dinfo::operator!=(const Dinfo& _d) const
{
    if ((this->m_name != _d.m_name)||(this->m_type != _d.m_type)||
        (this->m_theta != _d.m_theta)||(this->m_phi != _d.m_phi)||
            (this->m_telescope != _d.m_telescope)) return 1;
	return 0;
}

// #######################################################

int Dinfo::operator<(const Dinfo& _d) const
{
    if ( this->m_name == _d.m_name && this->m_type < _d.m_type &&
         this->m_theta < _d.m_theta && this->m_phi < _d.m_phi &&
         this->m_telescope == _d.m_telescope ) return 1;
	return 0;
}

// #######################################################

void Dinfo::setName(std::string _n)
{

	m_name = _n;
}

// #######################################################

void Dinfo::setType(Dtype _t)
{
	m_type = _t;
}

// #######################################################

void Dinfo::setType(std::string _s)
{
	m_type = _s;
}

// #######################################################

void Dinfo::setTheta(double theta)
{
    m_theta = theta;
}

// #######################################################

void Dinfo::setPhi(double phi)
{
    m_phi = phi;
}

// #######################################################

void Dinfo::setTelescope(std::string _d)
{
	m_telescope = _d;
}

// #######################################################

Detectors::Detectors(void) {}

// #######################################################

Detectors::Detectors(std::string filename)
{
	setAll(filename);
}

// #######################################################

Detectors::Detectors(const Detectors& _d) :
	m_addresses( _d.m_addresses ) { }

// #######################################################

Detectors &Detectors::operator=(const Detectors& _d)
{
	this->m_addresses = _d.m_addresses;
	return *this;
}

// #######################################################

Dinfo Detectors::Find(const uint16_t& address)
{
	addresses_t::iterator it = m_addresses.find(address);
	if (it != m_addresses.end() ) return it->second;
	else return Dinfo(std::string("none"), Dtype("none"));
}

// #######################################################

void Detectors::Add(const uint16_t& address, Dinfo _d)
{
	addresses_t::iterator it = m_addresses.find(address);
	if ((it != m_addresses.end())&&(it->second != _d))
	{
		std::cout << "Detector already set." << std::endl;
		std::cout << "Old detector element: " << Find(address) << std::endl;
		std::cout << "New detector element: " << _d << std::endl;
	}
	m_addresses[address] = _d;
}

// #######################################################

void Detectors::Remove(const uint16_t& address)
{
	addresses_t::iterator it = m_addresses.find(address);
	if (it != m_addresses.end()) m_addresses.erase( it );
	else {
		std::cout << "Warning: ADC address '" << address
		<< "' have not been set." << std::endl;
	}
}

// #######################################################

bool Detectors::setAll(std::string filename)
{
	std::ifstream input_file(filename.c_str());

	std::string line;

	bool status = true;

	while ( next_line(input_file, line) ){
		if (line.size()==0 || line[0]=='#') continue;

		std::istringstream icmd(line.c_str());
        std::string name, type, telescope;
		uint16_t addr;
        double theta, phi;
		icmd >> addr;
		icmd >> name;
        icmd >> theta;
        icmd >> phi;
        icmd >> type;
        icmd >> telescope;
		if (name.empty()) status = false;
		if (type.empty()) status = false;

        Add(addr, Dinfo(name, Dtype(type), telescope, theta, phi));
	}

	input_file.close();
	return status;
}

// #######################################################

bool Detectors::next_line(std::istream& in, std::string& cmd_line)
{
	cmd_line = "";
	std::string line;
	while ( getline(in, line) ){
		size_t ls = line.size();
		if ( ls == 0 ){
			break;
		} else if ( line[ls-1] != '\\' ) {
			cmd_line += line;
			break;
		} else {
			cmd_line += line.substr(0, ls-1);
		}
	}
	return in || !cmd_line.empty();
}

// #######################################################
// #######################################################

#if TEST_DETECTORREAD

int main(int argc, char* argv[])
{
	if (argc != 2){
		std::cerr << "Usage: '" << argv[0] << " <filename>'" << std::endl;
	}
	Detectors test(argv[1]);
	std::cout << test;
	Detectors newOne = test;

	test.Remove(65);
	test.Remove(69);

	std::cout << test;
	std::cout << newOne;

	return 0;

}

#endif
