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


#ifndef DETECTORREAD_H
#define DETECTORREAD_H

#include <string>
#include <map>
#include <vector>
#include <cmath>


//! Make a string uppercase only.
/*! \return the input string in upper case.
 */
std::string make_uppercase(const std::string& ss    /*!< String to convert to upper case.   */);

//! A enum class for detector type.
/*! This class is for defining what type of
 *  detector we are looking at.
 */

/*!
 * \class Dtype
 * \brief A wrapper class for the Dtype enum. This is a type describing a type of detector.
 * \details A class for the enum type Dtype. It incorperates some useful features as comparison and conversion to string and integer.
 * \author Vetle W. Ingeberg
 * \version 0.8.0.0
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class Dtype
{
public:
	enum Enum {
		none=0,		/*!< Not a real detector.					*/
		gamma,		/*!< Gamma detector. 						*/
		silicon,	/*!< Silicon detector. 						*/
		CFD,		/*!< Constant fraction discriminator data.	*/
		unknown 	/*!< Unknown detector.						*/
	};

	//! Constructors
	Dtype(void);

	//! Initialize with a enum value.
	Dtype(Enum ee /*!< Value to initiate with. */);

	//! Initiate with a string value.
	explicit Dtype(const std::string &ss);

	//! Overloaded assignment operators.
	Dtype& operator = (const Dtype& dd /*!< Value to assign. */);
	Dtype& operator = (const std::string& ss /*!< String value. */);
	Dtype& operator = (Enum ee /*!< enum value. */);

	//! Overloaded comparison operators.
	bool operator< (const Dtype& dd /*!< Value to compare. */) const;
	bool operator< (Enum ee /*!< Value to compare. */) const;
	bool operator<= (const Dtype& dd /*!< Value to compare. */) const;
	bool operator<= (Enum ee /*!< Value to compare. */) const;
	bool operator> (const Dtype& dd /*!< Value to compare. */) const;
	bool operator> (Enum ee /*!< Value to compare. */) const;
	bool operator>= (const Dtype& dd /*!< Value to compare. */) const;
	bool operator>= (Enum ee /*!< Value to compare. */) const;
	bool operator== (const Dtype& dd /*!< Value to compare. */) const;
	bool operator== (const std::string& ss /*!< Value to compare. */) const;
	bool operator== (const Enum ee /*!< Value to compare. */) const;
	bool operator!= (const Dtype& dd /*!< Value to compare. */) const;
	bool operator!= (const std::string& ss /*!< Value to compare. */) const;
	bool operator!= (const Enum ee /*!< Value to compare. */) const;

	//! Accessor functions
    /*! Get enum as string.
     *  \return enum as string.
     */
    inline std::string getString 	(void) const;

    /*! Get enum as enum type.
     *  \return the enum value.
     */
	inline Enum getEnum 			(void) const;

    /*! Get enum as integer.
     *  \return the integer value of the enum.
     */
	inline int getValue				(void) const;

private:
    // Static functions
    //! Create an variable from string.
    /*! \return Enum from string.
     */
    static Enum	fromString(const std::string ss /*!< Convert from string and return as enum.    */);

    //! Get as string.
    /*! \return the enum type name as string.
     */
    static std::string toString(Enum ee /*!< Element to be converted.   */);

    //! Get as integer value.
    /*! \return a integer representing the enum value.
     */
    static int toValue(Enum ee  /*!< Element to be converted.   */);

	// Data members

	//! Variable to contain enum value.
	Enum m_enum;
	//! Variable to contain value as a string.
	std::string m_string;

	//! Varable to contain value as a number.
	int m_value;
	

};

//! For nice string output.
inline std::ostream& operator<<(std::ostream& _os, const Dtype _e)
{
	_os << _e.getString();
	return _os;
}

inline std::string Dtype::getString(void) const
{
	return m_string;
}

Dtype::Enum Dtype::getEnum(void) const
{
	return m_enum;
}

int Dtype::getValue(void) const
{
	return m_value;
}


// #######################################################

