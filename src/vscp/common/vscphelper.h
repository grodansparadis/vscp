// FILE: vscphelper.h
//
// Copyright (C) 2002-2014 Ake Hedman akhe@grodansparadis.com
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

/*!
    \file vscphelper.h
    \brief	The vscphelper file contains often used functionality when working with VSCP.
    \details vscphealper have common used functionality to do things in the VSCP world. 
    It can be seens as the main toolbox for the VSCP programmer.
 */


#if !defined(VSCPHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define VSCPHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_


#include <wx/wx.h>
#include <wx/hashmap.h>
#ifndef WIN32
#include <sys/times.h>
#endif


#include "vscp.h"
#include "vscp_class.h"
#include "vscp_type.h"
#include "canal.h"
#include "../../common/crc.h"

// Forward declaration
class CMDF;

#ifdef __cplusplus
extern "C" {
#endif

    //@{

    /*! 
      \union vscp_value
      \brief VSCP Data coding declaration
     */
    union vscp_value {
        /// float value
        double float_value;
        /// Integer and long value and 
        long long_value;
        /// Integer value
        int int_value;
        /// Byte value
        unsigned char byte_value;
    };
    //@}


	// ***************************************************************************
	//                                General Helpers
	// ***************************************************************************

	/*!
      Read a numerical value from a string
      The string value can be set as a hex or a decimal value.
      \param strval wxString containing value to be converted
      \return Unsigned long containing value
     */
 
    uint32_t vscp_readStringValue(const wxString& strval);

	/*!
		Convert string to lowercase
	*/
	int vscp_lowercase(const char *s);

	/*!
		String non case compare
		@param s1 String1 to compare
		@param s2 String2 to compare
		@return 0 if strings are the same
	*/
	int vscp_strcasecmp(const char *s1, const char *s2);

	/*!
		String non case compare with length
		@param s1 String1 to compare
		@param s2 String2 to compare
		@param len Number of byte to compare
		@return 0 if strings are the same
	*/
	int vscp_strncasecmp(const char *s1, const char *s2, size_t len);


	/*!
		Stringify binary data. 	
		@param to Pointer output buffer that holds the result. 
			Output buffer must be twice as big as input,
			because each byte takes 2 bytes in string representation
		@param p Pointer to digest.
		@param len Digest len
	*/
	void vscp_bin2str( char *to, const unsigned char *p, size_t len ); 

	/*!
		Get GMT time // http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3
	*/
	void vscp_getTimeString( char *buf, size_t buf_len, time_t *t ); 

	// ***************************************************************************
	//                                Data Coding Helpers
	// ***************************************************************************

    
    /*!
        Fetch datacoding byte from measurement events
        \param pEvent Pointer to VSCP event
        \return Measurement datacoding byte or zero if its not an 
            event with a datacoding.
     */
    uint8_t vscp_getMeasurementDataCoding(const vscpEvent *pEvent);

    /*!
      Get bitarray from coded event data
      \param pNorm Pointer to normalized integer.
      \param length Number of bytes it consist of including
      the first normalize byte.
      \return Bitarray as a unsigned 64-bit integer.
     */
    uint64_t vscp_getDataCodingBitArray(const uint8_t *pNorm, uint8_t length);

    /*!
      Get normalised integer from coded event data
      \param pNorm Pointer to normalised integer.
      \param length Number of bytes it consist of including
      the first normalise byte.
      \return returns value as a double.
     */
    double vscp_getDataCodingNormalizedInteger(const unsigned char *pNorm, 
                                                    unsigned char length);

    /*!
      Get the string from coded event data
      \param pString Pointer to normalised integer.
      \param length Number of bytes it consist of including
      the first normalise byte.
      \return Returns unicode UTF-8 string of event data
     */
	 
    wxString& vscp_getDataCodingString(const unsigned char *pString, 
                                            const unsigned char length);


    /*!
      Get data in the VSCP data coding format to a string. Works for
	  CLASS1.MEAUREMENT, CLASS2_LEVEL1.MEASUREMENT
      \param pEvent Pointer to VSCP event.
      \param str String that holds the result
      \return true on success, false on failure.
     */

    bool vscp_getVSCPMeasurementAsString(const vscpEvent *pEvent, wxString& str);

    
    /*!
      Get data in the VSCP data coding format as a double. Works for
	  CLASS1.MEAUREMENT, CLASS2_LEVEL1.MEASUREMENT, 
      VSCP_CLASS1_MEASUREZONE, VSCP_CLASS1_SETVALUEZONE
      \param pEvent Pointer to VSCP event.
      \param pvalue Pointer to double that holds the result
      \return true on success, false on failure.
     */
    bool vscp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue);
	
	/*!
      Get data in the VSCP data coding format to a string. Works for
	  CLASS1.MEASUREMENT64, CLASS2_LEVEL1.MEASUREMENT64
      \param pEvent Pointer to VSCP event.
      \param str String that holds the result
      \return true on success, false on failure.
     */
 
    bool vscp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent, wxString& str);

	
    /*!
      Convert a floating point measurement value into VSCP data with the
      first byte being the normalizer byte
      CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
      \param value Floating point value to convert.
      \param pdata Pointer to beginning of VSCP returned event data.
      \param psize Pointer to size for returned data.
      \param unit Untit for the data. Zero is default.
      \param sensoridx Sensor index 0-7. Zero is default.
      \return true on success, false on failure.
     */
    bool vscp_convertFloatToNormalizedEventData( uint8_t *pdata,
                                                    uint16_t *psize,
                                                    double value,                                                
                                                    uint8_t unit,
                                                    uint8_t sensoridx ); 

	/*!
	  Convert a floating point measurement value into VSCP data as a
      single precision float (32-bit) for
	  CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
      \param value Floating point value to convert.
      \param pdata Pointer to beginning of VSCP returned event data.
	  \param psize Pointer to size for returned data.
	  \param unit Untit for the data. Zero is default.
	  \param sensoridx Sensor index 0-7. Zero is default.
      \return true on success, false on failure.
     */
	bool vscp_convertFloatToFloatEventData( uint8_t *pdata,
                                                uint16_t *psize, 
                                                float value,
                                                uint8_t unit,
                                                uint8_t sensoridx );

    /*!
      Convert an integer measurement value into VSCP data with the
      first byte being the normalizer byte
      CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
      \param value Floating point value to convert.
      \param pdata Pointer to beginning of VSCP returned event data.
      \param psize Pointer to size for returned data.
      \param unit Untit for the data. Zero is default.
      \param sensoridx Sensor index 0-7. Zero is default.
      \return true on success, false on failure.
     */
    bool vscp_convertIntegerToNormalizedEventData( uint8_t *pdata,
                                                    uint16_t *psize,
                                                    uint64_t val64,
                                                    uint8_t unit=0,
                                                    uint8_t sensoridx=0 ); 

    /*!
	  Convert a floating point measurement value into VSCP data as a
      single precision float (32-bit) for
	  CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
      \param value Floating point value to convert.
      \param pEvent Pointer to event with pdata set to NULL. cscp_class and
                    vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
	  \param psize Pointer to size for returned data.
	  \param unit Untit for the data. Zero is default.
	  \param sensoridx Sensor index 0-7. Zero is default.
      \return true on success, false on failure.
     */
    bool vscp_makeFloatMeasurementEvent( vscpEvent *pEvent, 
                                            float value,
                                            uint8_t unit,
                                            uint8_t sensoridx );
	
	/*!
      Get data in the VSCP data coding format to a string. Works for
	  CLASS1.MEASUREZONE and CLASS1.SETVALUEZONE,CLASS2_LEVEL1.MEASUREZONE
	  CLASS2_LEVEL1.SETVALUEZONE
      \param pEvent Pointer to VSCP event.
      \param str String that holds the result
      \return true on success, false on failure.
     */
 
    bool vscp_getVSCPMeasurementZoneAsString(const vscpEvent *pEvent, wxString& str);


    /*!
      Get data in the VSCP data coding format to a float
      \param pNorm Pointer to VSCP event.
      \param length Number of bytes it consist of including datacoding byte
      \return value as float
     */
    float vscp_getMeasurementAsFloat(const unsigned char *pNorm, const unsigned char length);


    /*!
      Replace backshlashes in a string with forward slashes
      \param strval String that should be handled.
      \return Fixed string.
     */
 
    wxString& vscp_replaceBackslash(wxString& strval);



    /*!
      Get VSCP priority
      \param pEvent Pointer to VSCP event to set priority for.
      \return Priority (0-7) for event.
     */
    unsigned char vscp_getVscpPriority(const vscpEvent *pEvent);

    /*!
      Get VSCP priority
      \param pEvent Pointer to VSCP event to set priority for.
      \return Priority (0-7) for event.
     */
    unsigned char vscp_getVscpPriorityEx(const vscpEventEx *pEvent);

    /*!
      Set VSCP priority
      \param pEvent Pointer to VSCP event to set priority for.
      \param priority Priority (0-7) to set.
     */
    void vscp_setVscpPriority(vscpEvent *pEvent, unsigned char priority);
    
     /*!
      Set VSCP priority Ex
      \param pEvent Pointer to VSCP event to set priority for.
      \param priority Priority (0-7) to set.
     */
    void vscp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority);

    /*!
      Get VSCP head from CAN id
      \param id CAN id
      \return VSCP head
     */
    unsigned char vscp_getVSCPheadFromCANid(const uint32_t id);

    /*!
     Get VSCP class from CAN id
      \param id CAN id
      \return VSCP class
     */
    uint16_t vscp_getVSCPclassFromCANid(const uint32_t id);

    /*!
      Get VSCP type from CAN id
      \param id CAN id
     \return VSCP type
     */
    uint16_t vscp_getVSCPtypeFromCANid(const uint32_t id);

    /*!
     Get VSCP nide nickname from CAN id
      \param id CAN id
      \return VSCP node nickname
     */
    uint16_t vscp_getVSCPnicknameFromCANid(const uint32_t id);

    /*!
      Get CAN id from VSCP info
      \param priority VSCP priority
      \param vscp_class VSCP class
      \param vscp_type VSCP type
      \return CAN id with nickname == 0
     */
    uint32_t vscp_getCANidFromVSCPdata(const unsigned char priority,
            const uint16_t vscp_class,
            const uint16_t vscp_type);

    /*!
      Get CAN id from VSCP event
      \param pEvent Pointer to VSCP event
      \return CAN id with nickname == 0
     */
    uint32_t vscp_getCANidFromVSCPevent(const vscpEvent *pEvent);

    /*!
      Get CAN id from VSCP event
      \param pEvent Pointer to VSCP event
      \return CAN id with nickname == 0
     */
    uint32_t vscp_getCANidFromVSCPeventEx(const vscpEventEx *pEvent);

    /*!
      Calculate CRC for VSCP event
     */
    unsigned short vscp_vscp_calc_crc(vscpEvent *pEvent, short bSet);



    // Helpers


    /*!
        calcCRC4GUIDArray

        \param Pointer to GUID array (MSB-LSB order)
        \retur crc for GUID.
    */
    uint8_t vscp_calcCRC4GUIDArray(const uint8_t *pguid);

    /*!
        calcCRC4GUIDString

        \param Pointer to GUID array (MSB-LSB order)
        \retur crc for GUID.
    */
    uint8_t vscp_calcCRC4GUIDString(const wxString &strguid);


    /*!
      Get GUID from string

      \param pEvent Pointer to VSCP event
      \param strGUID String with GUID (xx:yy:zz....)
      \return True on success, false on failure.
     */
 
    bool vscp_getGuidFromString(vscpEvent *pEvent, const wxString& strGUID);

    
        /*!
      Get GUID from string

      \param pEvent Pointer to VSCP event
      \param strGUID String with GUID (xx:yy:zz....)
      \return True on success, false on failure.
     */
 
    bool vscp_getGuidFromStringEx(vscpEventEx *pEventEx, const wxString& strGUID);


    /*!
      Fill event GUID from a string
     */
 
    bool vscp_getGuidFromStringToArray(unsigned char *pGUID, const wxString& strGUID);

	
	/*!
      Write out GUID to string

      \param pGUID Pointer to VSCP GUID array.
      \param strGUID Reference to string for written GUID
      \return True on success, false on failure.
    */

    bool vscp_writeGuidArrayToString(const unsigned char *pGUID, wxString& strGUID);


    /*!
      Write out GUID to string

      \param pEvent Pointer to VSCP event
      \param strGUID Reference to string for written GUID
      \return True on success, false on failure.
     */
	 
    bool vscp_writeGuidToString(const vscpEvent *pEvent, wxString& strGUID);

    
        /*!
      Write out GUID to string

      \param pEvent Pointer to VSCP event
      \param strGUID Reference to string for written GUID
      \return True on success, false on failure.
     */
	 
    bool vscp_writeGuidToStringEx(const vscpEventEx *pEvent, wxString& strGUID);


    /*!
      Write out GUID to string as four rows

      \param pEvent Pointer to VSCP event
      \param strGUID Reference to string for written GUID
      \return True on success, false on failure.
     */
 
    bool vscp_writeGuidToString4Rows(const vscpEvent *pEvent, wxString& strGUID);

    
        /*!
      Write out GUID to string as four rows

      \param pEvent Pointer to VSCP event
      \param strGUID Reference to string for written GUID
      \return True on success, false on failure.
     */
	 
    bool vscp_writeGuidToString4RowsEx(const vscpEventEx *pEvent, wxString& strGUID);



    /*!
      Check if GUID is all null
      \param pGUID pointer to GUID to check
      \return tru of empty, false if not.
     */
    bool vscp_isGUIDEmpty(const unsigned char *pGUID);

    /*!
      Compare two GUID's
      \param pGUID1 First GUID to compare
      \param pGUID2 Second GUID to compare
      \return True if the two GUID's are equal. False otherwise.
     */
    bool vscp_isSameGUID(const unsigned char *pGUID1, const unsigned char *pGUID2);

    /*!
        Recerse GUID
        \param pGUID Pointer to GUID to reverse.
        \return true if OK.
     */
    bool vscp_reverseGUID(unsigned char *pGUID);

    /*!
      Convert a standard VSCP event to the Ex version
     */
    bool vscp_convertVSCPtoEx(vscpEventEx *pEventEx, const vscpEvent *pEvent);

    /*!
      Convert an Ex event to a standard VSCP event
     */
    bool vscp_convertVSCPfromEx(vscpEvent *pEvent, const vscpEventEx *pEventEx);

    /*!
      Delete a standard VSCP event
     */
    void vscp_deleteVSCPevent(vscpEvent *pEvent);

    /*!
      Delete an Ex event
     */
    void vscp_deleteVSCPeventEx(vscpEventEx *pEventEx);

    /*!
      Clear VSCP filter so it will allow all events to go through
      \param pFilter Pointer to VSCP filter.
     */
    void vscp_clearVSCPFilter(vscpEventFilter *pFilter);

    /*!
      Check filter/mask to check if filter should be delivered

      filter ^ bit    mask    out
      ============================
           0           0       1    filter == bit, mask = don't care result = true
           0           1       1    filter == bit, mask = valid, result = true
           1           0       1    filter != bit, mask = don't care, result = true
           1           1       0    filter != bit, mask = valid, result = false

      Mask tells *which* bits that are of interest means
      it always returns true if bit set to zero (0=don't care).

      Filter tells the value fo valid bits. If filter bit is == 1 the bits
      must be equal to get a true filter return.

      So a nill mask will let everything through
      \return true if message should be delivered false if not.
     */
    bool vscp_doLevel2Filter(const vscpEvent *pEvent,
            const vscpEventFilter *pFilter);

    bool vscp_doLevel2FilterEx(const vscpEventEx *pEventEx,
            const vscpEventFilter *pFilter);

    /*!
        Read a filter from a string
        \param pFilter Filter structure to write filter to.
        \param strFilter Filter in string form 
                filter-priority, filter-class, filter-type, filter-GUID
        \return true on success, fals eon failure.
     */
 
    bool vscp_readFilterFromString(vscpEventFilter *pFilter, wxString& strFilter);


    /*!
        Read a mask from a string
        \param pFilter Filter structure to write mask to.
        \param strMask Mask in string form 
                mask-priority, mask-class, mask-type, mask-GUID
        \return true on success, fals eon failure.
     */
 
    bool vscp_readMaskFromString(vscpEventFilter *pFilter, wxString& strMask);
	

    /*!
      Convert an Event from a CANAL message
     */
    bool vscp_convertCanalToEvent(vscpEvent *pvscpEvent,
            const canalMsg *pcanalMsg,
            unsigned char *pGUID,
            bool bCAN = false);
    
    /*!
      Convert an Event from a CANAL message
     */
    bool vscp_convertCanalToEventEx(vscpEventEx *pvscpEvent,
            const canalMsg *pcanalMsg,
            unsigned char *pGUID,
            bool bCAN = false);

    /*!
      Covert VSCP event to CANAL message
     */
    bool vscp_convertEventToCanal(canalMsg *pcanalMsg,
            const vscpEvent *pvscpEvent);
    
    /*!
      Covert VSCP event to CANAL message
    */
    bool vscp_convertEventExToCanal(canalMsg *pcanalMsg,
            const vscpEventEx *pvscpEvent);


    /*!
      Make a timestamp for events etc 
      \return Event timestamp as an unigned long
     */
    unsigned long vscp_makeTimeStamp(void);


    /*!
      Copy a VSCP event to another

      \param pEventTo Pointer to event to copy to.
      \param pEventFrom Pointer to event to copy from.
      \return True on success.
     */
    bool vscp_copyVSCPEvent(vscpEvent *pEventTo, const vscpEvent *pEventFrom);
    

    /*!
      Write VSCP data to string
      \param pEvent Pointer to event where data is fetched from
      \param str String that receive result.
      \param bUseHtmlBreak Set to true to use <br> instead of \\n as
      line break 
      \return True on success false on failure.
     */
	 
    bool vscp_writeVscpDataToString(const vscpEvent *pEvent, 
								wxString& str, 
								bool bUseHtmlBreak = false);
								

    /*!
      Write VSCP data to string
      \param sizeData Number of databytes.
      \param pData Pointer to datastructure.
       \param str String that receive result.
      \param bUseHtmlBreak Set to true to use <br> instead of \\n as
      line break 
      \return True on success false on failure.
     */
	 
    bool vscp_writeVscpDataWithSizeToString(const uint16_t sizeData,
										const unsigned char *pData,
										wxString& str,
										bool bUseHtmlBreak = false,
                                        bool bBreak = true );
									

    /*!
      Get VSCP data from a string
      \param pEvent Ponter to a VSCP event to write parsed data to.
      \param str A string with comma or whitespace separated data in decimal
      or hexadecimal form. Data can span multiple lines.
      \return true on success, false on failure.
     */
 
    bool vscp_getVscpDataFromString(vscpEvent *pEvent, const wxString& str);


    /*!
      Get VSCP data from a string
      \param pData Pointer to a unsigned byte array to write parsed data to.
      \param psizeData Number of data bytes. 
      \param str A string with comma or whitespace separated data in decimal
      or hexadecimal form. Data can span multiple lines.
      \return true on success, false on failure.
     */
 
    bool vscp_getVscpDataArrayFromString( uint8_t *pData, 
										uint16_t *psizeData, 
										const wxString& str);
									

    /*!
      Write event to string.
      priority,class,type,guid,data
      \param pEvent Event that should be presented
      \param str String that receive the result
      \return true on success, false on failure.
     */
 
    bool vscp_writeVscpEventToString(vscpEvent *pEvent, wxString& str);


    /*!
      Write event to string.
      priority,class,type,guid,data
      \param pEvent Event that should be presented
      \param str String that receive the result
      \return true on success, false on failure.
     */
	 
    bool vscp_writeVscpEventExToString(vscpEventEx *pEvent, wxString& str);
	


    /*!
      Get event data from string format
      Format: head,class,type,obid,timestamp,GUID,data1,data2,data3....
      \param pEvent Event that will get data
      \param str String that contain the event on string form
      \return true on success, false on failure.
     */
	 
    bool vscp_getVscpEventFromString(vscpEvent *pEvent, const wxString& str);
	

    /*!
      Get event data from string format
      Format: head,class,type,obid,timestamp,GUID,data1,data2,data3....
      \param pEventEx Pointer to VSCP event that will get the parsed data
      \param str String that contain the event on string form
      \return true on success, false on failure.
     */
 
    bool vscp_getVscpEventExFromString(vscpEventEx *pEventEx, const wxString& str);


    /*!
      Get Data in real text.
      This for meaurement class events that can be describes in real text
      in a deterministic way. Temperature event is typical which can be returned
      in clear text as "Temperature = 22.5 C".
      \param pEvent Event that should be presented
      \return Text data representation of the event data or an empty string 
      if the class/type pair is not supported..
     */
 
    wxString& vscp_getRealTextData(vscpEvent *pEvent);


    /*!
      This function makes a HTML string from a standard string. LF is replaced
      with a '<BR>'.
      \param str String that should be HTML coded.
     */
 
    void vscp_makeHtml(wxString& str);


	/*
		Get device HTML status from device
		@param registers Aray with all 256 registers for the device
	    @param pmdf Optional pointer to CMDF class which gives more info
				about the device if it is supplied.
	 */
	 
	wxString& vscp_getDeviceHtmlStatusInfo( const uint8_t *registers, CMDF *pmdf );


#ifdef __cplusplus
}
#endif

#endif // #if !defined(AFX_VSCPHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)

