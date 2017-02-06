// FILE: vscphelper.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2017 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


/*!
    \file vscphelper.h
    \brief	The vscphelper file contains often used functionality when working with VSCP.
    \details vscphealper have common used functionality to do things in the VSCP world. 
    It can be seen as the main toolbox for the VSCP programmer.
 */


#if !defined(VSCPHELPER_H__INCLUDED_)
#define VSCPHELPER_H__INCLUDED_


#include <wx/wx.h>
#include <wx/hashmap.h>
#ifndef WIN32
#include <sys/times.h>
#endif 

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <canal.h>

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
        Return non zero if endiness is little endian
    */
    int vscp_littleEndian( void );

    /*!
        Return non zero if endiness is big endian
    */
    int vscp_bigEndian( void );

    /*!
      Read a numerical value from a string
      The string value can be set as a hex or a decimal value.
      @param strval wxString containing value to be converted
      @return Unsigned long containing value
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
    /
    void vscp_bin2str( char *to, const unsigned char *p, size_t len ); 

    /*!
        Get GMT time // http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3
    */
    void vscp_getTimeString( char *buf, size_t buf_len, time_t *t ); 
    
    
    /*!
     *  Escape XML string 
     *
     *  @param Buffer holding input string. Buffer size must be large enough to
     *          hold expanded result.
     */ 
    void vscp_toXMLEscape( char *temp_str );

    
    
    
    
    
    // ***************************************************************************
    //                             Measurement Helpers
    // ***************************************************************************

    
    
    
        
    /*!
        Fetch data coding byte from measurement events
        @param pEvent Pointer to VSCP event
        @return Measurement data coding byte or zero if its not an 
        event with a data coding.
    */
    uint8_t vscp_getMeasurementDataCoding( const vscpEvent *pEvent );

    /*!
        Get bit array from coded event data
        @param pCode Pointer to data coding byte.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return Bit array as a unsigned 64-bit integer.
    */
    uint64_t vscp_getDataCodingBitArray( const uint8_t *pCode, uint8_t length);


    /*!
        Get integer from coded event data
        @param pCode Pointer to normalised integer.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return returns value as a 64-bit integer.
     */
    int64_t vscp_getDataCodingInteger( const uint8_t *pCode,
                                            uint8_t length );

    /*!
        Get normalised integer from coded event data
        @param pCode Pointer to normalised integer.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return returns value as a double.
    */
    double vscp_getDataCodingNormalizedInteger( const unsigned char *pCode, 
                                                    unsigned char length);

    /*!
        Get the string from coded event data
        @param pString Pointer to normalised integer.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return Returns unicode UTF-8 string of event data
    */

    bool vscp_getDataCodingString( const unsigned char *pCode, 
                                            unsigned char dataSize,
                                            wxString& strResult );


    /*!
        Write data from event in the VSCP data coding format to a string. 
        
        Works for
        
        CLASS1.MEASUREMENT
        CLASS2_LEVEL1.MEASUREMENT 
        VSCP_CLASS1_MEASUREZONE
        VSCP_CLASS1_SETVALUEZONE
        CLASS2_MEASUREMENT_FLOAT
        CLASS2_MEASUREMENT_STR
         
        @param pEvent Pointer to VSCP event.
        @param str String that holds the result
        @return true on success, false on failure.
    */

    bool vscp_getVSCPMeasurementAsString( const vscpEvent *pEvent, 
                                            wxString& str);

    
    /*!
        Write data from event in the VSCP data coding format as a double. 
        
        Works for
        
        CLASS1.MEASUREMENT
        CLASS2_LEVEL1.MEASUREMENT
        VSCP_CLASS1_MEASUREZONE
        VSCP_CLASS1_SETVALUEZONE
        CLASS2_MEASUREMENT_FLOAT
        CLASS2_MEASUREMENT_STR
        
        @param pEvent Pointer to VSCP event.
        @param pvalue Pointer to double that holds the result
        @return true on success, false on failure.
    */
    bool vscp_getVSCPMeasurementAsDouble( const vscpEvent *pEvent, 
                                            double *pvalue );

    /*!
     * Get measurement unit for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement unit or -1 for error (event that is not a measurement). 
     */
    int vscp_getVSCPMeasurementUnit( const vscpEvent *pEvent );
    
    /*!
     * Get measurement sensor index for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement sensor index or -1 for error or for event that is not a measurement
     *          or measurement event that does not have a sensor index. 
     */
    int vscp_getVSCPMeasurementSensorIndex( const vscpEvent *pEvent );
    
    /*!
     * Get measurement zone for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement zone or 0 for error or event that is not a measurement
     *          or measurement event that does not have a zone). 
     */
    int vscp_getVSCPMeasurementZone( const vscpEvent *pEvent );
    
    /*!
     * Get measurement subzone for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement subzone or -1 for error or for event that is not a measurement
     *          or measurement event that does not have a subzone. 
     */
    int vscp_getVSCPMeasurementSubZone( const vscpEvent *pEvent );
    
    /*!
     * Check if event is a measurement
     * @param pEvent Pointer to VSCP event.
     * @return Return true if the event is a measurement.
     */
    bool vscp_isVSCPMeasurement( const vscpEvent *pEvent );
    
    /*!
        Get data in the VSCP data coding format to a string. Works for
        CLASS1.MEASUREMENT64, CLASS2_LEVEL1.MEASUREMENT64
        @param pEvent Pointer to VSCP event.
        @param str String that holds the result
        @return true on success, false on failure.
     */
    bool vscp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent, 
                                                    wxString& str);
    
    

    /*!
      Convert a floating point measurement value into VSCP data with the
      first byte being the normalizer byte
      CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
      @param value Floating point value to convert.
      @param pdata Pointer to beginning of VSCP returned event data.
      @param psize Pointer to size for returned data.
      @param unit Unit for the data. Zero is default.
      @param sensoridx Sensor index 0-7. Zero is default.
      @return true on success, false on failure.
    */
    bool vscp_convertFloatToNormalizedEventData( uint8_t *pdata,
                                                    uint16_t *psize,
                                                    double value,
                                                    uint8_t unit,
                                                    uint8_t sensoridx ); 

    /*!
        Convert a floating point measurement value into VSCP data as a
        single precision float (32-bit) for
     *  VSCP_CLASS1_MEASUREMENT, VSCP_CLASS2_LEVEL1.MEASUREMENT
        @param value Floating point value to convert.
        @param pdata Pointer to beginning of VSCP returned event data.
        @param psize Pointer to size for returned data.
        @param unit Unit for the data. Zero is default.
        @param sensoridx Sensor index 0-7. Zero is default.
        @return true on success, false on failure.
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
      @param value integer value to convert.
      @param pdata Pointer to beginning of VSCP returned event data.
      @param psize Pointer to size for returned data.
      @param unit Unit for the data. Zero is default.
      @param sensoridx Sensor index 0-7. Zero is default.
      @return true on success, false on failure.
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
        @param value Floating point value to convert.
        @param pEvent Pointer to event with pdata set to NULL. VSCP_class and
        vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
        @param psize Pointer to size for returned data.
        @param unit Unit for the data. Zero is default.
        @param sensoridx Sensor index 0-7. Zero is default.
        @return true on success, false on failure.
    */
    bool vscp_makeFloatMeasurementEvent( vscpEvent *pEvent, 
                                            float value,
                                            uint8_t unit,
                                            uint8_t sensoridx );

    /*!
        Convert a floating point measurement value into VSCP data as a
        string for CLASS1.MEASUREMENT or CLASS2_LEVEL1.MEASUREMENT
        @param value Floating point value to convert.
        @param pEvent Pointer to event with pdata set to NULL. vscp_class and
        vscp_type must be set to CLASS1.MEASUREMENT or CLASS2_LEVEL1.MEASUREMENT,
        @param psize Pointer to size for returned data.
        @param unit Unit for the data. Zero is default.
        @param sensoridx Sensor index 0-7. Zero is default.
        @return true on success, false on failure.
    */
    bool vscp_makeStringMeasurementEvent( vscpEvent *pEvent,
                                          double value,
                                          uint8_t unit,
                                          uint8_t sensoridx );
    
    /*!
     * Construct CLASS2.MEASUREMENT_FLOAT level II measurement event 
     * 
     * @param pEvent Pointer to event that will be filled with data or NULL
     *          if event should be allocated.
     * @param value Floating point value that event should be made from.
     * @param unit Unit for this measurement. 0-255. Default is zero.
     * @param sensorsidx Sensor index for this measurement. 0-255. Default us zero.
     * @param zone Zone for this measurement. 0-255. Zero is default.
     * @param Subzone Sub zone for this measurement. 0-255- Zero is default.
     * @return True is returned on success, false if an error occurred.
     */
    bool vscp_makeLevel2FloatMeasurementEvent( vscpEvent *pEvent, 
                                                    uint16_t type,
                                                    double value,
                                                    uint8_t unit,
                                                    uint8_t sensoridx,
                                                    uint8_t zone,
                                                    uint8_t subzone );
    
    /*!
     * Construct CLASS2.MEASUREMENT_STRING level II measurement event 
     * 
     * @param pEvent Pointer to event that will be filled with data or NULL
     *          if event should be allocated.
     * @param value Floating point value that event should be made from.
     * @param unit Unit for this measurement. 0-255. Default is zero.
     * @param sensorsidx Sensor index for this measurement. 0-255. Default us zero.
     * @param zone Zone for this measurement. 0-255. Zero is default.
     * @param Subzone Sub zone for this measurement. 0-255- Zero is default.
     * @return True is returned on success, false if an error occurred.
     */
    bool vscp_makeLevel2StringMeasurementEvent( vscpEvent *pEvent,
                                                    uint16_t type,
                                                    double value,
                                                    uint8_t unit,
                                                    uint8_t sensoridx,
                                                    uint8_t zone,
                                                    uint8_t subzone );


    /*!
      Get data in the VSCP data coding set as float format to a float
      @param pNorm Pointer to VSCP event.
      @param length Number of bytes it consist of including data coding byte
      @return value as float
     */
    float vscp_getMeasurementAsFloat(const unsigned char *pNorm,
                                        const unsigned char length);
    /*!
     * Convert a Level I measurement event to a Level II double measurement event
     * VSCP_CLASS2_MEASUREMENT_FLOAT
     * 
     * @param pEventLevel1 Pointer to level I measurement event to be converted.
     * @param pEventÖevel2 Pointer to resulting level II measurement event.
     * @return true on success, false otherwise.
     */
    bool vscp_convertLevel1MeasuremenToLevel2Double( vscpEvent *pEventLevel1 );
    
    /*!
     * Convert a Level I measurement event to a Level II string measurement event
     * VSCP_CLASS2_MEASUREMENT_STRING
     * 
     * @param pEventLevel1 Pointer to level I measurement event to be converted.
     * @param pEventÖevel2 Pointer to resulting level II measurement event.
     * @return true on success, false otherwise.
     */
    bool vscp_convertLevel1MeasuremenToLevel2String( vscpEvent *pEventLevel1 );
    
    
    
    
    
    
    // -------------------------------------------------------------------------
    
    
    
    
    
    /*!
      Replace backslashes in a string with forward slashes
      @param strval String that should be handled.
      @return Fixed string.
     */
    wxString& vscp_replaceBackslash( wxString& strval );



    /*!
      Get VSCP priority
      @param pEvent Pointer to VSCP event to set priority for.
      @return Priority (0-7) for event.
     */
    unsigned char vscp_getVscpPriority(const vscpEvent *pEvent);

    /*!
      Get VSCP priority
      @param pEvent Pointer to VSCP event to set priority for.
      @return Priority (0-7) for event.
     */
    unsigned char vscp_getVscpPriorityEx(const vscpEventEx *pEvent);

    /*!
      Set VSCP priority
      @param pEvent Pointer to VSCP event to set priority for.
      @param priority Priority (0-7) to set.
     */
    void vscp_setVscpPriority(vscpEvent *pEvent, unsigned char priority);
    
     /*!
      Set VSCP priority Ex
      @param pEvent Pointer to VSCP event to set priority for.
      @param priority Priority (0-7) to set.
     */
    void vscp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority);

    /*!
      Get VSCP head from CANAL id
      @param id CAN id
      @return VSCP head
     */
    unsigned char vscp_getVSCPheadFromCANALid(uint32_t id);

    /*!
     Get VSCP class from CANAL id
      @param id CAN id
      @return VSCP class
     */
    uint16_t vscp_getVSCPclassFromCANALid(uint32_t id);

    /*!
      Get VSCP type from CANAL id
      @param id CAN id
     @return VSCP type
     */
    uint16_t vscp_getVSCPtypeFromCANALid(uint32_t id);

    /*!
     Get VSCP nide nickname from CANAL id
      @param id CAN id
      @return VSCP node nickname
     */
    uint8_t vscp_getVSCPnicknameFromCANALid(uint32_t id);

    /*!
      Get CANAL id from VSCP info
      @param priority VSCP priority
      @param vscp_class VSCP class
      @param vscp_type VSCP type
      @return CAN id with nickname == 0
     */
    uint32_t vscp_getCANALidFromVSCPdata(unsigned char priority,
                                            const uint16_t vscp_class,
                                            const uint16_t vscp_type);

    /*!
      Get CANAL id from VSCP event
      @param pEvent Pointer to VSCP event
      @return CAN id with nickname == 0
     */
    uint32_t vscp_getCANALidFromVSCPevent(const vscpEvent *pEvent);

    /*!
      Get CAN id from VSCP event
      @param pEvent Pointer to VSCP event
      @return CAN id with nickname == 0
     */
    uint32_t vscp_getCANALidFromVSCPeventEx(const vscpEventEx *pEvent);

    /*!
      Calculate CRC for VSCP event
     */
    unsigned short vscp_calc_crc_Event(vscpEvent *pEvent, short bSet);


    /*!
      Calculate CRC for VSCP event
     */
    unsigned short vscp_calc_crc_EventEx(vscpEventEx *pEvent, short bSet);

    
    
    // Helpers
    


    /*!
        calcCRC4GUIDArray

        @param Pointer to GUID array (MSB-LSB order)
        \return crc for GUID.
    */
    uint8_t vscp_calcCRC4GUIDArray(const uint8_t *pguid);

    /*!
        calcCRC4GUIDString

        @param Pointer to GUID array (MSB-LSB order)
        \return crc for GUID.
    */
    uint8_t vscp_calcCRC4GUIDString(const wxString &strguid);


    /*!
      Get GUID from string

      @param pEvent Pointer to VSCP event
      @param strGUID String with GUID (xx:yy:zz....)
      @return True on success, false on failure.
    */
 
    bool vscp_getGuidFromString(vscpEvent *pEvent, const wxString& strGUID);

    
    /*!
      Get GUID from string

      @param pEvent Pointer to VSCP event
      @param strGUID String with GUID (xx:yy:zz....)
      @return True on success, false on failure.
    */
 
    bool vscp_getGuidFromStringEx(vscpEventEx *pEventEx, 
                                    const wxString& strGUID);


    /*!
      Fill event GUID from a string
     */
 
    bool vscp_getGuidFromStringToArray(unsigned char *pGUID, 
                                        const wxString& strGUID);

    /*!
      Write out GUID to string

      @param pGUID Pointer to VSCP GUID array.
      @param strGUID Reference to string for written GUID
      @return True on success, false on failure.
    */

    bool vscp_writeGuidArrayToString(const unsigned char *pGUID, 
                                        wxString& strGUID);


    /*!
      Write out GUID to string

      @param pEvent Pointer to VSCP event
      @param strGUID Reference to string for written GUID
      @return True on success, false on failure.
     */

    bool vscp_writeGuidToString(const vscpEvent *pEvent,        
                                    wxString& strGUID);

    
        /*!
      Write out GUID to string

      @param pEvent Pointer to VSCP event
      @param strGUID Reference to string for written GUID
      @return True on success, false on failure.
     */

    bool vscp_writeGuidToStringEx(const vscpEventEx *pEvent,            
                                    wxString& strGUID);


    /*!
      Write out GUID to string as four rows

      @param pEvent Pointer to VSCP event
      @param strGUID Reference to string for written GUID
      @return True on success, false on failure.
     */
 
    bool vscp_writeGuidToString4Rows(const vscpEvent *pEvent,   
                                        wxString& strGUID);

    
    /*!
      Write out GUID to string as four rows

      @param pEvent Pointer to VSCP event
      @param strGUID Reference to string for written GUID
      @return True on success, false on failure.
    */
    
    bool vscp_writeGuidToString4RowsEx(const vscpEventEx *pEvent, 
                                        wxString& strGUID);



    /*!
      Check if GUID is all null
      @param pGUID pointer to GUID to check
      @return true if empty, false if not.
     */
    bool vscp_isGUIDEmpty(const unsigned char *pGUID);

    /*!
      Compare two GUID's
      @param pGUID1 First GUID to compare
      @param pGUID2 Second GUID to compare
      @return True if the two GUID's are equal. False otherwise.
     */
    bool vscp_isSameGUID(const unsigned char *pGUID1, 
                            const unsigned char *pGUID2);

    /*!
        Reverse GUID
        @param pGUID Pointer to GUID to reverse.
        @return true if OK.
     */
    bool vscp_reverseGUID(unsigned char *pGUID);

    /*!
      Convert a standard VSCP event to the Ex version
     */
    bool vscp_convertVSCPtoEx(vscpEventEx *pEventEx, 
                                const vscpEvent *pEvent);

    /*!
      Convert an Ex event to a standard VSCP event
     */
    bool vscp_convertVSCPfromEx(vscpEvent *pEvent, 
                                    const vscpEventEx *pEventEx);

    /*!
      Delete a standard VSCP event
     */
    void vscp_deleteVSCPevent(vscpEvent *pEvent);

    /*!
      Delete an Ex event
     */
    void vscp_deleteVSCPeventEx(vscpEventEx *pEventEx);
    
    
    
    
    
    /*!
     * Convert VSCP Event to JSON formated string
     */
    void vscp_convertEventToJSON( vscpEvent *pEvent, wxString& strJSON );
    
    /*!
     * Convert VSCP EventEx to JSON formated string
     */
    void vscp_convertEventExToJSON( vscpEventEx *pEventEx, wxString& strJSON );
    
    
    
    
    
    /*!
     * Convert VSCP Event to XML formated string
     */
    void vscp_convertEventToXML( vscpEvent *pEvent, wxString& strXML );
    
    /*!
     * Convert VSCP EventEx to XML formated string
     */
    void vscp_convertEventExToXML( vscpEventEx *pEventEx, wxString& strXML );
    
    
    
    
    
    /*!
     * Convert VSCP Event to HTML formated string
     */
    void vscp_convertEventToHTML( vscpEvent *pEvent, wxString& strHTML );
    
    /*!
     * Convert VSCP EventEx to HTML formated string
     */
    void vscp_convertEventExToHTML( vscpEventEx *pEventEx, wxString& strHTML );
    
    
    
    
    
     /*!
     * Convert VSCP Event to XML formated string
     */
    void vscp_convertEventToXML( vscpEvent *pEvent, wxString& strXML );
    
    /*!
     * Convert VSCP EventEx to XML formated string
     */
    void vscp_convertEventExToXML( vscpEventEx *pEventEx, wxString& strXML );
    

    
    
    
    /*!
      Clear VSCP filter so it will allow all events to go through
      @param pFilter Pointer to VSCP filter.
     */
    void vscp_clearVSCPFilter(vscpEventFilter *pFilter);
    
    /*!
     * Copy filter from one filter to another
     * @param pToFilter Pointer to filter to copy data to
     * @param pFromFilter Pinter to filter to copy data from
     */  
    void vscp_copyVSCPFilter( vscpEventFilter *pToFilter, const vscpEventFilter *pFromFilter);

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

      Filter tells the value for valid bits. If filter bit is == 1 the bits
      must be equal to get a true filter return.

      So a nill mask will let everything through
      @return true if message should be delivered false if not.
     */
    bool vscp_doLevel2Filter(const vscpEvent *pEvent,
            const vscpEventFilter *pFilter);

    bool vscp_doLevel2FilterEx(const vscpEventEx *pEventEx,
            const vscpEventFilter *pFilter);

    /*!
        Read a filter from a string
        If strFilter is an empty string all elements in filter will be set to zero. Arguments is
        priority,class,type,GUID and all is optional but if given must be given in order.
        @param pFilter Filter structure to write filter to.
        @param strFilter Filter in string form 
                filter-priority, filter-class, filter-type, filter-GUID
        @return true on success, false on failure.
     */
 
    bool vscp_readFilterFromString( vscpEventFilter *pFilter, const wxString& strFilter);

    /*!
        Write filter to string
        @param pFilter Filter structure to write out to string.
        @param strFilter Filter in string form 
                filter-priority, filter-class, filter-type, filter-GUID
        @return true on success, false on failure.
    */
    bool vscp_writeFilterToString( const vscpEventFilter *pFilter, wxString& strFilter);

    /*!
        Read a mask from a string
        If strMask is an empty string elements in mask will be set to zero. Arguments is
        priority,class,type,GUID and all is optional but if given must be given in order.
        @param pFilter Filter structure to write mask to.
        @param strMask Mask in string form 
                mask-priority, mask-class, mask-type, mask-GUID
        @return true on success, false on failure.
     */
 
    bool vscp_readMaskFromString( vscpEventFilter *pFilter, const wxString& strMask);

    /*!
        Write mask to string
        @param pFilter Filter structure to write out to string.
        @param strFilter Mask in string form 
                mask-priority, mask-class, mask-type, mask-GUID
        @return true on success, false on failure.
    */
    bool vscp_writeMaskToString( const vscpEventFilter *pFilter, wxString& strFilter);

    /*!
      Convert an Event from a CANAL message
     */
    bool vscp_convertCanalToEvent(vscpEvent *pvscpEvent,
                                     const canalMsg *pcanalMsg,
                                     unsigned char *pGUID );
    
    /*!
      Convert an Event from a CANAL message
     */
    bool vscp_convertCanalToEventEx(vscpEventEx *pvscpEvent,
                                       const canalMsg *pcanalMsg,
                                       unsigned char *pGUID );

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
        @return Event timestamp as an unsigned long
     */
    unsigned long vscp_makeTimeStamp(void);


    /*!
      Copy a VSCP event to another

      @param pEventTo Pointer to event to copy to.
      @param pEventFrom Pointer to event to copy from.
      @return True on success.
    */
    bool vscp_copyVSCPEvent(vscpEvent *pEventTo, const vscpEvent *pEventFrom);
    

    /*!
      Write VSCP data to string    DEPRECATED: USE: vscp_writeVscpDataWithSizeToString
      @param pEvent Pointer to event where data is fetched from
      @param str String that receive result.
      @param bUseHtmlBreak Set to true to use <br> instead of \\n as
      line break 
      @return True on success false on failure.
     */

    bool vscp_writeVscpDataToString(const vscpEvent *pEvent, 
                                        wxString& str, 
                                        bool bUseHtmlBreak = false);


    /*!
      Write VSCP data to string
      @param sizeData Number of data bytes.
      @param pData Pointer to data structure.
       @param str String that receive result.
      @param bUseHtmlBreak Set to true to use <br> instead of \\n as
      line break 
      @return True on success false on failure.
    */

    bool vscp_writeVscpDataWithSizeToString(const uint16_t sizeData,
                                                const unsigned char *pData,
                                                wxString& str,
                                                bool bUseHtmlBreak = false,
                                                bool bBreak = true );

    /*!
      Set VSCP data from a string
      @param pEvent Pointer to a VSCP event to write parsed data to.
      @param str A string with comma or whitespace separated data in decimal
      or hexadecimal form. Data can span multiple lines.
      @return true on success, false on failure.
    */
 
    bool vscp_setVscpDataFromString(vscpEvent *pEvent, const wxString& str);


    /*!
      Set VSCP data from a string
      @param pData Pointer to a unsigned byte array to write parsed data to.
      @param psizeData Number of data bytes. 
      @param str A string with comma or whitespace separated data in decimal
      or hexadecimal form. Data can span multiple lines.
      @return true on success, false on failure.
    */
 
    bool vscp_setVscpDataArrayFromString( uint8_t *pData, 
                                            uint16_t *psizeData, 
                                            const wxString& str );


    /*!
      Write event to string.
      priority,class,type,guid,data
      @param pEvent Event that should be presented
      @param str String that receive the result
      @return true on success, false on failure.
    */
 
    bool vscp_writeVscpEventToString( const vscpEvent *pEvent, wxString& str);


    /*!
      Write event to string.
      priority,class,type,guid,data
      @param pEvent Event that should be presented
      @param str String that receive the result
      @return true on success, false on failure.
     */
 
    bool vscp_writeVscpEventExToString( const vscpEventEx *pEvent, wxString& str);



    /*!
      Get event data from string format
      Format: head,class,type,obid,timestamp,GUID,data1,data2,data3....
      @param pEvent Event that will get data
      @param str String that contain the event on string form
      @return true on success, false on failure.
     */
 
    bool vscp_setVscpEventFromString( vscpEvent *pEvent, const wxString& str);


    /*!
      Get event data from string format
      Format: head,class,type,obid,timestamp,GUID,data1,data2,data3....
      @param pEventEx Pointer to VSCP event that will get the parsed data
      @param str String that contain the event on string form
      @return true on success, false on failure.
     */
 
    bool vscp_setVscpEventExFromString( vscpEventEx *pEventEx, const wxString& str);

    /*!
        Write in measurement data into real text string             TODO REMOVE !!!!!!!!!
        @param vscptype VSCP type
        @param unit Measurement unit, 0-3 for Level I, 0-255 for Level II.
        @param sensoridx Index for sensor, 0-7 for Level I, 0-255 for Level II.
    */
    wxString& writeMeasurementValue( uint16_t vscptype,
                                        uint8_t unit,
                                        uint8_t sensoridx,
                                        wxString& strValue );

    /*!
      Get Data in real text.
      This for measurement class events that can be describes in real text
      in a deterministic way. Temperature event is typical which can be returned
      in clear text as "Temperature = 22.5 C".
      @param pEvent Event that should be presented
      @return Text data representation of the event data or an empty string 
      if the class/type pair is not supported..
     */
 
    wxString& vscp_getRealTextData(vscpEvent *pEvent);


    /*!
      This function makes a HTML string from a standard string. LF is replaced
      with a '<BR>'.
      @param str String that should be HTML coded.
     */
 
    void vscp_makeHtml(wxString& str);


    /*
        Get device HTML status from device
        @param registers Array with all 256 registers for the device
        @param pmdf Optional pointer to CMDF class which gives more info
        about the device if it is supplied.
    */
    wxString& vscp_getDeviceHtmlStatusInfo( const uint8_t *registers, CMDF *pmdf );




#ifdef __cplusplus
}
#endif

#endif // #if !defined(AFX_VSCPHELPER_H__INCLUDED_)

