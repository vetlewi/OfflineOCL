// FileReader 2.0

#include "FileReader.h"
#include "PixieSettings.h"
#include "experimentsetup.h"
#include "XIA_CFD.h"

#include <cstdint>
#include <iostream>

#include "TDRWordBuffer.h"

#define BUFFER_SIZE 1024
//#define BUFFER_SIZE 2048


#define DIRTY_FIX 0

inline uint64_t extract(const uint64_t &val, const int &begin, const int &end)
{
	uint64_t mask = (1 << (end - begin)) - 1;
	return (val >> begin) & mask;
}

void ShowBit32(uint32_t u){
    int values[32];
    for (int i = 0 ; i < 32 ; ++i){
        values[i] = u%2;
        u = u/2;
    }
    for (int i = 31 ; i >= 0 ; --i){
        std::cout << values[i];
    } std::cout << std::endl;
}

FileReader::FileReader() :
	file( 0 ),
    errorflag( false )
{
}

// #########################################################

FileReader::~FileReader()
{
	Close();
}

// #########################################################

bool FileReader::Open(std::string filename, size_t want)
{
    error_number = 0;
    read_number = 0;
	Close();
    file = std::fopen(filename.c_str(), "rb");
	errorflag = (file==0)
                ;//|| std::fseek(file, want, SEEK_SET) != (int)want;
    return !errorflag;
}

// #########################################################

void FileReader::Close()
{
	if (file){
		std::fclose( file );
        file = 0;
	}
}

// #########################################################

int FileReader::Read(WordBuffer *buffer)
{

    if ( errorflag || (!file) ){
        return -1;
    }

    word_t *data = new word_t[BUFFER_SIZE];
    size_t n_decoded=0;
    for (int i = 0 ; i < BUFFER_SIZE ; ++i){
        if ( ReadEvent(data[n_decoded]) )
             ++n_decoded;
        else
            break;
    }



    if (n_decoded != buffer->GetSize())
        buffer->Resize(n_decoded);

    buffer->SetBuffer(data);

    if ( (n_decoded != BUFFER_SIZE) || (!file) ){
        return 0;
    }

	return 1;
}

// #########################################################

bool FileReader::ReadEvent(word_t &hit)
{

    if ( fread(&eventdata, sizeof(uint32_t), 4, file) != 4 )
        return false;

    event_length = ( eventdata[0] & 0x3FFE0000 ) >> 17;

    hit.address = ( eventdata[0] & 0x00000FFF );

    hit.finishcode = ( ( eventdata[0] & 0x80000000 ) > 0 ) ? 1 : 0;

    // Calculate full timestamp.
    hit.timestamp = (eventdata[2] & 0xFFFF);
    hit.timestamp <<= 32;
    hit.timestamp |= eventdata[1];

    // Extract CFD.
    hit.cfddata = (eventdata[2] & 0xFFFF0000) >> 16;

    // Extract energy
    hit.adcdata = (eventdata[3] & 0xFFFF);

    // Move file to the end of the event event.
    if ( event_length != 4)
        fseek(file, sizeof(uint32_t)*(event_length - 4), SEEK_CUR);

    // Calculate the correct correction
    switch ( GetSamplingFrequency(hit.address) ) {
    case f100MHz :
        hit.cfdcorr = XIA_CFD_Fraction_100MHz(hit.cfddata, &hit.cfdfail);
        hit.timestamp *= 10;
        if ( hit.cfddata == 0 )
            hit.cfdfail = 1;
        break;
    case f250MHz :
        hit.cfdcorr = XIA_CFD_Fraction_250MHz(hit.cfddata, &hit.cfdfail);
        hit.timestamp *= 8;
        if ( hit.cfddata == 0 )
            hit.cfdfail = 1;
        break;
    case f500MHz :
        hit.cfdcorr = XIA_CFD_Fraction_500MHz(hit.cfddata, &hit.cfdfail);
        hit.timestamp *= 10;
        if ( hit.cfddata == 0 )
            hit.cfdfail = 1;
        break;
    default :
        hit.cfdcorr = 0;
        hit.cfdfail = 1;
        hit.timestamp *= 10;
        break;
    }

    return true;
}