/*!
 * \class Dinfo
 * \brief Dinfo is a class for containing useful information about a detector.
 * \details The class containts the name, ADC address, its telescope relation (if any, may be used for other purposes), it placement
 * in the detector array, etc.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \copyright GNU Public Licence v. 3
 * \todo This type may implement additional fields such as the calibration of the detectors, etc.
 */
class Dinfo {
public:

	//! Default constructor.
	Dinfo(void);

	//! Construct a detector with name and type.
	/*! If not able to construct from string, a
	 *  warning will be printed.
	 */
    Dinfo(std::string nn,		/*!< Name of the detector.                          */
          Dtype tt,             /*!< Type of the detector.                          */
          std::string tl="",  	/*!< Belongs to what telescope.                     */
          double th=0,          /*!< Angle to the z-axis [deg].                     */
          double ph=0          /*!< Rotation in the xy-plane on the y-axis [deg].  */);

	//! Copy constructor.
	/*! Needed to handle pass by value.
	 */
	Dinfo(const Dinfo &dd /*!< Value to copy. */);

	//! Destructor.
    ~Dinfo(){ }

	// Overloaded function to satisfy MAP container.

	//! Assign operator.
	Dinfo &operator=(const Dinfo &dd /*!< Value to assign. */);
	
	//! 'Equal to' operator.
	int operator==(const Dinfo &dd /*!< Value to compare. */) const;

	//! 'Not equal' operator
	int operator!=(const Dinfo &dd /*!< Value to compare. */) const;

	//! 'Less than' operator.
	int operator<(const Dinfo &dd /*!< Value to compare. */) const;


	//friend ostream &operator<<(ostream &os, const Dinfo &dd);

	//! Function to change name of detector.
	void setName(std::string nn /*!< New name of the detector. */);

	//! Function to change type of detector.
	void setType(Dtype tt /*!< New type of the detector (Dtype type). */);

	//! Function to change type of detector.
	void setType(std::string ss /*!< New type of the detector (determined from string). */);

    //! Function to change theta angle of detector.
    void setTheta(double theta /*!< New angle on the z-axis. */);

    //! Function to change phi angle of detector.
    void setPhi(double phi  /*!< New rotation in the xy-plane on the y-axis [deg]. */);
	
	//! Function to change telescope of detector.
	void setTelescope(std::string td /*!< New telescope info of the detector. */);	

	//! Function to get the detector name.
	/*! \return The name of the detector.
	 */
	inline std::string getName(void) const;

	//! Function to get type.
	/*! \return The type of the detector.
	 */
	inline Dtype getType(void) const;

    //! Function to get theta angle of detector.
    /*! \return the theta value of the detector.
     */
    inline double getTheta(void) const;
	
    //! Function to get phi angle of detector.
    /*! \return the phi value of the detector.
     */
    inline double getPhi(void) const;

    //! Fuction to get the angle with respect to
    /*! the beam axis. The beam axis is assumed
     *  to be along the y-axis.
     *  \return Angle with respect to the beam
     *  axis in radians.
     */
    inline double getScatAngle(void) const;

	//! Function to get telescope.
	/*! \return What telescope of the detector.
	 */
	inline std::string getTelescope(void) const;

private:

	//! The detector name. 
	std::string m_name;

	//! The detector type.
	Dtype m_type;

    //! Angle on the z-axis [deg].
    double m_theta;

    //! Rotation in the xy-plane on the y-axis [deg].
    double m_phi;
	
	//! The telescope name & position.
	std::string m_telescope;

};

inline std::string Dinfo::getName (void) const
{
	return m_name;
}

inline Dtype Dinfo::getType (void) const
{
	return m_type;
}

inline double Dinfo::getTheta(void) const
{
    return m_theta;
}

inline double Dinfo::getPhi(void) const
{
    return m_phi;
}

inline double Dinfo::getScatAngle(void) const
{
    return acos(sin(m_theta*acos(-1)/180.)*sin(m_phi*acos(-1)/180.));
}

inline std::string Dinfo::getTelescope (void) const
{
	return m_telescope;
}

//! Formatted stream output.
/*inline std::ostream& operator<< (std::ostream& _os, const Dinfo _e)
{
    _os << "Name: " << _e.getName() << " Type: " << _e.getType()
        << " Theta: " << _e.getTheta() << " Phi: " << _e.getPhi()
        << " Telescope: " << _e.getTelescope();
	return _os;
}*/

