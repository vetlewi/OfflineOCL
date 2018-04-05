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

#ifndef TDRWORDBUFFER_H
#define TDRWORDBUFFER_H

#include <memory>

//! Structure type to contain individual decoded TDR words.
//! \author Vetle W. Ingeberg
//! \date 2015-2016
//! \copyright GNU Public License v. 3
/*typedef struct {
	uint16_t address;		//!< Holds the address of the ADC. 
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    int64_t timestamp;		//!< Timestamp read out.

    uint16_t tracelength;	//!< Length of traces in current word.
    uint16_t traces[2048]; 	//!< Each word can contain up to 2000 traces.

    uint16_t error;
    
} word_t;*/
typedef struct {
    uint16_t address;		//!< Holds the address of the ADC.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    double cfdcorr;         //!< Correction from the CFD.
    int64_t timestamp;		//!< Timestamp in [ns].
    char cfdfail;           //!< Flag to tell if the CFD was forced or not.
} word_t;


/*!
 * \class WordBuffer
 * \brief The WordBuffer class
 * \details A container for storing a buffer of TDR data words.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class WordBuffer {
protected:
//! Initilizer
	WordBuffer(int sz,		/*!< The size of the word buffer.	*/
			   word_t *buf	/*!< The buffer data.				*/)
		: size( sz )
		, buffer( buf )
		, is_set( true ) { }
public:
	//! Virtual no-op destructor.
	virtual ~WordBuffer() { }

	//! Get data from the buffer.
	/*! \return the dataword from the buffer.
	 */
    inline word_t GetData(int idx /*!< Index of element to return. */) const
    { return buffer[idx]; }

	//! Overloaded [] operator.
	/*! \return getData(idx).
	 */
	word_t operator[](int idx /*!< Index of element to return. */) const
		{ return GetData(idx); }

	//! Give access to the data.
	/*! Used by classes to fetch the data.
	 */
	word_t* GetBuffer()
        { return buffer.release(); }

	//! Get the buffer size.
	/*! \return the number of elements in the buffer.
	 */
	unsigned int GetSize() const
		{ return size; }

    void SetBuffer(word_t* newBuf)
        {
            buffer.reset(newBuf);
        }

	//! Create a new buffer of the same type.
	virtual WordBuffer* New() { return 0; }

	void Resize(unsigned int new_size)
	{
		size = new_size;
        buffer.reset(new word_t[size]);
		is_set = true;
	}

private:
	//! Size of the buffer in word_t words.
	unsigned int size;

	//! The buffer data.
    std::unique_ptr<word_t[]> buffer;

	//! If the buffer is created or not.
	bool is_set;
};

//! Buffer type used in this project.
/*!
 * \class TDRWordBuffer
 * \brief Subclass of WordBuffer with a spesific size.
 * \details This class has a predetermined size equal to half of the buffer size
 * used in the data files used in this project. This is beacuse half of the
 * buffer is the CFD reading which is useless for the sorting and is discarded.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class TDRWordBuffer : public WordBuffer {
    enum { BUFSIZE = 256 /*!< The size of a buffer in words. */ };
public:
	TDRWordBuffer() : WordBuffer(BUFSIZE, new word_t[BUFSIZE]) { }
	TDRWordBuffer(int sz, word_t *buf ) : WordBuffer(sz, buf) { }
	~TDRWordBuffer() { delete GetBuffer(); }
	WordBuffer* New() { return new TDRWordBuffer(); }
};

#endif // TDRWORDBUFFER_H
