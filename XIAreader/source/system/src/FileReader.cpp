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
    uint32_t firstword;
    if (fread(&firstword, sizeof(uint32_t), 1, file) != 1){
        return false;
    }

    size_t evt_length = ( firstword & EVENTLENGTH ) >> EVENTLENGTH_OFFSET;

    // We read in the entire thing into our buffer (this is a static array in memory... Saves memory space I guess)
    if (fread(tmp, sizeof(uint32_t), evt_length-1, file) != evt_length - 1){
        return false; // Error reading.
    }

    hit.address = (firstword & CHANNELID) >> CHANNELID_OFFSET;
    int64_t time_low = (tmp[0] & EVENTTIMELOW) >> EVENTTIMELOW_OFFSET;
    int64_t time_high = (tmp[1] & EVENTTIMEHIGH) >> EVENTTIMEHIGH_OFFSET;
    hit.timestamp = (time_high << 32);
    hit.timestamp |= time_low;
    hit.cfddata = (tmp[1] & 0xFFFF0000) >> 16;
    hit.adcdata = (tmp[2] & 0xFFFF);

    switch ( GetSamplingFrequency(hit.address) ) {
    case f100MHz:
        hit.cfdcorr = XIA_CFD_Fraction_100MHz(hit.cfddata, &hit.cfdfail);
        hit.timestamp *= 10; // Convert to ns
        break;
    case f250MHz:
        hit.cfdcorr = XIA_CFD_Fraction_250MHz(hit.cfddata, &hit.cfdfail);
        hit.timestamp *= 8; // Convert to ns
        break;
    case f500MHz:
        hit.cfdcorr = XIA_CFD_Fraction_500MHz(hit.cfddata, &hit.cfdfail);
        hit.timestamp *= 10; // Convert to ns
        break;
    default:
        break;
    }
    return true;
}

/*word_t FileReader::ReadEvent(bool &success){

	word_t event_extracted;
    event_extracted.error = false;
	uint32_t fwbuf;
	if (fread(&fwbuf, sizeof(uint32_t), 1, file) != 1){
        success = false;
		return event_extracted;
	}
    size_t event_length = (fwbuf&EVENTLENGTH) >> EVENTLENGTH_OFFSET;


#if DIRTY_FIX
    read_number++;
    if (event_length != 1008){
        error_number++;
        ShowBit32(fwbuf);
        std::cout << "Error nr. " << error_number << "/" << read_number << " Event_length = " << event_length << " from " << ((fwbuf&CHANNELID) >> CHANNELID_OFFSET) << std::endl;
        event_length = 1008;
        event_extracted.error = true;
    }
#endif // DIRTY_FIX


    uint32_t *data = new uint32_t[event_length];
    if (fread(data+1, sizeof(uint32_t),event_length-1,file) != event_length-1){
        success = false;
        delete [] data;
        return event_extracted;
	}
	data[0] = fwbuf;

	size_t header_length = (data[0]&HEADERSIZE) >> HEADERSIZE_OFFSET;

#if DIRTY_FIX
    if (header_length > 8){
        std::cout << "This is strange... Header_length = " << header_length << std::endl;
        std::cout << "Resetting it to 8. This should not be done (very nasty!)" << std::endl;
        header_length = 8;
    }
#endif // DIRTY_FIX


    event_extracted.address = (data[0]&CHANNELID) >> CHANNELID_OFFSET;

    uint64_t timelow = (data[1]&EVENTTIMELOW) >> EVENTTIMELOW_OFFSET;
    uint64_t timehigh = (data[2]&EVENTTIMEHIGH) >> EVENTTIMEHIGH_OFFSET;
    event_extracted.timestamp = timehigh << 32;
    event_extracted.timestamp |= timelow;
    event_extracted.cfddata = (data[2]&0xFFFF0000) >> 16;
    event_extracted.adcdata = (data[3]&0xFFFF) >> 0;
    event_extracted.tracelength = (data[3]&TRACELENGTH) >> TRACELENGTH_OFFSET;

    if (event_extracted.tracelength <= 2000){
        for (size_t i = 0 ; i < event_length - header_length ; ++i){
            if (2*i+1 < 2048){
                event_extracted.traces[2*i] = (data[i+header_length]&FIRST_ADC) >> FIRST_ADC_OFFSET;
                event_extracted.traces[2*i+1] = (data[i+header_length]&SECOND_ADC) >> SECOND_ADC_OFFSET;
            }
        }
    }

    success = true;
    delete[] data;
    return event_extracted;
}*/