// #######################################################

/*!
 * \class Detectors
 * \brief Mapping of detector objects.
 * \details This is a class that reads a detector array from file and creates a mapping based on the ADC address of the detector.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \warning If elements from this list is compaired to strings it may use a lot of resorces.
 */
class Detectors {
public:

	//! Default constructor.
	Detectors(void);

	//! Construct a detector map from a text file.
	/*! For details on the formatting, @see setAll() */
	Detectors(std::string filename /*!< The filename of the file to read. */);

	//! Copy constructor
	/*! Needed for handling pass by value.
	 */
	Detectors(const Detectors &dd /*!< Value to copy. */);

	//! Destructor
	~Detectors(){}

	//! Assign operator
	Detectors &operator=(const Detectors &dd /*!< Map to assign. */);

	//! Find a detector by adc address.
	/*! \return the detector information,
	 *  if known, or default ("", none) otherwise.
	 */
	Dinfo Find(const uint16_t& address /*!< The adc address of the detector. */);

    Dinfo Find(const uint16_t& address /*!< The adc address of the detector. */) const;

	//! Add a parameter to the mapping.
	/*! Will print an warning if ADCaddress
	 *  allready exist in the mapping.
	 */
	void Add(const uint16_t& address,	/*!< The adc address of the detector to add.	*/
	 		 Dinfo dd 					/*!< The detector name and type. 			 	*/);

	//! Remove a detector from the mapping.
	void Remove(const uint16_t& address /*!< The adc address of the detector to remove.	*/);

	//! Set one or more detector from external text file.
	/*! The format for the text file is:
     *  <pre>
	 *  &lt;adc-address&gt; (tab/space(s)) &lt;detector-name&gt; (tab/space(s)) &lt;detector-type&gt;
     *  </pre>
     *  Only one detector for each line.\n
	 *  adc-address should be an unsigned integer.\n
	 *  detector-name should be a string without spaces.\n
	 *  The detector-type should be given as:\n
	 *  gamma-detectors: "gamma" or "GAMMA" (case insensitive)\n
	 *  silicon-detectors: "silicon" or "SILICON" (case insensitive)\n
	 *  Constant fraction discriminator: "cfd" or "CFD" (case insensitve)\n
	 *  None: "none" or "NONE" (case insensitive)\n
	 *  Unknown: If not any of the above.
	 *  Comment lines starts with "#".
	 *  The function will print a message if an error was encounted.
	 *  \return True if the file was parsed completely; if false, 
	 *  the file might be wrongly formatted and some detectors might
	 *  not have been mapped correctly.
	 */
	bool setAll(std::string filename /*!< The filename of the file to read. */);

	//! Fuction to generate a string of all the maped detectors.
	/*! \return A list of all listed detectors.
	 */
    //std::string getOutput(void);

private:
	//! The map type used in this class.
	typedef std::map<uint16_t, Dinfo> addresses_t;

	//! The map from address to detector object.
	addresses_t m_addresses;

	//! Function for getting a line.
	/*! Used for handling the data from input file.
	 * \return True if EOF is not reached; False otherwise.
	 */
	bool next_line(std::istream& in, 		/*!< File to read from 			*/
	 			   std::string& cmd_line	/*!< Line to read string into.	*/);
};

//! Creates a string containing the entire table of entries in a well formated way.
/*inline std::string Detectors::getOutput(void)
{
	std::string output;

	output += "ADC address => Name:     Type:\n";

	addresses_t::iterator ii = m_addresses.begin();
	for (ii = m_addresses.begin() ;
		 ii != m_addresses.end() ; ++ii)
	{
		std::string result;
		std::ostringstream convert;

		convert << (*ii).first << " => " << (*ii).second << std::endl;

		result = convert.str();

		output += result;
	}
	return output;
}

//! Easy readable stream output.
inline std::ostream& operator<< (std::ostream& _os, Detectors& _d)
{
	std::string output = _d.getOutput();
	_os << output;
	return _os;
}
*/
#endif // DETECTORREAD_H
