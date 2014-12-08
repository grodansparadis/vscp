// FILE: vscphelper.cpp
//
// Copyright (C) 2002-2014 
// Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

// HISTORY:
//    021107 - AKHE Started this file
//


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/defs.h"
#include "wx/app.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <math.h>


#ifndef WIN32
#include <sys/times.h>
#include <sys/types.h>
#endif

#include "vscp.h"
#include "../common/mdf.h"
#include "../../common/crc8.h"
#include "vscphelper.h"


#define Swap8Bytes(val) \
	 ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
	   (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
	   (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
	   (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )


// ***************************************************************************
//                                General Helpers
// ***************************************************************************


///////////////////////////////////////////////////////////////////////////////
// vhlp_lowercase
//

int vscp_lowercase(const char *s) 
{
	return tolower(* (const unsigned char *) s);
}


///////////////////////////////////////////////////////////////////////////////
// vhlp_strcasecmp
//

int vscp_strcasecmp(const char *s1, const char *s2) 
{
	int diff;

	do {
		diff = vscp_lowercase(s1++) - vscp_lowercase(s2++);
	} while (diff == 0 && s1[-1] != '\0');

	return diff;
}

///////////////////////////////////////////////////////////////////////////////
// vhlp_strncasecmp
//

int vscp_strncasecmp(const char *s1, const char *s2, size_t len) 
{
	int diff = 0;

	if ( len > 0 ) {
		do {
			diff = vscp_lowercase(s1++) - vscp_lowercase(s2++);
		} while (diff == 0 && s1[-1] != '\0' && --len > 0);
	}

	return diff;
}


///////////////////////////////////////////////////////////////////////////////
// vhlp_bin2str
//

void vscp_bin2str(char *to, const unsigned char *p, size_t len  ) 
{
	static const char *hex = "0123456789abcdef";

	for (; len--; p++) {
		*to++ = hex[p[0] >> 4];
		*to++ = hex[p[0] & 0x0f];
	}

	*to = '\0';
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getTimeString
//

void vscp_getTimeString( char *buf, size_t buf_len, time_t *t ) 
{
	strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime( t ) );
}


// ***************************************************************************
//                                Data Coding Helpers
// ***************************************************************************


uint8_t vscp_getMeasurementDataCoding(const vscpEvent *pEvent)
{
    uint8_t datacoding_byte = -1;
    
    if ( NULL == pEvent ) return -1;
    if ( NULL == pEvent->pdata ) return -1;
    if ( pEvent->sizeData < 1 ) return -1;
    
    if ( VSCP_CLASS1_MEASUREMENT == pEvent->vscp_class ) {
        datacoding_byte = pEvent->pdata[0];
    }
    else if ( VSCP_CLASS2_LEVEL1_MEASUREMENT == pEvent->vscp_class ) {
        if ( pEvent->sizeData >= 16 ) datacoding_byte = pEvent->pdata[16];
    } 
        
    return datacoding_byte;
}


//////////////////////////////////////////////////////////////////////////////
// getDataCodingBitArray
//

uint64_t vscp_getDataCodingBitArray(const uint8_t *pCode,
                                        const uint8_t length)
{
	uint64_t bitArray = 0;

    if ( NULL == pCode ) return 0;
    if ( ( length > 7 ) || ( length <= 1 ) ) return 0;

    for ( int i=0; i<length-1; i++ ) {
        bitArray = bitArray << 8;
        bitArray |= *(pCode + 1 + i);
    }

	return bitArray;
}


//////////////////////////////////////////////////////////////////////////////
// vscp_getDataCodingInteger
//

int64_t vscp_getDataCodingInteger(const uint8_t *pCode, uint8_t length )
{
    int64_t value64 = 0;
    //uint8_t byteArray[8];

    if ( NULL == pCode ) return 0;
    if ( length < 2 ) return 0;

    // Check if this is a negative number
	if ( (*(pCode + 1)) & 0x80) {
	    memset( (uint8_t *)&value64, 0xff, 8 );
	}
    else {
        memset( (uint8_t *)&value64, 0, 8 );
    }

    for ( int i=1; i<length; i++ ) {
        value64 = ( value64 << 8 ) + pCode[i];
    }

    return value64;
}


//////////////////////////////////////////////////////////////////////////////
// getDataCodingNormalizedInteger
//

double vscp_getDataCodingNormalizedInteger(const uint8_t *pCode,
		                                      uint8_t length )
{
	uint8_t valarray[ 8 ];
    uint8_t normbyte;
	uint8_t decibyte;
	int64_t value64;
	double value = 0;
	bool bNegative = false; // set for negative number

	// Check pointer
	if (NULL == pCode) return 0;

	// Check character count
	if ((length > 8) || (length < 2)) return 0;

	memset(valarray, 0, sizeof( valarray));
	normbyte = *pCode;
	decibyte = *(pCode + 1);

	// Check if this is a negative number
	if ( (*(pCode + 2)) & 0x80) {
		bNegative = true;
	}

	switch (length - 2) {

	case 1: // 8-bit
		memcpy((char *) &valarray, (pCode + 2), (length - 2));
		value = *((int8_t *) valarray);
		break;

	case 2: // 16-bit
		memcpy((char *) &valarray, (pCode + 2), (length - 2));	
		value = wxINT16_SWAP_ON_LE(* ((int16_t *) valarray));	
		break;

	case 3: // 24-bit
		memcpy(((char *) &valarray + 1), (pCode + 2), (length - 2));
		if ( bNegative ) *valarray = 0xff; // First byte must be 0xff			
		value = wxINT32_SWAP_ON_LE(* ((int32_t *) valarray));	
		break;

	case 4: // 32-bit
		memcpy((char *) &valarray, (pCode + 2), (length - 2));	
		value = wxINT32_SWAP_ON_LE(* ((int32_t *) valarray));
		break;

	case 5: // 40-bit
		memcpy(((char *) &valarray + 3), (pCode + 2), (length - 2));
		if ( bNegative ) {			
			*valarray = 0xff; // First byte must be 0xff
			*(valarray+1) = 0xff;
			*(valarray+2) = 0xff;
		}
	
#ifdef  WORDS_BIGENDIAN		
		value = *((int64_t *) valarray);
#else
		value64 = Swap8Bytes( *((int64_t *) valarray) );
		value = value64;
#endif				
		break;

	case 6: // 48-bit
		memcpy(((char *) &valarray + 2), (pCode + 2), (length - 2));
		if ( bNegative ) {			
			*valarray = 0xff; // First byte must be 0xff
			*(valarray+1) = 0xff;
		}

#ifdef  WORDS_BIGENDIAN			
		value = *((int64_t *) valarray);
#else
		value64 = Swap8Bytes( *((int64_t *) valarray) );
		value = value64;
#endif			
		break;
	}

	// Bring back decimal points
#ifdef WIN32
	if ( decibyte & 0x80 ) {
        decibyte &= 0x7f;
        value = value * (pow(10.0, decibyte));
    }
    else {
        value = value / (pow(10.0, decibyte));
    }
#else
    if ( decibyte & 0x80 ) {
        decibyte &= 0x7f;
        value = value * (pow(10, decibyte));
    }
    else {
        value = value / (pow(10, decibyte));
    }
#endif
	return value;

}




//////////////////////////////////////////////////////////////////////////////
// getDataCodingString
//


wxString& vscp_getDataCodingString(const unsigned char *pCode,
                                      unsigned char length)							
{

	static wxString str;
	char buf[ 20 ];

	str.Empty();

	if (NULL != pCode) {
		memset(buf, 0, sizeof( buf));
		memcpy(buf, pCode+1, length-1 );
		str = wxString::FromUTF8(buf);
	}

	return str;

}

//////////////////////////////////////////////////////////////////////////////
// getDataCodingString
//


bool vscp_getDataCodingString(const unsigned char *pCode,
                                        unsigned char dataSize, 
                                        wxString& strResult )							
{	
	char buf[ 20 ];

    if ( NULL == pCode ) return false;

	strResult.Empty();
	if (NULL != pCode) {
		memset(buf, 0, sizeof( buf));
		memcpy( buf, pCode+1, dataSize-1 );
	
		strResult = wxString::FromUTF8(buf);
	}

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// getMeasurementAsFloat
//

float vscp_getMeasurementAsFloat(const unsigned char *pCode, 
                                    unsigned char length)								
{
    float *pfloat = NULL;
    float value = 0.0f;
    
    // Check pointers
    if ( NULL == pCode ) return false;
    
	//float value;
	//value = std::numeric_limits<float>::infinity();
	if (length >= 5) {
		pfloat = (float*)(pCode + 1);
		value = *pfloat;
		//value = pfloat[0];
		// please insert test for (!NaN || !INF)
	}
    
	return value;
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPMeasurementAsString
//
//


bool vscp_getVSCPMeasurementAsString(const vscpEvent *pEvent, wxString& strValue)

{
	int i, j;
	int offset = 0;
    
    // Check pointers
    if ( NULL == pEvent ) return false;


	strValue.Empty();


	// Check pointers
	if (NULL == pEvent) return false;
	if (NULL == pEvent->pdata) return false;

	// If class >= 512 and class < 1024 we
	// have GUID in front of data. 
	if ( ( pEvent->vscp_class >= VSCP_CLASS2_LEVEL1_PROTOCOL) && 
			(pEvent->vscp_class < VSCP_CLASS2_PROTOCOL) ) {
		offset = 16;
	}
	
	// Must be at least two data bytes
	if (pEvent->sizeData-offset < 2) return false;

	unsigned short type = (0x07 & (pEvent->pdata[0+offset] >> 5));
	switch (type) {

	case 0: // series of bits
		for (i = 1; i < (pEvent->sizeData-offset); i++) {

			for ( j=7; j>=0; j--) {

				if (pEvent->pdata[ i+offset ] & (1<<j)) {		
					strValue += wxT("1");				
				} else {
					strValue += wxT("0");
				}

				//if ((i != (pEvent->sizeData - 1 - offset)) && (j != 0)) {
					//strValue += wxT(",");
				//}
			}
            strValue += wxT(" ");
		}
		break;

	case 1: // series of bytes
		for (i = 1; i < (pEvent->sizeData-offset); i++) {

			strValue += wxString::Format(wxT("%d"), pEvent->pdata[ i+offset ]);
		
			if (i != (pEvent->sizeData - 1 - offset)) {

				strValue += wxT(",");

			}
		}
		break;

	case 2: // string
	{
		char strData[ 9 ];
		memset(strData, 0, sizeof( strData));
		for (i = 1; i < (pEvent->sizeData-offset); i++) {
			strData[ i - 1 ] = pEvent->pdata[ i+offset ];
		}

		strValue = wxString::FromAscii(strData);
	
	}
		break;

	case 3: // integer
	{
        double value = (double)vscp_getDataCodingInteger( pEvent->pdata+offset, pEvent->sizeData-offset );
        strValue.Printf(_("%.0lf"), value );
	}
		break;

	case 4: // normalized integer
	{
	    double value = vscp_getDataCodingNormalizedInteger( pEvent->pdata+offset, pEvent->sizeData-offset);
        strValue.Printf(_("%lf"), value );
	}
		break;


	case 5: // Floating point value
	{
		// s eeeeeeee mmmmmmmmmmmmmmmmmmmmmmm 
		// s = sign bit( 1-bit) 
		// e = exponent ( 8-bit) 
		// m = mantissa (23-bit)
		int sign = 1;
		unsigned char exponent;

		// Check the sign
		if (pEvent->pdata[ 1+offset ] & 0x80) {
			sign = -1;
		}

		// Clear the sign bit
		pEvent->pdata[ 1+offset ] &= 0x7f;

		// Get the exponent
		exponent = (pEvent->pdata[ 1+offset ] << 1);
		if (pEvent->pdata[ 2+offset ] & 0x80) {
			exponent = exponent | 1;
		}

		// Clear the exponent
		pEvent->pdata[ 1+offset ] = 0;
		pEvent->pdata[ 2+offset ] &= 0x7f;

        uint32_t value = *((uint32_t *) (pEvent->pdata + 1 + offset));
        value = wxUINT32_SWAP_ON_LE(value);

        wxDouble dValue = value;
        dValue = sign * (dValue * pow(10.0, exponent));
        strValue.Printf(_("%f"), dValue);

	}
		break;

	case 6: // Not defined yet
		break;

	case 7: // Not defined yet
		break;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPMeasurementAsDouble
//
//

bool vscp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue)
{

    wxString str;
   
    // Check pointers
    if ( NULL == pEvent ) return false;
    if ( NULL == pvalue ) return false;
    
    if ( (VSCP_CLASS1_MEASUREMENT == pEvent->vscp_class) || 
             (VSCP_CLASS2_LEVEL1_MEASUREMENT == pEvent->vscp_class) ||
             (VSCP_CLASS1_MEASUREZONE == pEvent->vscp_class) || 
             (VSCP_CLASS1_SETVALUEZONE == pEvent->vscp_class) ) {

            if ( !vscp_getVSCPMeasurementAsString( pEvent, str ) ) return false;
            if ( !str.ToDouble( pvalue ) ) return false;

    }
    else if ( VSCP_CLASS1_MEASUREMENT64 == pEvent->vscp_class ){

        if ( !vscp_getVSCPMeasurementFloat64AsString( pEvent, str ) ) return false;
        if ( !str.ToDouble( pvalue ) ) return false;

    }
    else if ( VSCP_CLASS2_MEASUREMENT_STR == pEvent->vscp_class ){
	
        wxString str;		
        char buf[512];
        
        if ( 0 == pEvent->sizeData || NULL == pEvent->pdata ) return false;
        memcpy( buf, pEvent->pdata + 4, pEvent->sizeData-4 );

        str = wxString::FromAscii( buf );
        str.ToDouble( pvalue );
		
    }
    else {
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPMeasurementFloat64AsString
//
//

bool vscp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent, wxString& strValue)
{
    int offset = 0;
    
    // If class >= 512 and class <1024 we
	// have GUID in front of data. 
	if ( ( pEvent->vscp_class >= VSCP_CLASS2_LEVEL1_PROTOCOL) && 
			(pEvent->vscp_class < VSCP_CLASS2_PROTOCOL) ) {
		offset = 16;
	}
    
    if (pEvent->sizeData-offset != 8) return false;
    
	double *pfloat = (double*)(pEvent->pdata+offset);
    strValue.Printf( _("%lf"), *pfloat );

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVSCPMeasurementWithZoneAsString
//
//

bool vscp_getVSCPMeasurementWithZoneAsString(const vscpEvent *pEvent, wxString& strValue)
{
    int offset = 0;
    
    // If class >= 512 and class <1024 we
	// have GUID in front of data. 
	if ( ( pEvent->vscp_class >= VSCP_CLASS2_LEVEL1_PROTOCOL) && 
			(pEvent->vscp_class < VSCP_CLASS2_PROTOCOL) ) {
		offset = 16;
	}
    
    // Must at least have indez, zone, subzone, normalizewr byte, one data byte
    if (pEvent->sizeData-offset < 5) return false;
    
    // We mimic a standard measurement
    vscpEvent eventMimic;
	eventMimic.pdata = new uint8_t[pEvent->sizeData-offset-3];
    eventMimic.vscp_class = pEvent->vscp_class;
    eventMimic.vscp_type = pEvent->vscp_type;
    eventMimic.sizeData = pEvent->sizeData;
    memcpy( eventMimic.pdata, pEvent->pdata+offset+3, pEvent->sizeData-offset-3 );
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// convertFloatToNormalizedEventData
//

bool vscp_convertFloatToNormalizedEventData( uint8_t *pdata,
                                                uint16_t *psize,
                                                double value,                                                
                                                uint8_t unit,
                                                uint8_t sensoridx )
{
    // Check pointer
    if ( NULL == pdata ) return false;
    if ( NULL == psize ) return false;

    // No data assigned yet
    *psize = 0;

    unit &= 3;      // Mask of invalid bits
    unit <<= 3;     // Shift to correct position

    sensoridx &= 7;   // Mask of invalid bits

    char buf[128];
    bool bNegative = (value>0) ? false : true ;
    int ndigits = 0;
    uint64_t val64;
    double intpart;
#ifdef WIN32
    _snprintf(buf, sizeof(buf), "%g", value);
#else
    snprintf(buf, sizeof(buf), "%g", value);
#endif
    char *pos = strchr(buf,'.');
    if ( NULL != pos ) {
        ndigits = strlen(pos)-1;
    }
    else {
       ndigits=0;
    }

    (void)modf( value, &intpart );
    val64 = (uint64_t)(value * pow(10.0,ndigits));

    wxUINT64_SWAP_ON_LE(val64);

    if ( val64 < ((double)0x80) ) {
        *psize = 3;
        pdata[2] = val64 & 0xff;
    }
    else if ( val64 < ((double)0x8000) ) {
        *psize = 4;
        pdata[2] = (val64 >> 8) & 0xff;
        pdata[3] = val64 & 0xff;
    }
    else if ( val64 < ((double)0x800000) ) {
        *psize = 5;
        pdata[2] = (val64 >> 16) & 0xff;
        pdata[3] = (val64 >> 8) & 0xff;
        pdata[4] = val64 & 0xff;
    }
    else if ( val64 < ((double)0x80000000) ) {
        *psize = 6;
        pdata[2] = (val64 >> 24) & 0xff;
        pdata[3] = (val64 >> 16) & 0xff;
        pdata[4] = (val64 >> 8) & 0xff;
        pdata[5] = val64 & 0xff;
    }
    else if ( val64 < ((double)0x8000000000) ) {
        *psize = 7;
        pdata[2] = (val64 >> 32) & 0xff;
        pdata[3] = (val64 >> 24) & 0xff;
        pdata[4] = (val64 >> 16) & 0xff;
        pdata[5] = (val64 >> 8) & 0xff;
        pdata[6] = val64 & 0xff;
    }
    else if ( val64 < ((double)0x80000000) ) {
        *psize = 8;
        pdata[2] = (val64 >> 40) & 0xff;
        pdata[3] = (val64 >> 32) & 0xff;
        pdata[4] = (val64 >> 24) & 0xff;
        pdata[5] = (val64 >> 16) & 0xff;
        pdata[6] = (val64 >> 8) & 0xff;
        pdata[7] = val64 & 0xff;
    }
    else {
        return false;
    }

    pdata[0] = VSCP_DATACODING_NORMALIZED + unit + sensoridx;     // Normalized integer + unit + sensorindex
    pdata[1] = VSCP_DATACODING_NORMALIZED + ndigits;              // Decimal point shifted five steps to the left

    return true;
} 


//////////////////////////////////////////////////////////////////////////////
// vscp_convertFloatToFloatEventData
//

bool vscp_convertFloatToFloatEventData( uint8_t *pdata,
                                                uint16_t *psize, 
                                                float value,
                                                uint8_t unit,
                                                uint8_t sensoridx )
{
    // Max an min for Single-precision floating-point IEEE 754-1985
    double float_max = 3.4e38;
    double float_min = -3.4e38;

    // Check pointer
    if ( NULL == pdata ) return false;
    if ( NULL == psize ) return false;

    if ( value > float_max ) return false;
    if ( value < float_min ) return false;

    // We must make sure
    if ( 4 !=  sizeof( float ) ) return false;

    wxUINT32_SWAP_ON_LE( (uint32_t)value );
    void *p = (void *)&value;

    *psize = 5;
    pdata[0] = VSCP_DATACODING_SINGLE + unit + sensoridx;  // float + unit + sensorindex
    memcpy( pdata + 1, p, 4 );
          
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// vscp_convertIntegerToNormalizedEventData
//

bool vscp_convertIntegerToNormalizedEventData( uint8_t *pdata,
                                                    uint16_t *psize,
                                                    uint64_t val64,
                                                    uint8_t unit,
                                                    uint8_t sensoridx )
{
    uint8_t i;
    uint8_t data[8];

    uint8_t *p = (uint8_t *)&val64;


    // wxUINT64_SWAP_ON_LE( val64 );  does not work at least in 32-bit builds
    if ( wxIsPlatformLittleEndian() ) {
        for ( i=7; i>0; i--) {
            data[ 7-i ] = *(p+i);
        }
    }
    else {
        memcpy ( data, (uint8_t *)val64, 8 );
    }

    // Count the leasing zeror
    uint8_t nZeros = 0;
    for ( i=0; i<8; i++ ) {
        if ( *(p+i) ) break;
        nZeros++;
    }




    *psize = 1; // Size will be at least one byte
    int pos = 0;
    for ( i = nZeros; i<8; i++ ) {
        pdata[ pos + 1] =  *(p+i+nZeros);
        (*psize)++;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// vscp_makeFloatMeasurementEvent
//

bool vscp_makeFloatMeasurementEvent( vscpEvent *pEvent, 
                                        float value,
                                        uint8_t unit,
                                        uint8_t sensoridx )
{
    uint8_t offset = 0;
    
    // Allocate data if needed
    if ( ( NULL == pEvent->pdata ) && 
        ( VSCP_CLASS1_MEASUREMENT == pEvent->vscp_class ) ) {
        offset = 0;
        pEvent->pdata = new uint8_t[ 16 + 5 ];
        if ( NULL == pEvent->pdata ) return false;
    }
    else if ( ( NULL == pEvent->pdata ) &&
         ( VSCP_CLASS1_MEASUREMENT == pEvent->vscp_class ) ) {
        offset = 16;
        pEvent->pdata = new uint8_t[ 5 ];
        if ( NULL == pEvent->pdata ) return false;
    }
    else {
        return false;
    }

    return vscp_convertFloatToFloatEventData( pEvent->pdata+offset,
                                                &pEvent->sizeData, 
                                                value,
                                                unit,
                                                sensoridx );
}

//////////////////////////////////////////////////////////////////////////////
// vscp_getVSCPMeasurementZoneAsString
//

bool vscp_getVSCPMeasurementZoneAsString(const vscpEvent *pEvent, wxString& str)
{
    return false;   // TODO
}


//////////////////////////////////////////////////////////////////////////////
// replaceBackslash
//

wxString& vscp_replaceBackslash(wxString& wxstr)
{
	int pos;
	while (-1 != (pos = wxstr.Find('\\'))) {
		wxstr[ pos ] = '/';
	}

	return wxstr;
}


////////////////////////////////////////////////////////////////////////////////////
// readStringValue

uint32_t vscp_readStringValue(const wxString& strval)
{
	static unsigned long val;
	wxString str = strval;
	
	str.Trim();
	str.Trim(false);
	str.MakeLower();
	if (wxNOT_FOUND != str.Find(_("0x"))) {
		if ( !str.ToULong(&val, 16) ) {
			val = 0;
		}
	} else {
		if ( !str.ToULong(&val) ) {
			val = 0;
		}
	}

	return val;
}


///////////////////////////////////////////////////////////////////////////////
// getVscpPriority

unsigned char vscp_getVscpPriority(const vscpEvent *pEvent)
{
	// Must be a valid message pointer
	if (NULL == pEvent) return 0;

	return (( pEvent->head >> 5 ) & 0x07);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityEx

unsigned char vscp_getVscpPriorityEx(const vscpEventEx *pEvent)
{
	// Must be a valid message pointer
	if (NULL == pEvent) return 0;

	return (( pEvent->head >> 5 ) & 0x07);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpPriority

void vscp_setVscpPriority(vscpEvent *pEvent, unsigned char priority)
{
	// Must be a valid message pointer
	if (NULL == pEvent) return;

    pEvent->head &= ~VSCP_HEADER_PRIORITY_MASK;
	pEvent->head |= ( priority<<5);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityEx

void vscp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority)
{
	// Must be a valid message pointer
	if (NULL == pEvent) return;

	pEvent->head &= ~VSCP_HEADER_PRIORITY_MASK;
	pEvent->head |= (priority<<5);
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPheadFromCANALid
//

unsigned char vscp_getVSCPheadFromCANALid(uint32_t id)
{
    uint8_t hardcoded = 0;
    uint8_t priority = (0x07 & (id >> 26));
    if ( id & (1<<25) ) hardcoded = VSCP_HEADER_HARD_CODED;
	return ( (priority<<5) | hardcoded );
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPclassFromCANALid
//

uint16_t vscp_getVSCPclassFromCANALid(uint32_t id)
{
	return(uint16_t) (0x1ff & (id >> 16));
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPtypeFromCANALid
//

uint16_t vscp_getVSCPtypeFromCANALid(uint32_t id)
{
	return(uint16_t) (0xff & (id >> 8));
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPnicknameFromCANALid
//

uint8_t vscp_getVSCPnicknameFromCANALid(uint32_t id)
{
	return( id & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
// getCANALidFromVSCPdata
//

uint32_t vscp_getCANALidFromVSCPdata(unsigned char priority, 
		                            const uint16_t vscp_class, 
		                            const uint16_t vscp_type)
{
	//unsigned long t1 = (unsigned long)priority << 20;
	//unsigned long t2 = (unsigned long)pvscpMsg->vscp_class << 16;
	//unsigned long t3 = (unsigned long)pvscpMsg->vscp_type << 8;
	return( ((unsigned long) priority << 26) |
			((unsigned long) vscp_class << 16) |
			((unsigned long) vscp_type << 8) |
			0);
}

///////////////////////////////////////////////////////////////////////////////
// getCANCANALidFromVSCPevent
//

uint32_t vscp_getCANALidFromVSCPevent(const vscpEvent *pEvent)
{
	return( ((unsigned long) vscp_getVscpPriority(pEvent) << 26) |
			((unsigned long) pEvent->vscp_class << 16) |
			((unsigned long) pEvent->vscp_type << 8) |
			0);
}

///////////////////////////////////////////////////////////////////////////////
// getCANCANALidFromVSCPeventEx
//

uint32_t vscp_getCANALidFromVSCPeventEx(const vscpEventEx *pEvent)
{
	return( ((unsigned long) vscp_getVscpPriorityEx(pEvent) << 26) |
			((unsigned long) pEvent->vscp_class << 16) |
			((unsigned long) pEvent->vscp_type << 8) |
			0);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_calc_crc_Event
//
// Calculate CRC for Level 2 package. If bSet is true also write the
// crc into the packet.
//

unsigned short vscp_calc_crc_Event(vscpEvent *pEvent, short bSet)
{
	unsigned short crc = 0;
	unsigned char *p;
	unsigned char *pbuf;

	// Must be a valid message pointer
	if (NULL == pEvent) return 0;

	crcInit();

	pbuf = (unsigned char *) malloc(23 + pEvent->sizeData);

	if (NULL != pbuf) {

		p = pbuf;

		memcpy(p, (unsigned char *) &pEvent->head, 1);
		p++;

		memcpy(p, (unsigned char *) &pEvent->vscp_class, 2);
		p += 2;

		memcpy(p, (unsigned char *) &pEvent->vscp_type, 2);
		p += 2;

		memcpy(p, (unsigned char *) &pEvent->GUID, 16);
		p += 16;

		memcpy(p, pEvent->pdata, pEvent->sizeData);
		p += pEvent->sizeData;

		memcpy(p, (unsigned char *) &pEvent->sizeData, 2);
		p += 2;

		crc = crcFast(pbuf, sizeof( pbuf));

		if (bSet) pEvent->crc = crc;

		free(pbuf);
	}

	return crc;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_calc_crc_EventEx
//
// Calculate CRC for Level 2 package. If bSet is true also write the
// crc into the packet.
//

unsigned short vscp_calc_crc_EventEx(vscpEventEx *pEvent, short bSet)
{
	unsigned short crc = 0;
	unsigned char *p;
	unsigned char *pbuf;

	// Must be a valid message pointer
	if (NULL == pEvent) return 0;

	crcInit();

	pbuf = (unsigned char *) malloc(23 + pEvent->sizeData);

	if (NULL != pbuf) {

		p = pbuf;

		memcpy(p, (unsigned char *) &pEvent->head, 1);
		p++;

		memcpy(p, (unsigned char *) &pEvent->vscp_class, 2);
		p += 2;

		memcpy(p, (unsigned char *) &pEvent->vscp_type, 2);
		p += 2;

		memcpy(p, (unsigned char *) &pEvent->GUID, 16);
		p += 16;

		memcpy(p, pEvent->data, pEvent->sizeData);
		p += pEvent->sizeData;

		memcpy(p, (unsigned char *) &pEvent->sizeData, 2);
		p += 2;

		crc = crcFast(pbuf, sizeof( pbuf));

		if (bSet) pEvent->crc = crc;

		free(pbuf);
	}

	return crc;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_calcCRC4GUIDArray
//

uint8_t vscp_calcCRC4GUIDArray(const uint8_t *pguid)
{
    uint8_t crc=0;

    init_crc8();
    for ( int i=0; i<16; i++ ) {
        crc8( &crc, pguid[i] );
    }

    return crc;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_calcCRC4GUIDString
//

uint8_t vscp_calcCRC4GUIDString(const wxString &strguid)
{
    uint8_t crc=0;
    uint8_t guid[16];

    memset( guid, 0, 16 );
    vscp_getGuidFromStringToArray( guid, strguid );

    init_crc8();
    for ( int i=0; i<16; i++ ) {
        crc8( &crc, guid[i] );
    }

    return crc;
}


///////////////////////////////////////////////////////////////////////////////
// getGuidFromString
//

bool vscp_getGuidFromString(vscpEvent *pEvent, const wxString& strGUID)
{
	unsigned long val;

	// Check pointer
	if (NULL == pEvent) return false;

	if (0 == strGUID.Find(_("-"))) {
		memset(pEvent->GUID, 0, 16);
	} 
    else {
		wxStringTokenizer tkz(strGUID, wxT(":"));
		for (int i = 0; i < 16; i++) {
			tkz.GetNextToken().ToULong(&val, 16);
			pEvent->GUID[ i ] = (uint8_t) val;
			// If no tokens left no use to continue
			if (!tkz.HasMoreTokens()) break;
		}
	}

	return true;

}


///////////////////////////////////////////////////////////////////////////////
// getGuidFromStringEx
//

bool vscp_getGuidFromStringEx(vscpEventEx *pEvent, const wxString& strGUID)
{
	unsigned long val;

	// Check pointer
	if (NULL == pEvent) return false;

	if (0 == strGUID.Find(_("-"))) {
		memset(pEvent->GUID, 0, 16);
	} else {
		wxStringTokenizer tkz(strGUID, wxT(":"));
		for (int i = 0; i < 16; i++) {
			tkz.GetNextToken().ToULong(&val, 16);
			pEvent->GUID[ i ] = (uint8_t) val;
			// If no tokens left no use to continue
			if (!tkz.HasMoreTokens()) break;
		}
	}

	return true;

}


///////////////////////////////////////////////////////////////////////////////
// getGuidFromStringToArray
//

bool vscp_getGuidFromStringToArray(unsigned char *pGUID, const wxString& strGUID)
{
	unsigned long val;
    
    // If GUID is empty or "-" set all to zeror
    if ( ( 0 == strGUID.Length() ) || ( _("-") == strGUID ) ) {
        memset( pGUID, 0, 16 );
        return true;
    }


	wxStringTokenizer tkz(strGUID, wxT(":"));
	for (int i = 0; i < 16; i++) {
		tkz.GetNextToken().ToULong(&val, 16);
		pGUID[ i ] = (uint8_t) val;
		// If no tokens left no use to continue
		if (!tkz.HasMoreTokens()) break;
	}

	return true;

}

///////////////////////////////////////////////////////////////////////////////
// writeGuidToString
//

bool vscp_writeGuidToString(const vscpEvent *pEvent, wxString& strGUID)
{
	// Check pointer
	if (NULL == pEvent) return false;

	strGUID.Printf(_("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"),
			pEvent->GUID[0], pEvent->GUID[1], pEvent->GUID[2], pEvent->GUID[3],
			pEvent->GUID[4], pEvent->GUID[5], pEvent->GUID[6], pEvent->GUID[7],
			pEvent->GUID[8], pEvent->GUID[9], pEvent->GUID[10], pEvent->GUID[11],
			pEvent->GUID[12], pEvent->GUID[13], pEvent->GUID[14], pEvent->GUID[15]);

	return true;

}

///////////////////////////////////////////////////////////////////////////////
// writeGuidToStringEx
//

bool vscp_writeGuidToStringEx(const vscpEventEx *pEvent, wxString& strGUID)
{
	// Check pointer
	if (NULL == pEvent) return false;

	strGUID.Printf(_("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"),
			pEvent->GUID[0], pEvent->GUID[1], pEvent->GUID[2], pEvent->GUID[3],
			pEvent->GUID[4], pEvent->GUID[5], pEvent->GUID[6], pEvent->GUID[7],
			pEvent->GUID[8], pEvent->GUID[9], pEvent->GUID[10], pEvent->GUID[11],
			pEvent->GUID[12], pEvent->GUID[13], pEvent->GUID[14], pEvent->GUID[15]);

	return true;

}

///////////////////////////////////////////////////////////////////////////////
// writeGuidToString4Rows
//

bool vscp_writeGuidToString4Rows(const vscpEvent *pEvent, wxString& strGUID)
{
	// Check pointer
	if (NULL == pEvent) return false;

	strGUID.Printf(_("%02X:%02X:%02X:%02X\n%02X:%02X:%02X:%02X\n%02X:%02X:%02X:%02X\n%02X:%02X:%02X:%02X"),
			pEvent->GUID[0], pEvent->GUID[1], pEvent->GUID[2], pEvent->GUID[3],
			pEvent->GUID[4], pEvent->GUID[5], pEvent->GUID[6], pEvent->GUID[7],
			pEvent->GUID[8], pEvent->GUID[9], pEvent->GUID[10], pEvent->GUID[11],
			pEvent->GUID[12], pEvent->GUID[13], pEvent->GUID[14], pEvent->GUID[15]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeGuidToString4RowsEx
//

bool vscp_writeGuidToString4RowsEx(const vscpEventEx *pEvent, wxString& strGUID)
{
	// Check pointer
	if (NULL == pEvent) return false;

	strGUID.Printf(_("%02X:%02X:%02X:%02X\n%02X:%02X:%02X:%02X\n%02X:%02X:%02X:%02X\n%02X:%02X:%02X:%02X"),
			pEvent->GUID[0], pEvent->GUID[1], pEvent->GUID[2], pEvent->GUID[3],
			pEvent->GUID[4], pEvent->GUID[5], pEvent->GUID[6], pEvent->GUID[7],
			pEvent->GUID[8], pEvent->GUID[9], pEvent->GUID[10], pEvent->GUID[11],
			pEvent->GUID[12], pEvent->GUID[13], pEvent->GUID[14], pEvent->GUID[15]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeGuidToString
//

bool vscp_writeGuidArrayToString(const unsigned char *pGUID, wxString& strGUID)
{
	// Check pointer
	if (NULL == pGUID) return false;

	strGUID.Printf(_("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"),
			pGUID[0], pGUID[1], pGUID[2], pGUID[3],
			pGUID[4], pGUID[5], pGUID[6], pGUID[7],
			pGUID[8], pGUID[9], pGUID[10], pGUID[11],
			pGUID[12], pGUID[13], pGUID[14], pGUID[15]);

	return true;

}

///////////////////////////////////////////////////////////////////////////////
// isGUIDEmpty
//

bool vscp_isGUIDEmpty(const unsigned char *pGUID)
{
	// Check pointers
	if (NULL == pGUID) return false;

	return !(pGUID[0] + pGUID[1] + pGUID[2] + pGUID[3] +
			pGUID[4] + pGUID[5] + pGUID[6] + pGUID[7] +
			pGUID[8] + pGUID[9] + pGUID[10] + pGUID[11] +
			pGUID[12] + pGUID[13] + pGUID[14] + pGUID[15]);

}

//////////////////////////////////////////////////////////////////////////////
// isSameGUID
//

bool vscp_isSameGUID(const unsigned char *pGUID1, const unsigned char *pGUID2)
{
	// First check pointers
	if (NULL == pGUID1) return false;
	if (NULL == pGUID2) return false;

	if (0 != memcmp(pGUID1, pGUID2, 16)) return false;

	return true;

}

///////////////////////////////////////////////////////////////////////////////
// reverseGUID
//

bool vscp_reverseGUID(unsigned char *pGUID)
{
	uint8_t copyGUID[ 16 ];

	// First check pointers
	if (NULL == pGUID) return false;

	for (int i = 0; i < 16; i++) {
		copyGUID[ i ] = pGUID[ 15 - i ];
	}

	memcpy(pGUID, copyGUID, 16);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// convertVSCPtoEx

bool vscp_convertVSCPtoEx(vscpEventEx *pEventEx, const vscpEvent *pEvent)
{
	// Check pointers
	if (NULL == pEvent) return false;
	if (NULL == pEventEx) return false;

	if (pEvent->sizeData > VSCP_LEVEL2_MAXDATA) return false;

	// Convert
	pEventEx->crc = pEvent->crc;
	pEventEx->obid = pEvent->obid;
	pEventEx->timestamp = pEvent->timestamp;
	pEventEx->head = pEvent->head;
	pEventEx->obid = pEvent->obid;
	pEventEx->vscp_class = pEvent->vscp_class;
	pEventEx->vscp_type = pEvent->vscp_type;
	pEventEx->sizeData = pEvent->sizeData;

	memcpy(pEventEx->GUID, pEvent->GUID, 16);
	memcpy(pEventEx->data, pEvent->pdata, pEvent->sizeData);

	return true;

}

////////////////////////////////////////////////////////////////////////////////////
// convertVSCPfromEx
//

bool vscp_convertVSCPfromEx(vscpEvent *pEvent, const vscpEventEx *pEventEx)
{
	// Check pointers
	if ( NULL == pEvent ) return false;
	if ( NULL == pEventEx ) return false;
	
	if (pEventEx->sizeData > VSCP_LEVEL2_MAXDATA) return false;

	if (pEventEx->sizeData) {
		// Allocate memory for data
		if (NULL == (pEvent->pdata = new uint8_t[pEventEx->sizeData])) return false;
		memcpy(pEvent->pdata, pEventEx->data, pEventEx->sizeData);
	} else {
		// No data
		pEvent->pdata = NULL;
	}

	// Convert
	pEvent->crc = pEventEx->crc;
	pEvent->obid = pEventEx->obid;
	pEvent->timestamp = pEventEx->timestamp;
	pEvent->head = pEventEx->head;
	pEvent->obid = pEventEx->obid;
	pEvent->vscp_class = pEventEx->vscp_class;
	pEvent->vscp_type = pEventEx->vscp_type;
	pEvent->sizeData = pEventEx->sizeData;
	memcpy(pEvent->GUID, pEventEx->GUID, 16);

	return true;

}


////////////////////////////////////////////////////////////////////////////////////
// copyVSCPEvent

bool vscp_copyVSCPEvent(vscpEvent *pEventTo, const vscpEvent *pEventFrom)
{
	// Check pointers
	if (NULL == pEventTo) return false;
	if (NULL == pEventFrom) return false;

	if (pEventFrom->sizeData > VSCP_LEVEL2_MAXDATA) return false;

	// Convert
	pEventTo->crc = pEventFrom->crc;
	pEventTo->obid = pEventFrom->obid;
	pEventTo->timestamp = pEventFrom->timestamp;
	pEventTo->head = pEventFrom->head;
	pEventTo->obid = pEventFrom->obid;
	pEventTo->vscp_class = pEventFrom->vscp_class;
	pEventTo->vscp_type = pEventFrom->vscp_type;
	pEventTo->sizeData = pEventFrom->sizeData;

	memcpy(pEventTo->GUID, pEventFrom->GUID, 16);

	if (pEventFrom->sizeData) {
		pEventTo->pdata = new unsigned char[ pEventFrom->sizeData ];
		if (NULL != pEventTo->pdata) {
			memcpy(pEventTo->pdata, pEventFrom->pdata, pEventFrom->sizeData);
		}
	} else {
		pEventTo->pdata = NULL;
	}

	return true;

}

////////////////////////////////////////////////////////////////////////////////////
// deleteVSCPevent
//

void vscp_deleteVSCPevent(vscpEvent *pEvent)
{
	if (pEvent->sizeData) delete [] pEvent->pdata;
	delete pEvent;
}

////////////////////////////////////////////////////////////////////////////////////
// deleteVSCPevent
//

void vscp_deleteVSCPeventEx(vscpEventEx *pEventEx)
{
	delete pEventEx;
}

///////////////////////////////////////////////////////////////////////////////
// doLevel2Filter
//
//  filter ^ bit    mask    out
//  ============================
//        0          0       1    filter == bit, mask=don't care result = true
//        0          1       1    filter == bit, mask=valid, result = true
//        1          0       1    filter != bit, makse=don't care, result = true
//        1          1       0    filter != bit, mask=valid, result = false
//
// Mask tells *which* bits that are of interest means
// it always returns true if bit set to zero (0=don't care).
//
// Filter tells the value fo valid bits. If filter bit is == 1 the bits
// must be equal to get a true filter return.
//
// So a nill mask will let everything through
//

bool vscp_doLevel2Filter(const vscpEvent *pEvent,
		const vscpEventFilter *pFilter)
{
	// Must be a valid client
	if (NULL == pFilter) return false;

	// Must be a valid message
	if (NULL == pEvent) return false;

	// Test vscp_class
	if (0xffff != (uint16_t) (~(pFilter->filter_class ^ pEvent->vscp_class) |
			~pFilter->mask_class)) return false;

	// Test vscp_type
	if (0xffff != (uint16_t) (~(pFilter->filter_type ^ pEvent->vscp_type) |
			~pFilter->mask_type)) return false;

	// GUID
	for (int i = 0; i < 16; i++) {
		if (0xff != (uint8_t) (~(pFilter->filter_GUID[ i ] ^ pEvent->GUID[ i ]) |
				~pFilter->mask_GUID[ i ])) return false;
	}

	// Test priority
	if (0xff != (uint8_t) (~(pFilter->filter_priority ^ vscp_getVscpPriority(pEvent)) |
			~pFilter->mask_priority)) return false;

	return true;
}


bool vscp_doLevel2FilterEx( const vscpEventEx *pEventEx,
                        const vscpEventFilter *pFilter )
{
	// Must be a valid client
	if (NULL == pFilter) return false;

	// Must be a valid message
	if (NULL == pEventEx) return false;

	// Test vscp_class
	if (0xffff != (uint16_t) (~(pFilter->filter_class ^ pEventEx->vscp_class) |
			~pFilter->mask_class)) return false;

	// Test vscp_type
	if (0xffff != (uint16_t) (~(pFilter->filter_type ^ pEventEx->vscp_type) |
			~pFilter->mask_type)) return false;

	// GUID
	for (int i = 0; i < 16; i++) {
		if (0xff != (uint8_t) (~(pFilter->filter_GUID[ i ] ^ pEventEx->GUID[ i ]) |
				~pFilter->mask_GUID[ i ])) return false;
	}

	// Test priority
	if (0xff != (uint8_t) (~(pFilter->filter_priority ^ vscp_getVscpPriorityEx(pEventEx)) |
			~pFilter->mask_priority)) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// ClearVSCPFilter
//

void vscp_clearVSCPFilter(vscpEventFilter *pFilter)
{
	// Validate pointer
	if (NULL == pFilter) return;

	pFilter->filter_priority = 0x00;
	pFilter->mask_priority = 0x00;
	pFilter->filter_class = 0x00;
	pFilter->mask_class = 0x00;
	pFilter->filter_type = 0x00;
	pFilter->mask_type = 0x00;
	memset(pFilter->filter_GUID, 0x00, 16);
	memset(pFilter->mask_GUID, 0x00, 16);
}

//////////////////////////////////////////////////////////////////////////////
// readFilterFromString
//

bool vscp_readFilterFromString(vscpEventFilter *pFilter, wxString& strFilter)
{
	wxString strTok;

	// Check pointer
	if (NULL == pFilter) return false;

	wxStringTokenizer tkz(strFilter, _(","));

	// Get filter priority
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		pFilter->filter_priority = vscp_readStringValue(strTok);
	} else {
		return false;
	}

	// Get filter class
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		pFilter->filter_class = vscp_readStringValue(strTok);
	} else {
		return false;
	}

	// Get filter type
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		pFilter->filter_type = vscp_readStringValue(strTok);
	} else {
		return false;
	}

	// Get filter GUID
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		vscp_getGuidFromStringToArray(pFilter->filter_GUID,
				strTok);
	} else {
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// readMaskFromString
//

bool vscp_readMaskFromString(vscpEventFilter *pFilter, wxString& strFilter)
{
	wxString strTok;

	// Check pointer
	if (NULL == pFilter) return false;

	wxStringTokenizer tkz(strFilter, _(","));

	// Get filter priority
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		pFilter->mask_priority = vscp_readStringValue(strTok);
	} else {
		return false;
	}

	// Get filter class
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		pFilter->mask_class = vscp_readStringValue(strTok);
	} else {
		return false;
	}

	// Get filter type
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		pFilter->mask_type = vscp_readStringValue(strTok);
	} else {
		return false;
	}

	// Get filter GUID
	if (tkz.HasMoreTokens()) {
		strTok = tkz.GetNextToken();
		vscp_getGuidFromStringToArray(pFilter->mask_GUID,
				strTok);
	} else {
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// convertCanalToEvent
//

bool vscp_convertCanalToEvent(vscpEvent *pvscpEvent,
		                         const canalMsg *pcanalMsg,
		                         unsigned char *pGUID )
{
	// Must be valid pointers
	if (NULL == pGUID) return false;
	if (NULL == pcanalMsg) return false;
	if (NULL == pvscpEvent) return false;

	// Copy in i/f GUID
	memcpy(pvscpEvent->GUID, pGUID, 16);

	pvscpEvent->head = 0;

    if ( pcanalMsg->sizeData > 8 ) {
        pcanalMsg->sizeData;
    }

	if (pcanalMsg->sizeData > 0) {

		// Allocate storage for data
		pvscpEvent->pdata = new uint8_t[ pcanalMsg->sizeData ];

		if (NULL != pvscpEvent->pdata) {
			// Assign size (max 8 bytes it's CAN... )
			pvscpEvent->sizeData = pcanalMsg->sizeData;
			memcpy(pvscpEvent->pdata, pcanalMsg->data, pcanalMsg->sizeData);
		} 
        else {
			pvscpEvent->sizeData = 0;
		}
	} 
    else {
		pvscpEvent->pdata = NULL;
		pvscpEvent->sizeData = 0;
	}

	// Build ID
	pvscpEvent->head = vscp_getVSCPheadFromCANALid(pcanalMsg->id);
	if (pcanalMsg->id & 0x02000000) pvscpEvent->head |= VSCP_HEADER_HARD_CODED;
	pvscpEvent->vscp_class = vscp_getVSCPclassFromCANALid(pcanalMsg->id);
	pvscpEvent->vscp_type = vscp_getVSCPtypeFromCANALid(pcanalMsg->id);

	// Timestamp
	pvscpEvent->timestamp = pcanalMsg->timestamp;

	// Set nickname id
	pvscpEvent->GUID[ 15 ] = (unsigned char) (0xff & pcanalMsg->id);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// convertCanalToEvent
//

bool vscp_convertCanalToEventEx(vscpEventEx *pvscpEventEx,
		const canalMsg *pcanalMsg,
		unsigned char *pGUID )
{
	vscpEvent *pEvent = new vscpEvent;
	bool rv = vscp_convertCanalToEvent(pEvent,
                                         pcanalMsg,
                                         pGUID );
	
	if ( rv ) {	
		vscp_convertVSCPtoEx(pvscpEventEx, pEvent );
		vscp_deleteVSCPevent(pEvent);
	}

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// convertEventToCanal
//

bool vscp_convertEventToCanal(canalMsg *pcanalMsg, const vscpEvent *pvscpEvent)
{
	unsigned char nodeid = 0;
	short sizeData = 0;
	uint16_t vscp_class = 0;

	if (NULL == pcanalMsg) return false;
	if (NULL == pvscpEvent) return false;

    sizeData = pvscpEvent->sizeData;
    vscp_class = pvscpEvent->vscp_class;
    
	pcanalMsg->obid = pvscpEvent->obid;
	pcanalMsg->flags = 0;

	// Level II events with class == 512 is recognized by the daemon and
	// sent to the correct interface as Level I events if the interface
	// is addressed by the client.
	if ((512 == pvscpEvent->vscp_class) && (pvscpEvent->sizeData >= 16)) {

		nodeid = pvscpEvent->pdata[ 15 ]; // Save the nodeid

		// We must translate the data part of the event to standard format
		sizeData = pvscpEvent->sizeData - 16;
		memcpy(pvscpEvent->pdata, pvscpEvent->pdata + 16, sizeData);

		vscp_class = pvscpEvent->vscp_class - 512;
	}

	// Always extended
	pcanalMsg->flags = CANAL_IDFLAG_EXTENDED;

	if (sizeData <= 8) {
		pcanalMsg->sizeData = (unsigned char) sizeData;
	} else {
		pcanalMsg->sizeData = 8;
	}

	unsigned char priority = ((pvscpEvent->head & VSCP_MASK_PRIORITY) >> 5);

	//unsigned long t1 = (unsigned long)priority << 20;
	//unsigned long t2 = (unsigned long)pvscpMsg->vscp_class << 16;
	//unsigned long t3 = (unsigned long)pvscpMsg->vscp_type << 8;
	pcanalMsg->id = ((unsigned long) priority << 26) |
			((unsigned long) vscp_class << 16) |
			((unsigned long) pvscpEvent->vscp_type << 8) |
			nodeid; // Normally we are the host of hosts
	// but for class=512 events nodeid
	// is present in GUID LSB

	if (pvscpEvent->head & VSCP_HEADER_HARD_CODED) {
		pcanalMsg->id |= VSCP_CAN_ID_HARD_CODED;
	}

	if (NULL != pvscpEvent->pdata) {
		memcpy(pcanalMsg->data, pvscpEvent->pdata, pcanalMsg->sizeData);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// convertEventExToCanal
//

bool vscp_convertEventExToCanal(canalMsg *pcanalMsg, const vscpEventEx *pvscpEventEx)
{
	bool rv;

	if (NULL == pcanalMsg) return false;
	if (NULL == pvscpEventEx) return false;

	vscpEvent *pEvent = new vscpEvent();
	if (NULL == pEvent) return false;

	if (!vscp_convertVSCPfromEx(pEvent, pvscpEventEx)) {
		vscp_deleteVSCPevent(pEvent);
		return false;
	}

	rv = vscp_convertEventToCanal(pcanalMsg, pEvent);

	vscp_deleteVSCPevent(pEvent);

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeVscpDataToString
//

bool vscp_writeVscpDataToString(const vscpEvent *pEvent, wxString& str, bool bUseHtmlBreak)
{
	wxString wrk, strBreak;

	// Check pointers
	if (NULL == pEvent->pdata) return false;

	str.Empty();

	if (bUseHtmlBreak) {
		strBreak = _("<br>");
	} else {
		strBreak = _("\r\n");
	}

	for (int i = 0; i < pEvent->sizeData; i++) {

		wrk.Printf(_("0x%02X"), pEvent->pdata[i]);

		if (i < (pEvent->sizeData - 1)) {
			wrk += _(",");
		}

		if (!((i + 1) % 8)) wrk += strBreak;
		str += wrk;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// writeVscpDataWithSizeToString
//

bool vscp_writeVscpDataWithSizeToString(const uint16_t sizeData,
                                            const unsigned char *pData,
                                            wxString& str,
                                            bool bUseHtmlBreak,
                                            bool bBreak )
{
	wxString wrk, strBreak;

	// Check pointers
	if (NULL == pData) return false;

	str.Empty();

	if (bUseHtmlBreak) {
		strBreak = _("<br>");
	} else {
		strBreak = _("\r\n");
	}

	for (int i = 0; i < sizeData; i++) {

		wrk.Printf(_("0x%02X"), pData[i]);

		if (i < (sizeData - 1)) {
			wrk += _(",");
		}

        if ( bBreak ) {
		    if (!((i + 1) % 8)) wrk += strBreak;
        }
		str += wrk;
	}

	return true;
}




//////////////////////////////////////////////////////////////////////////////
// setVscpDataFromString
//

bool vscp_setVscpDataFromString(vscpEvent *pEvent, const wxString& str)
{
	// Check pointers
	if (NULL == pEvent) return false;

	wxStringTokenizer tkz(str, _(","));

	uint8_t data[ VSCP_MAX_DATA ];
	pEvent->sizeData = 0;
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken();
		data[ pEvent->sizeData ] = vscp_readStringValue(token);
		pEvent->sizeData++;
		if (pEvent->sizeData >= VSCP_MAX_DATA) break;
	}

	if (pEvent->sizeData > 0) {
		pEvent->pdata = new uint8_t[pEvent->sizeData];
		if (NULL != pEvent->pdata) {
			memcpy(pEvent->pdata, &data, pEvent->sizeData);
		}
	} else {
		pEvent->pdata = NULL;
	}

	return true;

}

//////////////////////////////////////////////////////////////////////////////
// setVscpDataArrayFromString
//

bool vscp_setVscpDataArrayFromString(uint8_t *pData, 
                                        uint16_t *psizeData, 
                                        const wxString& str)
{
	// Check pointers
	if (NULL == pData) return false;
	if (NULL == psizeData) return false;

	*psizeData = 0;
	wxStringTokenizer tkz(str, _(","));

	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken();
		pData[ *psizeData ] = vscp_readStringValue( token );
		(*psizeData)++;
		if (*psizeData >= VSCP_MAX_DATA) break;
	}

	return true;
    
}

////////////////////////////////////////////////////////////////////////////////////
// makeTimeStamp
//

unsigned long vscp_makeTimeStamp(void)
{
#ifdef WIN32
	return GetTickCount();
#else
	tms tm;
	return times(&tm);
#endif
}

////////////////////////////////////////////////////////////////////////////////////
// writeVscpEventToString
//
// head,class,type,obid,timestamp,GUID,data1,data2,data3....
//

bool vscp_writeVscpEventToString( const vscpEvent *pEvent, wxString& str)
{
	// Check pointer
	if (NULL == pEvent) return false;

	str.Printf(_("%d,%d,%d,%d,%d,"),
			pEvent->head,
			pEvent->vscp_class,
			pEvent->vscp_type,
            pEvent->obid,
            pEvent->timestamp );

	wxString strGUID;
	vscp_writeGuidToString(pEvent, strGUID);
	str += strGUID;
	if (pEvent->sizeData) {
		str += wxT(",");

		wxString strData;
		vscp_writeVscpDataToString(pEvent, strData);
		str += strData;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// writeVscpEventExToString
//
// head,class,type,obid,timestamp,GUID,data1,data2,data3....
//

bool vscp_writeVscpEventExToString( const vscpEventEx *pEventEx, wxString& str)
{
	vscpEvent Event;
    Event.pdata = NULL;

	// Check pointer
	if (NULL == pEventEx) return false;

	Event.pdata = NULL;
	vscp_convertVSCPfromEx(&Event, pEventEx);
	vscp_writeVscpEventToString(&Event, str);
	if ( NULL != Event.pdata ) delete Event.pdata;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// setVscpEventFromString
//
// Format: 
//		head,class,type,obid,timestamp,GUID,data1,data2,data3....
//

bool vscp_setVscpEventFromString(vscpEvent *pEvent, const wxString& strEvent)
{
	// Check pointer
	if (NULL == pEvent) {
		return false;
	}

	wxString str = strEvent;

	wxStringTokenizer tkz(str, _(","));

	// Get head
	if (tkz.HasMoreTokens()) {
		str = tkz.GetNextToken();
		pEvent->head = vscp_readStringValue(str);
	} else {
		return false;
	}

	// Get Class
	if (tkz.HasMoreTokens()) {
		str = tkz.GetNextToken();
		pEvent->vscp_class = vscp_readStringValue(str);
	} else {
		return false;
	}


	// Get Type
	if (tkz.HasMoreTokens()) {
		str = tkz.GetNextToken();
		pEvent->vscp_type = vscp_readStringValue(str);
	} else {
		return false;
	}

	// Get OBID  -  Kept here to be compatible with receive
	if (tkz.HasMoreTokens()) {
		str = tkz.GetNextToken();
		pEvent->obid = vscp_readStringValue(str);
	} else {
		return false;
	}

	// Get Timestamp
	if (tkz.HasMoreTokens()) {
		str = tkz.GetNextToken();
		pEvent->timestamp = vscp_readStringValue(str);
		if (!pEvent->timestamp) {
#ifdef WIN32
			pEvent->timestamp = GetTickCount();
#else
			pEvent->timestamp = 0; // TODO
#endif
		}
	} else {
		return false;
	}

	// Get GUID
	wxString strGUID;
	if (tkz.HasMoreTokens()) {
		strGUID = tkz.GetNextToken();
		vscp_getGuidFromString(pEvent, strGUID);
	} else {
		return false;
	}

	// Handle data
	pEvent->sizeData = 0;
	char data[ 512 ];
	while (tkz.HasMoreTokens()) {
		str = tkz.GetNextToken();
		data[ pEvent->sizeData ] = vscp_readStringValue(str);
		pEvent->sizeData++;
	}

	// OK add in the data
	if (pEvent->sizeData) {
		uint8_t *pData = new uint8_t[ pEvent->sizeData ];
		if (NULL != pData) {
			memcpy(pData, data, pEvent->sizeData);
			pEvent->pdata = pData;
		}
	} else {
		pEvent->pdata = NULL;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// setVscpEventExFromString
//
// Format: 
//		head,class,type,obid,timestamp,GUID,data1,data2,data3....
//

bool vscp_setVscpEventExFromString(vscpEventEx *pEventEx, const wxString& strEvent)
{
	bool rv;
	vscpEvent event;

	// Parse the string data
	rv = vscp_setVscpEventFromString(&event, strEvent);
	vscp_convertVSCPtoEx(pEventEx, &event);

	// Remove possible data
	if (event.sizeData) delete [] event.pdata;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// makeHtml
//
//

void vscp_makeHtml(wxString& str)
{
	wxString strOriginal = str;

	str.Empty();
	for (uint32_t i = 0; i < strOriginal.Len(); i++) {
		if (0x0a == (unsigned char) strOriginal.GetChar(i)) {
			str += _("<br>");
		} else {
			str += strOriginal.GetChar(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// getDeviceHtmlStatusInfo
//

wxString &vscp_getDeviceHtmlStatusInfo(const uint8_t *registers, CMDF *pmdf)
{
    static wxString strHTML;
    wxString str;

    strHTML = _("<html><body>");
    strHTML += _("<h4>Clear text node data</h4>");
    strHTML += _("<font color=\"#009900\">");

    strHTML += _("nodeid = ");
    str = wxString::Format(_("%d"), registers[0x91]);
    strHTML += str;
    strHTML += _("<br>");

    strHTML += _("GUID = ");
    vscp_writeGuidArrayToString(registers + 0xd0, str);
    strHTML += str;
    strHTML += _("<br>");

    strHTML += _("MDF URL = ");
    char url[33];
    memset(url, 0, sizeof( url));
    memcpy(url, registers + 0xe0, 32);
    str = wxString::From8BitData(url);
    strHTML += str;
    strHTML += _("<br>");

    strHTML += _("Alarm: ");
    if (registers[0x80]) {
        strHTML += _("Yes");
    } else {
        strHTML += _("No");
    }
    strHTML += _("<br>");


    strHTML += _("Node Control Flags: ");
    if (registers[0x83] & 0x10) {
        strHTML += _("[Register Write Protect] ");
    } else {
        strHTML += _("[Register Read/Write] ");
    }
    switch ((registers[0x83] & 0xC0) >> 6) {
    case 1:
        strHTML += _(" [Initialized] ");
        break;
    default:
        strHTML += _(" [Uninitialized] ");
        break;
    }
    strHTML += _("<br>");

    strHTML += _("Firmware VSCP conformance : ");
    strHTML += wxString::Format(_("%d.%d"), registers[0x81], registers[0x82]);
    strHTML += _("<br>");

    strHTML += _("User ID: ");
    strHTML += wxString::Format(_("%d.%d.%d.%d.%d"),
            registers[0x84],
            registers[0x85],
            registers[0x86],
            registers[0x87],
            registers[0x88]);
    strHTML += _("<br>");

    strHTML += _("Manufacturer device ID: ");
    strHTML += wxString::Format(_("%d.%d.%d.%d"),
            registers[0x89],
            registers[0x8A],
            registers[0x8B],
            registers[0x8C]);
    strHTML += _("<br>");

    strHTML += _("Manufacturer sub device ID: ");
    strHTML += wxString::Format(_("%d.%d.%d.%d"),
            registers[0x8d],
            registers[0x8e],
            registers[0x8f],
            registers[0x90]);
    strHTML += _("<br>");

    strHTML += _("Page select: ");
    strHTML += wxString::Format(_("%d (MSB=%d LSB=%d)"),
            registers[0x92] * 256 + registers[0x93],
            registers[0x92],
            registers[0x93]);
    strHTML += _("<br>");

    strHTML += _("Firmware version: ");
    strHTML += wxString::Format(_("%d.%d.%d"),
            registers[0x94],
            registers[0x95],
            registers[0x96]);
    strHTML += _("<br>");

    strHTML += _("Boot loader algorithm: ");
    strHTML += wxString::Format(_("%d - "),
            registers[0x97]);
    switch (registers[0x97]) {

    case 0x00:
        strHTML += _("VSCP universal algorithm 0");
        break;

    case 0x01:
        strHTML += _("Microchip PIC algorithm 0");
        break;

    case 0x10:
        strHTML += _("Atmel AVR algorithm 0");
        break;

    case 0x20:
        strHTML += _("NXP ARM algorithm 0");
        break;

    case 0x30:
        strHTML += _("ST ARM algorithm 0");
        break;

    default:
        strHTML += _("Unknown algorithm.");
        break;
    }

    strHTML += _("<br>");

    strHTML += _("Buffer size: ");
    strHTML += wxString::Format(_("%d bytes. "),
            registers[0x98]);
    if (!registers[0x98]) strHTML += _(" ( == default size (8 or 487 bytes) )");
    strHTML += _("<br>");

    strHTML += _("Number of register pages: ");
    strHTML += wxString::Format(_("%d"),
            registers[0x99]);
    if (registers[0x99] > 22) {
        strHTML += _(" (Note: VSCP Works display max 22 pages.) ");
    }
    strHTML += _("<br>");

    // Decision matrix info.
    if (NULL != pmdf) {

        unsigned char data[8];
        memset(data, 0, 8);

        strHTML += _("Decison Matrix: Rows=");
        strHTML += wxString::Format(_("%d "), pmdf->m_dmInfo.m_nRowCount);
        strHTML += _(" Offset=");
        strHTML += wxString::Format(_("%d "), pmdf->m_dmInfo.m_nStartOffset);
        strHTML += _(" Page start=");
        strHTML += wxString::Format(_("%d "), pmdf->m_dmInfo.m_nStartPage);
        strHTML += _(" Row Size=");
        strHTML += wxString::Format(_("%d "), pmdf->m_dmInfo.m_nRowSize);
        strHTML += _(" Level=");
        strHTML += wxString::Format(_("%d "), pmdf->m_dmInfo.m_nLevel);
        strHTML += _(" # actions define =");
        strHTML += wxString::Format(_("%d "), pmdf->m_dmInfo.m_list_action.GetCount());
        strHTML += _("<br>");
    } else {
        strHTML += _("No Decision Matrix is available on this device.");
        strHTML += _("<br>");
    }


    if (NULL != pmdf) {

        // MDF Info
        strHTML += _("<h1>MDF Information</h1>");

        strHTML += _("<font color=\"#009900\">");

        // Manufacturer data
        strHTML += _("<b>Module name :</b> ");
        strHTML += pmdf->m_strModule_Name;
        strHTML += _("<br>");

        strHTML += _("<b>Module model:</b> ");
        strHTML += pmdf->m_strModule_Model;
        strHTML += _("<br>");

        strHTML += _("<b>Module version:</b> ");
        strHTML += pmdf->m_strModule_Version;
        strHTML += _("<br>");

        strHTML += _("<b>Module last change:</b> ");
        strHTML += pmdf->m_changeDate;
        strHTML += _("<br>");

        strHTML += _("<b>Module description:</b> ");
        strHTML += pmdf->m_strModule_Description;
        strHTML += _("<br>");

        strHTML += _("<b>Module URL</b> : ");
        strHTML += _("<a href=\"");
        strHTML += pmdf->m_strModule_InfoURL;
        strHTML += _("\">");
        strHTML += pmdf->m_strModule_InfoURL;
        strHTML += _("</a>");
        strHTML += _("<br>");


        MDF_MANUFACTURER_LIST::iterator iter;
        for (iter = pmdf->m_list_manufacturer.begin();
                iter != pmdf->m_list_manufacturer.end(); ++iter) {

            strHTML += _("<hr><br>");

            CMDF_Manufacturer *manufacturer = *iter;
            strHTML += _("<b>Manufacturer:</b> ");
            strHTML += manufacturer->m_strName;
            strHTML += _("<br>");

            MDF_ADDRESS_LIST::iterator iterAddr;
            for (iterAddr = manufacturer->m_list_Address.begin();
                    iterAddr != manufacturer->m_list_Address.end(); ++iterAddr) {

                CMDF_Address *address = *iterAddr;
                strHTML += _("<h4>Address</h4>");
                strHTML += _("<b>Street:</b> ");
                strHTML += address->m_strStreet;
                strHTML += _("<br>");
                strHTML += _("<b>Town:</b> ");
                strHTML += address->m_strTown;
                strHTML += _("<br>");
                strHTML += _("<b>City:</b> ");
                strHTML += address->m_strCity;
                strHTML += _("<br>");
                strHTML += _("<b>Post Code:</b> ");
                strHTML += address->m_strPostCode;
                strHTML += _("<br>");
                strHTML += _("<b>State:</b> ");
                strHTML += address->m_strState;
                strHTML += _("<br>");
                strHTML += _("<b>Region:</b> ");
                strHTML += address->m_strRegion;
                strHTML += _("<br>");
                strHTML += _("<b>Country:</b> ");
                strHTML += address->m_strCountry;
                strHTML += _("<br><br>");
            }

            MDF_ITEM_LIST::iterator iterPhone;
            for (iterPhone = manufacturer->m_list_Phone.begin();
                    iterPhone != manufacturer->m_list_Phone.end(); ++iterPhone) {

                CMDF_Item *phone = *iterPhone;
                strHTML += _("<b>Phone:</b> ");
                strHTML += phone->m_strItem;
                strHTML += _(" ");
                strHTML += phone->m_strDescription;
                strHTML += _("<br>");
            }

            MDF_ITEM_LIST::iterator iterFax;
            for (iterFax = manufacturer->m_list_Fax.begin();
                    iterFax != manufacturer->m_list_Fax.end(); ++iterFax) {

                CMDF_Item *fax = *iterFax;
                strHTML += _("<b>Fax:</b> ");
                strHTML += fax->m_strItem;
                strHTML += _(" ");
                strHTML += fax->m_strDescription;
                strHTML += _("<br>");
            }

            MDF_ITEM_LIST::iterator iterEmail;
            for (iterEmail = manufacturer->m_list_Email.begin();
                    iterEmail != manufacturer->m_list_Email.end(); ++iterEmail) {

                CMDF_Item *email = *iterEmail;
                strHTML += _("<b>Email:</b> <a href=\"");
                strHTML += email->m_strItem;
                strHTML += _("\" >");
                strHTML += email->m_strItem;
                strHTML += _("</a> ");
                strHTML += email->m_strDescription;
                strHTML += _("<br>");
            }

            MDF_ITEM_LIST::iterator iterWeb;
            for (iterWeb = manufacturer->m_list_Web.begin();
                    iterWeb != manufacturer->m_list_Web.end(); ++iterWeb) {

                CMDF_Item *web = *iterWeb;
                strHTML += _("<b>Web:</b> <a href=\"");
                strHTML += web->m_strItem;
                strHTML += _("\">");
                strHTML += web->m_strItem;
                strHTML += _("</a> ");
                strHTML += web->m_strDescription;
                strHTML += _("<br>");
            }

        } // manufacturer

    } else {
        strHTML += _("No MDF info available.");
        strHTML += _("<br>");
    }


    strHTML += _("</font>");
    strHTML += _("</body></html>");

    return strHTML;
}


///////////////////////////////////////////////////////////////////////////////
// getRealTextData
//
//

wxString& vscp_getRealTextData(vscpEvent *pEvent)
{
	int offset=0;
	static wxString str;
	wxString wrkstr1, wrkstr2, wrkstr3;
	int i;

	str.Empty();

	// Check pointer
	if (NULL == pEvent) return str;
	
		// If class >= 512 and class <1024 we
	// have GUID in front of data. 
	if ( ( pEvent->vscp_class >= VSCP_CLASS2_LEVEL1_PROTOCOL) && 
			(pEvent->vscp_class < VSCP_CLASS2_PROTOCOL) ) {
		offset = 16;
	}

	switch (pEvent->vscp_class-(offset?512:0)) {

		// **** CLASS ****
	case VSCP_CLASS1_PROTOCOL:
	{
		switch (pEvent->vscp_type) {

		case VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT:
			break;

		case VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE:
			break;

		case VSCP_TYPE_PROTOCOL_PROBE_ACK:
			break;

		case VSCP_TYPE_PROTOCOL_SET_NICKNAME:
			break;

		case VSCP_TYPE_PROTOCOL_NICKNAME_ACCEPTED:
			break;

		case VSCP_TYPE_PROTOCOL_DROP_NICKNAME:
			break;

		case VSCP_TYPE_PROTOCOL_READ_REGISTER:
			break;

		case VSCP_TYPE_PROTOCOL_RW_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_WRITE_REGISTER:
			break;

		case VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER:
			break;

		case VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER:
			break;

		case VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER:
			break;

		case VSCP_TYPE_PROTOCOL_START_BLOCK:
			break;

		case VSCP_TYPE_PROTOCOL_BLOCK_DATA:
			break;

		case VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK:
			break;

		case VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK:
			break;

		case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA:
			break;

		case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK:
			break;

		case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK:
			break;

		case VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE:
			break;

		case VSCP_TYPE_PROTOCOL_RESET_DEVICE:
			break;

		case VSCP_TYPE_PROTOCOL_PAGE_READ:
			break;

		case VSCP_TYPE_PROTOCOL_PAGE_WRITE:
			break;

		case VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_HIGH_END_SERVER_PROBE:
			break;

		case VSCP_TYPE_PROTOCOL_HIGH_END_SERVER_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_INCREMENT_REGISTER:
			break;

		case VSCP_TYPE_PROTOCOL_DECREMENT_REGISTER:
			break;

		case VSCP_TYPE_PROTOCOL_WHO_IS_THERE:
			break;

		case VSCP_TYPE_PROTOCOL_WHO_IS_THERE_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO:
			break;

		case VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_GET_EMBEDDED_MDF:
			break;

		case VSCP_TYPE_PROTOCOL_GET_EMBEDDED_MDF_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ:
			break;

		case VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE:
			break;

		case VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_GET_EVENT_INTEREST:
			break;

		case VSCP_TYPE_PROTOCOL_GET_EVENT_INTEREST_RESPONSE:
			break;

		case VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE_ACK:
			break;

		case VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE_NACK:
			break;

		case VSCP_TYPE_PROTOCOL_START_BLOCK_ACK:
			break;

		case VSCP_TYPE_PROTOCOL_START_BLOCK_NACK:
			break;
		}
	}
		break;

		// **** CLASS ****
	case VSCP_CLASS1_ALARM:
	{

		switch (pEvent->vscp_type) {

		case VSCP_TYPE_ALARM_WARNING:
			break;

		case VSCP_TYPE_ALARM_ALARM:
			break;

		case VSCP_TYPE_ALARM_SOUND:
			break;

		case VSCP_TYPE_ALARM_LIGHT:
			break;

		case VSCP_TYPE_ALARM_POWER:
			break;

		case VSCP_TYPE_ALARM_EMERGENCY_STOP:
			break;

		case VSCP_TYPE_ALARM_EMERGENCY_PAUSE:
			break;

		case VSCP_TYPE_ALARM_EMERGENCY_RESET:
			break;

		case VSCP_TYPE_ALARM_EMERGENCY_RESUME:
			break;

		}
	}
		break;


		// **** CLASS ****
	case VSCP_CLASS1_SECURITY:
	{
		switch (pEvent->vscp_type) {
		case VSCP_TYPE_SECURITY_MOTION:
			break;
		}
	}
		break;

		// **** CLASS ****
	case VSCP_CLASS1_MEASUREMENT:
	{
		// the sensor number can be 0...7 and equals bits 2,1,0 of datacoding byte
		str = wxString::Format(_("Sensor %d "), VSCP_DATACODING_INDEX(*(pEvent->pdata+offset)));

		// disassemble the data bytes to clear text depending on the datacoding
		// this is the same for all CLASS1_MEASUREMENT events. The only thing thats
		// different is the unit that follows!
		// for debugging we put out in [] the type of data, and its value
		// after the =
		// this works on bits 7,6,5 of the datacoding byte
		switch (VSCP_DATACODING_TYPE(*(pEvent->pdata+offset)) & 0xE0) {

		case 0x00: // bit format
		{
			str += _("[bit] = ");
			//char sth[8];
			str += wxString::Format(wxT("%X"), 
					(long) vscp_getDataCodingBitArray(pEvent->pdata+offset, 
					pEvent->sizeData-offset));
		}
			break;
		case 0x20: // byte format
            vscp_getVSCPMeasurementAsString( pEvent, wrkstr1 );
			str += _("[byte] = ?");
            str += wrkstr1;
			break;
		case 0x40: // string format
		{
			str += _("[string] = ");
            wxString wxstr;
			vscp_getDataCodingString(pEvent->pdata+offset, 
										pEvent->sizeData-offset,
                                        wxstr );
            str += wxstr;
		}
			break;
		case 0x60: // int format
            vscp_getVSCPMeasurementAsString( pEvent, wrkstr1 );
			str += _("[int] = ");
            str += wrkstr1;
			break;
		case 0x80: // normalized int format
		{
			double temp = 
				vscp_getDataCodingNormalizedInteger(pEvent->pdata+offset, 
												pEvent->sizeData-offset);
			str += wxString::Format(_("[nint] = %f "), temp);
		}
			break;
		case 0xA0: // float format
			if ( (pEvent->sizeData-offset) >= 5 ) {
				float msrmt = vscp_getMeasurementAsFloat(pEvent->pdata+offset, 
						pEvent->sizeData-offset);
				str += wxString::Format(_("[float] = %g "), msrmt);
			}
			else {
				str += _("[float] = invalid event format ");
			}
			break;
		}

		// here we put out the unit depending on each measurement type's own
		// definition which unit to use (this is bits 4,3 of datacoding byte)
		switch (pEvent->vscp_type) {

		case VSCP_TYPE_MEASUREMENT_COUNT:
		{
		}
			break;

		case VSCP_TYPE_MEASUREMENT_LENGTH:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = meter
				str += _("m");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_MASS:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = kilogram
				str += _("Kg");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_TIME:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = millisecond
				str += _("ms");
				break;
			case 0x01: // unit = second
				str += _("s");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRIC_CURRENT:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = ampere
				str += _("A");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_TEMPERATURE:
		{

			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {

			case 0x00: // default unit = kelvin
				str += _("K");
				break;

			case 0x01: // Celsius
				str += _("C");
				break;

			case 0x02: // Farenheit
				str += _("F");
				break;
			}

		}
			break;

		case VSCP_TYPE_MEASUREMENT_AMOUNT_OF_SUBSTANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = mole
				str += _("mol");
				break;
			}

		}
			break;

		case VSCP_TYPE_MEASUREMENT_INTENSITY_OF_LIGHT:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = candela
				str += _("cd");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_FREQUENCY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = hertz
				str += _("Hz");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_RADIOACTIVITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = bequerel
				str += _("Bq");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_FORCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = newton
				str += _("N");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_PRESSURE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = pascal
				str += _("pa");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ENERGY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = joule
				str += _("J");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_POWER:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = watt
				str += _("W");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRICAL_CHARGE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = Coulomb
				str += _("C");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRICAL_POTENTIAL:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = volt
				str += _("V");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRICAL_CAPACITANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = Fahrad
				str += _("F");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRICAL_RECISTANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = ohms
				str += _("Ohm");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRICAL_CONDUCTANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = siemens
				str += _("S");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_MAGNETIC_FIELD_STRENGTH:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = ampere meters
				str += _("Am");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_MAGNETIC_FLUX:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = weber
				str += _("Wb");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_MAGNETIC_FLUX_DENSITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = tesla
				str += _("T");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_INDUCTANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = henry
				str += _("H");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_FLUX_OF_LIGHT:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = lumen
				str += _("lm");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ILLUMINANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = lux
				str += _("lx");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_RADIATION_DOSE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = gray
				str += _("Gy");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_CATALYTIC_ACITIVITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = katal
				str += _("kat");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_VOLUME:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = qubic meters
				str += _("qm");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_SOUND_INTENSITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = bel
				str += _("bel");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ANGLE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = radian
				str += _("rad");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_POSITION:
		{

		}
			break;

		case VSCP_TYPE_MEASUREMENT_SPEED:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = meters / second
				str += _("m/s");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ACCELERATION:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = meters / sqaresecond
				str += _("m/s^2");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_TENSION:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = newton / meter
				str += _("N/m");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_HUMIDITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = relative humity
				str += _("%");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_FLOW:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = qubicmeter / second
				str += _("m^3/s");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_THERMAL_RESISTANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = kelvin / watt
				str += _("K/W");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_REFRACTIVE_POWER:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = diopter
				str += _("dpt");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_DYNAMIC_VISCOSITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = Pascal seconds
				str += _("Pa*s");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_SOUND_IMPEDANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = Newton * second / qubicmeter
				str += _("N*s/m^3");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_SOUND_RESISTANCE:
		{
		}
			break;

		case VSCP_TYPE_MEASUREMENT_ELECTRIC_ELASTANCE:
		{

		}
			break;

		case VSCP_TYPE_MEASUREMENT_LUMINOUS_ENERGY:
		{

		}
			break;

		case VSCP_TYPE_MEASUREMENT_LUMINANCE:
		{

		}
			break;

		case VSCP_TYPE_MEASUREMENT_CHEMICAL_CONCENTRATION:
		{

		}
			break;

			/* // this type (= 46) has become "reserved"
			case VSCP_TYPE_MEASUREMENT_ABSORBED_DOSE:
			{

			}
			break;
			 */

		case VSCP_TYPE_MEASUREMENT_DOSE_EQVIVALENT:
		{

		}
			break;


		case VSCP_TYPE_MEASUREMENT_DEWPOINT:
		{

		}
			break;

		case VSCP_TYPE_MEASUREMENT_RELATIVE_LEVEL:
		{

		}
			break;

		case VSCP_TYPE_MEASUREMENT_ALTITUDE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = meter
				str += _("m");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_AREA:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = squaremeter
				str += _("m^2");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_RADIANT_INTENSITY:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = watt per steradian
				str += _("W/sr");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_RADIANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = W/(sr*m^2)
				str += _("W/(sr*m^2)");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_IRRADIANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = Watt per squaremeter
				str += _("W/m^2)");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_SPECTRAL_RADIANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = W/(sr*m^2*nm)
				str += _("W/(sr*m^2*nm)");
				break;
			}
		}
			break;

		case VSCP_TYPE_MEASUREMENT_SPECTRAL_IRRADIANCE:
		{
			switch (VSCP_DATACODING_UNIT(*(pEvent->pdata+offset))) {
			case 0x00: // default unit = W/(m^2*nm)
				str += _("W/(m^2*nm)");
				break;
			}
		}
			break;

		}
	}
		break;

		// **** CLASS ****
	case VSCP_CLASS1_DATA:
	{

		switch (pEvent->vscp_type) {

		case VSCP_TYPE_DATA_IO:
		{

		}
			break;

		case VSCP_TYPE_DATA_AD:
		{

		}
			break;

		case VSCP_TYPE_DATA_DA:
		{

		}
			break;

		case VSCP_TYPE_DATA_RELATIVE_STRENGTH:
		{

		}
			break;

		case VSCP_TYPE_DATA_SIGNAL_LEVEL:
		{

		}
			break;

		case VSCP_TYPE_DATA_SIGNAL_QUALITY:
		{

		}
			break;

		}
	}
		break;

		// **** CLASS ****
	case VSCP_CLASS1_INFORMATION:

		switch (pEvent->vscp_type) {

		case VSCP_TYPE_INFORMATION_GENERAL:
		case VSCP_TYPE_INFORMATION_ON:
		case VSCP_TYPE_INFORMATION_OFF:
		case VSCP_TYPE_INFORMATION_ALIVE:
		case VSCP_TYPE_INFORMATION_TERMINATING:
		case VSCP_TYPE_INFORMATION_OPENED:
		case VSCP_TYPE_INFORMATION_CLOSED:
		case VSCP_TYPE_INFORMATION_NODE_HEARTBEAT:
		case VSCP_TYPE_INFORMATION_BELOW_LIMIT:
		case VSCP_TYPE_INFORMATION_ABOVE_LIMIT:
		case VSCP_TYPE_INFORMATION_PULSE:
		case VSCP_TYPE_INFORMATION_ERROR:
		case VSCP_TYPE_INFORMATION_RESUMED:
		case VSCP_TYPE_INFORMATION_PAUSED:
		case VSCP_TYPE_INFORMATION_GOOD_MORNING:
		case VSCP_TYPE_INFORMATION_GOOD_DAY:
		case VSCP_TYPE_INFORMATION_GOOD_AFTERNOON:
		case VSCP_TYPE_INFORMATION_GOOD_EVENING:
		case VSCP_TYPE_INFORMATION_GOOD_NIGHT:
		case VSCP_TYPE_INFORMATION_GOODBYE:
		case VSCP_TYPE_INFORMATION_STOP:
		case VSCP_TYPE_INFORMATION_START:
		case VSCP_TYPE_INFORMATION_RESET_COMPLETED:
		case VSCP_TYPE_INFORMATION_INTERRUPTED:
		case VSCP_TYPE_INFORMATION_PREPARING_TO_SLEEP:
		case VSCP_TYPE_INFORMATION_WOKEN_UP:
		case VSCP_TYPE_INFORMATION_DUSK:
		case VSCP_TYPE_INFORMATION_DAWN:
		case VSCP_TYPE_INFORMATION_ACTIVE:
		case VSCP_TYPE_INFORMATION_INACTIVE:
		case VSCP_TYPE_INFORMATION_BUSY:
		case VSCP_TYPE_INFORMATION_IDLE:
		case VSCP_TYPE_INFORMATION_LEVEL_CHANGED:
		case VSCP_TYPE_INFORMATION_WARNING:
		case VSCP_TYPE_INFORMATION_SUNRISE:
		case VSCP_TYPE_INFORMATION_SUNSET:
		case VSCP_TYPE_INFORMATION_DETECT:
		case VSCP_TYPE_INFORMATION_OVERFLOW:

			if ((pEvent->sizeData-offset) >= 3) {
				str = wxString::Format(_("Index=%d Zone=%d Subzone=%d\n"),
						pEvent->pdata[ 0+offset ],
						pEvent->pdata[ 1+offset ],
						pEvent->pdata[ 2+offset ]);
			}
			break;

		case VSCP_TYPE_INFORMATION_BUTTON:
			if ((pEvent->sizeData-offset) >= 3) {

				// Key type code
				if (0 == (pEvent->pdata[ 0+offset ] & 0x03)) {
					str = _("Button released.");
				} else if (1 == (pEvent->pdata[ 0+offset ] & 0x03)) {
					str = _("Button pressed.");
				} else if (2 == (pEvent->pdata[ 0+offset ] & 0x03)) {
					str = _("Keycode.");
				}
				else {
					str = _("Unknown key type code.");
				}

				str += wxString::Format(_("Repeat count = %d\n"), 
						(pEvent->pdata[ 0+offset ] >> 3 & 0x01f));

				str += wxString::Format(_("Zone=%d Subzone=%d\n"),
						pEvent->pdata[ 1+offset ],
						pEvent->pdata[ 2+offset ]);

				str += wxString::Format(_("Button Code=%d\n"),
						((pEvent->pdata[ 3+offset ] << 8) + pEvent->pdata[ 4+offset ]));

				str += wxString::Format(_("Code Page=%d\n"),
						((pEvent->pdata[ 5+offset ] << 8) + pEvent->pdata[ 6+offset ]));
			}
			break;

		case VSCP_TYPE_INFORMATION_MOUSE:
			if ((pEvent->sizeData-offset) == 7) {

				str += wxString::Format(_("Zone=%d Subzone=%d\n"),
						pEvent->pdata[ 1+offset ],
						pEvent->pdata[ 2+offset ]);

				if (0 == pEvent->pdata[ 0+offset ]) {
					str += _("Absolute coordinates.\n");
				} else if (1 == pEvent->pdata[ 0+offset ]) {
					str += _("Relative coordinates.\n");
				} else {
					str += _("Unknown coordinates.\n");
				}

				str += wxString::Format(_("x=%d y=%d\n"),
						((pEvent->pdata[ 3+offset ] << 8) + pEvent->pdata[ 4+offset ]),
						((pEvent->pdata[ 5+offset ] << 8) + pEvent->pdata[ 6+offset ]));

			} else {
				str = _("Wrong number of databytes.");
			}
			break;


		case VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY:
		{
			if ((pEvent->sizeData-offset) >= 5) {

				str = wxString::Format(_("Token activity.\n Event Code=%d ["), 
						(*(pEvent->pdata+offset) & 0x03));

				// Event Code
				switch (*(pEvent->pdata+offset) & 0x03) {

				case 0:
					str += _("Touched and released] ");
					break;

				case 1:
					str += _("Touched] ");
					break;

				case 2:
					str += _("Released] ");
					break;

				case 3:
					str += _("Reserved code] ");
					break;

				}

				// Token code
				str += wxString::Format(_("\nToken Code=%d ["),
						(*(pEvent->pdata+offset) >> 2) & 0x3f);
				switch ((*(pEvent->pdata+offset) >> 2) & 0x3f) {

				case 0:
					str += _("Unknown token. 128-bits.] ");
					break;

				case 1:
					str += _("iButton token. 64-bits.] ");
					break;

				case 2:
					str += _("RFID Token. 64-bits.] ");
					break;

				case 3:
					str += _("RFID Token. 128-bits.] ");
					break;

				case 4:
					str += _("RFID Token. 256-bits.] ");
					break;

				case 9:
					str += _("ID/Credit card. 128-bits.] ");
					break;

				case 16:
					str += _("Biometri device. 256-bits.] ");
					break;

				case 17:
					str += _("Biometri device. 64-bits.] ");
					break;

				case 18:
					str += _("Bluetooth device. 48-bits.] ");
					break;

				case 19:
					str += _("GSM IMEI Code. 64-bits.] ");
					break;

				case 20:
					str += _("GSM IMSI code. 64-bits.] ");
					break;

				case 21:
					str += _("RFID Token. 40-bits.] ");
					break;

				case 22:
					str += _("RFID Token. 32-bits.] ");
					break;

				case 23:
					str += _("RFID Token. 24-bits.] ");
					break;

				case 24:
					str += _("RFID Token. 16-bits.] ");
					break;

				case 25:
					str += _("RFID Token. 8-bits.] ");
					break;

				default:
					str += _("Reserved token. ] ");
				}

				wrkstr1 = wxString::Format(_("\nZone=%d\n Subzone=%d\n Index=%d\n"),
						pEvent->pdata[ 1+offset ],
						pEvent->pdata[ 2+offset ],
						pEvent->pdata[ 3+offset ]);
				str += wrkstr1;
			} else {
				str = _("Invalid data!");
			}
		} // Token activity
			break;


		case VSCP_TYPE_INFORMATION_STREAM_DATA:
		{
			str = wxString::Format(_("sequence index=%d\n"),
					pEvent->pdata[ 0+offset ]);
			str += _("steam data = ");
			for (i = 1; i < (pEvent->sizeData-offset); i++) {
				str += wxString::Format(_("%02X(%c) "),
						pEvent->pdata[ i+offset ],
						pEvent->pdata[ i+offset ]);
			}
			str += _("\n");
		}
			break;

		case VSCP_TYPE_INFORMATION_STREAM_DATA_WITH_ZONE:
		{

			str = wxString::Format(_("\nZone=%d\n Subzone=%d\n"),
					pEvent->pdata[ 0+offset ],
					pEvent->pdata[ 1+offset ]);

			str += wxString::Format(_("sequence index=%d\n"),
					pEvent->pdata[ 2+offset ]);
			str += _("steam data = ");
			for (i = 3; i < (pEvent->sizeData-offset); i++) {
				str += wxString::Format(_("%02X(%c) "),
						pEvent->pdata[ i+offset ],
						pEvent->pdata[ i+offset ]);
			}
			str += _("\n");
		}
			break;

		case VSCP_TYPE_INFORMATION_CONFIRM:
		{
			str = wxString::Format(_("\nZone=%d\n Subzone=%d\n Sequence index=%d\n"),
					pEvent->pdata[ 0+offset ],
					pEvent->pdata[ 1+offset ],
					pEvent->pdata[ 2+offset ]);
			str += wxString::Format(_("Class=%d\n"),
					((pEvent->pdata[ 3+offset ] << 8) + pEvent->pdata[ 4+offset ]));

			str += wxString::Format(_("Type=%d\n"),
					((pEvent->pdata[ 5+offset ] << 8) + pEvent->pdata[ 6+offset ]));
		}
			break;

		case VSCP_TYPE_INFORMATION_STATE:
		{
			str = wxString::Format(_("Zone=%d\n Subzone=%d\n User byte=%d\n"),
					pEvent->pdata[ 1+offset ],
					pEvent->pdata[ 2+offset ],
					pEvent->pdata[ 0+offset ]);
			str += wxString::Format(_("Current state=%d\n Previous state=%d\n"),
					pEvent->pdata[ 3+offset ],
					pEvent->pdata[ 3+offset ]);
		}
			break;

		case VSCP_TYPE_INFORMATION_ACTION_TRIGGER:
		{
			str = wxString::Format(_("Zone=%d\n Subzone=%d\n Action Trigger=%d\n"),
					pEvent->pdata[ 1+offset ],
					pEvent->pdata[ 2+offset ],
					pEvent->pdata[ 0+offset ]);
		}
			break;

		case VSCP_TYPE_INFORMATION_START_OF_RECORD:
		{
			str = wxString::Format(_("Zone=%d\n Subzone=%d\n Record=%d\n Count=%d\n"),
					pEvent->pdata[ 1+offset ],
					pEvent->pdata[ 2+offset ],
					pEvent->pdata[ 0+offset ],
					pEvent->pdata[ 3+offset ]);
		}
			break;

		case VSCP_TYPE_INFORMATION_END_OF_RECORD:
		{
			str = wxString::Format(_("Zone=%d\n Subzone=%d\n Record=%d\n"),
					pEvent->pdata[ 1+offset ],
					pEvent->pdata[ 2+offset ],
					pEvent->pdata[ 0+offset ]);
		}
			break;

		case VSCP_TYPE_INFORMATION_PRESET_ACTIVE:
		{
			str = wxString::Format(_("Zone=%d\n Subzone=%d\n Preset code=%d\n"),
					pEvent->pdata[ 1+offset ],
					pEvent->pdata[ 2+offset ],
					pEvent->pdata[ 3+offset ]);
		}
			break;

		} // switch type
		break;

	} // switch class

	return str;

}


