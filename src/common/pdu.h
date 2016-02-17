/********************************************************************
    created:	2006/11/22
    created:	22:11:2006   10:04
    author:		Louis huang<hlouis@gmail.com>
    purpose:	GSM SMS PDU format manipulate class
    Copyright (C) 2006-2008 Louis hunag

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

   http://www.codeproject.com/useritems/PDU_lib.asp
*********************************************************************/

/************************************************************************
    PDU FORMAT BRIEFING :)

    Send pdu message:
    <SMSC> + <PHONE> + <USERDATA>
    SMSC:		Message service center number part
    PHONE:		The destination phone number part
    USERDATA:	The message need to send
                [one octet indicate the message length] + [data]

************************************************************************/

#pragma once

#include <string>
#include <stl/tstl.h>
#include <atltime.h>		// Use some alt classes
#include <iostream>

class CPDUException
{
public:
    explicit CPDUException(std::tstring msg, DWORD dwError) :
    m_msg(msg), m_dwError(dwError)
    {
        return;
    }

    std::tstring GetMessage() { return m_msg; }
    DWORD GetCode() { return m_dwError; }

protected:
    std::tstring	m_msg;
    DWORD			m_dwError;
};

#define COUNTRY_NUMBER "86"				// So here is china:)

class CPDU
{
public:
    // Predefine data:
    typedef std::string pdudata;		// PDU string type

    typedef enum
    {
        EEncode_UNKNOW	= -1,
        EEncode_7bit	= 0x00,
        EEncode_8bit	= 0x04,
        EEncodeUCS2		= 0x08
    }
    EEncodeMethod;						// User data encoding method

    typedef enum
    {
        ESMSWay_DELIVER		= 0,		// I am the callee/receiver
        ESMSWay_SUBMIT,					// I am the caller/sender
        ESMSWay_UNKNOW
    }
    ESMSWay;

public:
    CPDU(void);
public:
    ~CPDU(void);

public:
    /**
     *	Set the message service center number
     *	@para string szNum: the new message number
     *
     *	@return void
     */
    void SetMSCNumber (std::tstring szNum);

    /**
     *	Set the send message validity period time
     *	@para long days: max 441 day
     *	@para int hours, mins
     *
     *	@return void
     */
    void SetValidityPeriod(long days, int hours, int mins);
    
    /**
     *	Compose a shot message
     *	@para pdudata pdu: [out] The pdu result
     *	@para string msg:	data need to be sent
     *	@para string phone:	the destination phone number
     *	@para string msc:	message center number, if null, use the preset number
     *	@para EEncodeMethod eMethod:	the encode method, default is UCS2
     *
     *	@return the data length without the SMSC part[you need it in AT+CMGS command]
     */
    int Compose (pdudata& pdu, 
        std::tstring msg, std::tstring phone,
        std::tstring msc = _T(""),
        EEncodeMethod eMethod = EEncodeUCS2) throw(...);

    /**
     *	Fetch data form a pdu format data
     *	only return the message part(USERDATA), use other member function to fetch information
     *	@para pdudata pdu: the source data
     *
     *	@return The nomral message
     */
    std::tstring Fetch(pdudata pdu) throw(...);

    /**
     *	Fetch Data
     */
    std::tstring GetCaller()		{ return m_szCaller; }
    std::tstring GetCallee()		{ return m_szCallee; }
    std::tstring GetTimeStamp()		{ return m_szTimeStamp; }

protected:
    std::tstring	m_szMSC;		// Message center number
    pdudata			m_pduMSC;		// PDU format MSC part
    std::tstring	m_szCaller;		// The caller/sender number
    std::tstring	m_szCallee;		// The callee/receiver number
    EEncodeMethod	m_eMethod;		// Data encoding method

    CTimeSpan	m_tpvp;			// Message validity period
    int			m_pduTpvp;		// Above data pdu format
    CTime		m_timeStamp;	// Time stamp

    // Received paras:
    bool		m_bTP_RP;		// Reply path. Parameter indicating that reply path exists.
    bool		m_bTP_UDHI;		// User data header indicator. This bit is set to 1 if the User Data field starts with a header
    bool		m_bTP_SRI;		// Status report indication. This bit is set to 1 if a status report is going to be returned to the SME
    bool		m_bTP_MMS;		// More messages to send. This bit is set to 0 if there are more messages to send
    ESMSWay		m_wayTP_MMI;	// The message direction
    std::tstring m_szTimeStamp;	// The message time stamp

protected:
    /**
     *	Encode USERDATA to pdu format by 7bit method
     *	The input pdu data is with length info (length is two bytes long)
     *	@para [out]pdudata pdu: output the final data
     *	@para [in]tstring message: the message need to be encoded
     *
     *	@return pdu length, if failed, return 0
     */
    int Encode7bit(pdudata& pdu, const std::tstring message);

    /**
    *	Decode USERDATA from pdu format by 7bit method
    *	The output pdu data is length+data (length is two bytes long)
    *	@para [in]pdudata pdu: pdu data need to decode
    *	@para [out]tstring message: the output message
    *
    *	@return the data length
    */
    int Decode7bit(const pdudata pdu, std::tstring& message);

    /**
    *	Encode USERDATA to pdu format by UCS2 method
    *	The input pdu data is with length info (length is two bytes long)
    *	@para [out]pdudata pdu: output the final data
    *	@para [in]tstring message: the message need to be encoded
    *
    *	@return pdu length, if failed, return 0
    */
    int EncodeUCS2(pdudata& pdu, std::tstring message);

    /**
    *	Decode USERDATA from pdu format by UCS2 method
    *	The output pdu data is length+data (length is two bytes long)
    *	@para [in]pdudata pdu: pdu data need to decode
    *	@para [out]tstring message: the output message
    *
    *	@return the data length
    */
    int DecodeUCS2(const pdudata pdu, std::tstring& message);

    /**
     *	Encode a normal phone number to PDU format number
     *	@para string number: number need to encode
     *	@para pdudata pduNum: PDU format number
     *
     *	@return The number real length.
     */
    int EncodePDUNumber(std::tstring number, pdudata& pduNum);

    /**
     *	Decode the PDU format number to normal number
     *	This function will automatic ignore the "91" prefix
     *	@para pdudata pNumber:	The number need to decode
     *
     *	@return string, normal number, if failed ,return null.
     */
    std::tstring DecodePDUNumber(pdudata pNumber);

    /**
     *	Parse the given parameter to the class attributes
     *	@para long iPara: the parameter
     *
     *	@return void
     */
    void ParseSMSPara(long iPara);

    /**
     *	Create the SMS parameter according the class attributes
     *
     *	@return The parameter
     */
    long CreateSMSPara();

    /**
     *	Parse the time stamp pdu part
     *	@para pdudata pdu: the time stamp pdu part
     *
     *	@return tstring of the time
     */
    std::tstring ParseTimeStamp(pdudata pdu);
};
