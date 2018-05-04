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

#if TDRFORMAT
    // If TDR format we read the first block of data now.
    ReadBlock();
#endif // TDRFORMAT

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

#define BLOCKSIZE 65536

inline uint64_t extract(const uint64_t &val, const int &begin, const int &end)
{
    uint64_t mask = (1 << (end - begin)) - 1;
    return (val >> begin) & mask;
}

static int64_t topTime;

int DecodeWords(word_t *data, uint64_t *buf, size_t length)
{
    int n_decoded=0;

    // We declare them here so that we don't use clock cycles doing that in the loop.
    uint16_t adcdata_w1;
    uint16_t adcdata_w2;
    uint16_t address_w1;
    uint16_t address_w2;
    uint64_t timestamp_w1;
    uint64_t timestamp_w2;

    if (topTime == 0){
        for (size_t i = 0 ; i < length ; ++i){
            uint16_t iden = extract(buf[i], 62, 64);
            if (iden == 2){
                topTime = extract(buf[i], 32,32+20)<<28;
                break;
            }
        }
    }

    for (size_t i = 0 ; i < length - 1 ; ++i){
        uint16_t iden = extract(buf[i], 62, 64);
        if (iden == 2){
            topTime = extract(buf[i], 32, 32+20) << 28;
        } else if (iden == 3) { // We are assuming that two consecutive words are the ADC value and the CFD value.
            adcdata_w1 = extract(buf[i], 32, 32+16);
            adcdata_w2 = extract(buf[i+1], 32, 32+16);
            address_w1 = extract(buf[i], 32+16, 32+16+12);
            address_w2 = extract(buf[i+1],32+16, 32+16+12);
            timestamp_w1 = extract(buf[i], 0, 28)+topTime;
            timestamp_w2 = extract(buf[i+1], 0, 28)+topTime;
            if ( ((address_w1&16)==0) && (address_w2 == address_w1+16) && (timestamp_w1 == timestamp_w2) ){
                data[n_decoded].address = address_w1;
                data[n_decoded].adcdata = adcdata_w1;
                data[n_decoded].cfddata = adcdata_w2;
                data[n_decoded].timestamp = timestamp_w1;
                ++n_decoded;
                ++i;
            } else if ( ((address_w2&16)==0) && (address_w1 == address_w2+16) && (timestamp_w2 == timestamp_w1) ){
                data[n_decoded].address = address_w2;
                data[n_decoded].adcdata = adcdata_w2;
                data[n_decoded].cfddata = adcdata_w1;
                data[n_decoded].timestamp = timestamp_w2;
                ++n_decoded;
                ++i;
            }
        }
    }

    for (int i = 0 ; i < n_decoded ; ++i){

        switch ( GetSamplingFrequency(data[i].address) ) {
        case f100MHz :
            data[i].cfdcorr = XIA_CFD_Fraction_100MHz(data[i].cfddata, &data[i].cfdfail);
            data[i].timestamp *= 10;
            if ( data[i].cfddata == 0 )
                data[i].cfdfail = 1;
            break;
        case f250MHz :
            data[i].cfdcorr = XIA_CFD_Fraction_250MHz(data[i].cfddata, &data[i].cfdfail);
            data[i].timestamp *= 8;
            if ( data[i].cfddata == 0 )
                data[i].cfdfail = 1;
            break;
        case f500MHz :
            data[i].cfdcorr = XIA_CFD_Fraction_500MHz(data[i].cfddata, &data[i].cfdfail);
            data[i].timestamp *= 10;
            if ( data[i].cfddata == 0 )
                data[i].cfdfail = 1;
            break;
        default :
            data[i].cfdcorr = 0;
            data[i].cfdfail = 1;
            data[i].timestamp *= 10;
            break;
        }

    }

    return n_decoded;

}

void FileReader::ReadBlock()
{
    if ( !file_stdio || errorflag )
        return;

    // Read header of the module.
    if ( std::fread(&header, sizeof(DATA_HEADER_T), 1, file_stdio) != 1 ){
        errorflag = true;
        return;
    }

    // Read the rest of the block.
    uint64_t data[(BLOCKSIZE-24)/sizeof(uint64_t)];
    int length = fread(data, sizeof(uint64_t), (BLOCKSIZE-24)/sizeof(uint64_t), file_stdio);
    found = DecodeWords(words, data, length);
    have_given = 0;
    return;
}

bool FileReader::ReadEvent(word_t &hit)
{

    // First we check if we have 'handed over' all the events extracted in the current
    if ( errorflag )
        return false;
    else if (have_given < found){
        hit = words[have_given++];
        return true;
    } else {
        ReadBlock();
        if (have_given < found){
            hit = words[have_given++];
            return true;
        } else
            return false;
    }
}
