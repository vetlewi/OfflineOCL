// FileReader 2.0

#include "FileReader.h"
#include "experimentsetup.h"
#include "XIA_CFD.h"

#include <cstdint>

#include "WordBuffer.h"

inline int seek(std::FILE* stream, int offset)
{
    long moved = 0, length = 0;
    uint32_t head;
    while ( moved < offset ){
        if ( std::fread(&head, sizeof(uint32_t), 1, stream) != 1 ) // Error while reading.
            return 1;
        length =  ( head & 0x3FFE0000 ) >> 17;
        if ( std::fseek(stream, length, SEEK_CUR) != 0 )
            return 1;
        ++moved;
    }
    return 0;
}

FileReader::FileReader() :
    file_stdio( nullptr ),
    errorflag( false )
{
}

// #########################################################

FileReader::~FileReader()
{
	Close();
}

// #########################################################

bool FileReader::Open(const char *filename, int want)
{
	Close();
    file_stdio = std::fopen(filename, "rb");

    errorflag = ( file_stdio==nullptr )
                || ( seek(file_stdio, want) != 0);

    return !errorflag;
}

// #########################################################

void FileReader::Close()
{
    if (file_stdio){
        std::fclose( file_stdio );
        file_stdio = nullptr;
	}
}

// #########################################################

int FileReader::Read(word_t *buffer, int size)
{

    if ( errorflag || (!file_stdio) ){
        return -1;
    }


    int have = 0;
    while ( have < size ){
        if ( !ReadEvent( buffer[have++]) ){

            // Check if EOF or error.
            if ( feof(file_stdio) ){
                errorflag = false;
                Close();
            } else if ( ferror(file_stdio) ){
                errorflag = true;
            } else {
                errorflag = true;
            }
            return errorflag ? -1 : 0;
        }
    }
	return 1;
}

// #########################################################

bool FileReader::ReadEvent(word_t &hit)
{

    uint32_t eventdata[4];
    uint32_t event_length;

    if ( std::fread(&eventdata, sizeof(uint32_t), 4, file_stdio) != 4 )
        return false; // Error or EOF.

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

    // Move file to the end of the event event

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

    if ( event_length != 4)
        return ( std::fseek(file_stdio, sizeof(uint32_t)*(event_length - 4), SEEK_CUR) == 0 );

    return true;
}
