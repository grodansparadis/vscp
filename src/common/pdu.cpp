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
*********************************************************************/
#define STRICT
#include <crtdbg.h>
#include <tchar.h>
#include <windows.h>

#include <sstream>
#include <iomanip>
#include <bitset>
#include <vector>
#include <algorithm>
#include "PDU.h"

CPDU::CPDU(void)
{
    m_pduTpvp   = 0;        // default is 5 min.
    m_bTP_RP    = false;
    m_bTP_UDHI  = false;
    m_bTP_SRI   = false;
    m_bTP_MMS   = false;
    m_wayTP_MMI = ESMSWay_UNKNOW;
    m_eMethod   = EEncode_UNKNOW;
}

CPDU::~CPDU(void)
{

}


//////////////////////////////////////////////////////////////////////////
// Methods
int CPDU::Compose(CPDU::pdudata& pdu,
                  std::tstring msg, std::tstring phone,
                  std::tstring msc /* = _T("") */, EEncodeMethod eMethod /* = EEncodeUCS2 */) throw(...)
{
    std::tcout.imbue(std::locale("chs"));
    std::tcout << _T("Compose a message:\n");

    if (!msc.empty())
        SetMSCNumber(msc);
    else if (m_szMSC.empty())
        throw CPDUException (_T("No Message center number"), 1);

    if (phone.empty())
        throw CPDUException (_T("No target user number"), 1);

    std::stringstream ssPdu;
    // First the MSC part
    ssPdu << std::hex << std::setw(2) << std::setfill('0') << (int)(m_pduMSC.length() / 2);
    ssPdu << m_pduMSC;

    // Start build the phone part
    std::stringstream ssPhone;
    ssPhone << "1100";              // The static phone part header, see document for detail
    pdudata pduPhone;
    ssPhone << std::hex << std::setw(2) << std::setfill('0') << EncodePDUNumber(phone, pduPhone);   // Number length
    ssPhone << "91" << pduPhone;    // phone and 91 prefix

    ssPhone << "00";                // The Protocol identifier 00 for SMS.
    ssPhone << std::hex << std::setw(2) << std::setfill('0') << eMethod;
    ssPhone  << std::hex << std::setw(2) << std::setfill('0') << m_pduTpvp;         // The validity period


    // Start build the USERDATA part
    pdudata pduData;
    if (eMethod == EEncodeUCS2)
    {
        if (msg.length() > 70)
            throw CPDUException (_T("Message text number too large!"), 2);
        EncodeUCS2(pduData, msg);
    }

    ssPhone << pduData;
    pduData = ssPhone.str();
    int iLen = (int)pduData.length() / 2;

    std::cout << "The data len is:\t" << iLen << '\n';

    ssPdu << pduData;
    pdu = ssPdu.str();

    std::cout << "The final pdu is:\n" << pdu
              << "\n------------------------------------------------\n"
              << std::endl;
    
    return iLen;
}

int CPDU::EncodeUCS2(pdudata& pdu, std::tstring message)
{
    size_t iLen = message.length();
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << iLen * 2;
    for (size_t i = 0; i < iLen; ++i)
        ss << std::hex << std::setw(4) << std::setfill('0') << message[i];

    pdu = ss.str();
    std::cout << "User data is:\t\t" << pdu << std::endl;
    return (int)iLen * 2;
}

std::tstring CPDU::Fetch(pdudata pdu) throw (...)
{
    std::tcout.imbue(std::locale("chs"));
    std::tcout << _T("Fetch data:\n");
//	std::cout << "Begin to fetch message from:\n" << pdu << '\n';
    int iPduLen = (int)pdu.length();
    int iOffset = 0;

    // First get the msc part
    int iLenSmsc = 0;
    std::stringstream ss(pdu.substr(iOffset, 2));
    ss >> std::hex >> iLenSmsc;
    iLenSmsc = iLenSmsc * 2;
    iOffset += 2;

    SetMSCNumber(DecodePDUNumber(pdu.substr(2, iLenSmsc)));
    std::tcout << _T("MSCS number is:\t\t") << m_szMSC << '\n';
    iOffset += iLenSmsc;

    // Parse the parameters
    ss.clear();
    ss.str(pdu.substr(iOffset, 2));
    long iPara = 0;
    ss >> std::hex >> iPara;
    ParseSMSPara(iPara);
    iOffset += 2;

    if (m_wayTP_MMI == ESMSWay_SUBMIT)
        iOffset += 2;		// jump over the message reference part if the type is submit message

    if (m_bTP_RP)		// There is replay number so parse it
    {
        ss.clear();
        ss.str(pdu.substr(iOffset, 2));
        int iLen = 0;
        ss >> std::hex >> iLen;
        iOffset += 2;

        iOffset += 2;		//Jump over the number format part (91)
        m_szCaller = DecodePDUNumber(pdu.substr(iOffset, iLen + 1));
        std::tcout << _T("The caller/Sender is:\t") << m_szCaller << '\n';
        iOffset += iLen + 1;
    }

    iOffset += 2;		// Jump over the Protocol identifier part.

    // Get the data encoding method
    int iMethod = 0;
    ss.clear();
    ss.str(pdu.substr(iOffset, 2));
    ss >> std::hex >> iMethod;
    m_eMethod = (EEncodeMethod)iMethod;
    iOffset += 2;

    if (m_wayTP_MMI == ESMSWay_DELIVER)
    {
        m_szTimeStamp = ParseTimeStamp(pdu.substr(iOffset, 14));
        iOffset += 14;
        std::tcout << _T("The time stamp is:\t") << m_szTimeStamp << '\n';
    }
    else if (m_wayTP_MMI == ESMSWay_SUBMIT)
        iOffset += 2;	// Jump over the TP-Validity-Period part

    // Decode the user data
    std::tstring szUserData;
    if (m_eMethod == EEncodeUCS2)
        DecodeUCS2(pdu.substr(iOffset), szUserData);
    else if (m_eMethod == EEncode_7bit)
        Decode7bit(pdu.substr(iOffset), szUserData);

    std::tcout << _T("The user data is:\n") << szUserData
               << _T("\n------------------------------------------------\n")
               << std::endl;
    return szUserData;
}

std::tstring CPDU::ParseTimeStamp(pdudata pdu)
{
    std::tstringstream ss;
    ss << _T("20");
    ss << pdu[1] << pdu[0];	//year
    ss << ' ' << pdu[3] << pdu[2]; //month
    ss << '/' << pdu[5] << pdu[4]; //day
    ss << ' ' << pdu[7] << pdu[6]; //Hour
    ss << ':' << pdu[9] << pdu[8]; //min
    ss << ':' << pdu[11] << pdu[10]; //sec

    int iGMT = 0;
    std::stringstream ss2(pdu.substr(12, 2));
    ss2 >> std::hex >> iGMT;
    ss << _T(" GMT+") << iGMT / 4;

    return ss.str();
}

void CPDU::ParseSMSPara(long iPara)
{
    std::bitset<8> bit (iPara);
    m_bTP_RP	= !bit[7];			// TODO: make sure this message meaning
    m_bTP_UDHI	= bit[6];
    m_bTP_SRI	= bit[5];
    m_bTP_MMS	= bit[2];

    if (!bit[1] && bit[0])
        m_wayTP_MMI = ESMSWay_SUBMIT;
    else if (!bit[1] && !bit[0])
        m_wayTP_MMI = ESMSWay_DELIVER;
    else
        m_wayTP_MMI = ESMSWay_UNKNOW;
}

int CPDU::DecodeUCS2(const pdudata pdu, std::tstring& message)
{
    int iLen = 0;
    std::stringstream ss(pdu.substr(0, 2));
    ss >> std::hex >> iLen;

    int iChar = 0;
    std::tstringstream tss;
    for (int i = 0; i < iLen / 2; ++i)
    {
        ss.clear();
        ss.str(pdu.substr(2 + i * 4, 4));
        ss >> std::hex >> iChar;
        tss << (TCHAR)iChar;
    }
    message = tss.str();

// 	std::cout << "The pdu length is: " << iLen << '\n';
// 
// 	std::tcout.imbue(std::locale("chs"));
// 	std::tcout << _T("The message is:") << message << std::endl;

    return iLen;
}

void CPDU::SetMSCNumber(std::tstring szNum)
{
    m_szMSC = szNum;
    EncodePDUNumber(szNum, m_pduMSC);
    m_pduMSC = "91" + m_pduMSC;
//	std::cout << "PDU MSC number is:\t" << m_pduMSC << std::endl;
}


std::tstring CPDU::DecodePDUNumber(pdudata pNumber)
{
    pdudata pduTemp = pNumber.substr(0, 2);
    int iStart = 0;
    if (pduTemp == "91")
        iStart = 2;

    std::tstringstream tss;
    for (int i = iStart; i < (int)pNumber.length(); i+=2)
    {
        tss << pNumber[i + 1];
        tss << pNumber[i];
    }

    std::tstring szNumber = tss.str();
    
    if (szNumber[szNumber.length() - 1] == 'f' || szNumber[szNumber.length() - 1] == 'F')
        szNumber.erase(szNumber.length() - 1);
    return szNumber;
}

int CPDU::EncodePDUNumber(std::tstring number, pdudata& pduNum)
{
    if (number[0] == '+')
        number = number.substr(1);

    if (number.substr(0, 2) != _T(COUNTRY_NUMBER))
        number = _T(COUNTRY_NUMBER) + number;

    int iLen = (int)number.length();

    if ( (iLen % 2) != 0)
        number += 'F';

    std::stringstream ss;
    int iGap = 1;
    for (size_t i = 0; i < number.length(); ++i)
    {
        int test = number[i + iGap];
        ss << ss.narrow(number[i + iGap]);		// Only convert number here, so no data lose possible
        iGap = (i % 2 == 0) ? -1 : 1;
    }

    pduNum = ss.str();
    return iLen;
}

void CPDU::SetValidityPeriod(long days, int hours, int mins)
{
    if (days > 30)
    {
        if (days > 441)
            days = 441;
        m_pduTpvp = (days / 7) + 192;
    }
    else if (days >= 1)
        m_pduTpvp = days + 166;
    else if (hours >= 12)
    {
        if (hours >= 24)
            hours = 23;
        m_pduTpvp = (hours - 12) * 2 + 143;
    }
    else
    {
        if (mins >= 60)
            mins = 59;

        m_pduTpvp = hours * 12 + mins / 5 - 1;
        if (m_pduTpvp < 0) m_pduTpvp = 0;
    }

    m_tpvp = CTimeSpan(days, hours, mins, 0);
}

int CPDU::Decode7bit(const pdudata pdu, std::tstring& message)
{
    long iLeft = 0;
    std::stringstream ss;
    std::tstringstream tss;
    int iCount = 0, iChar = 0;
    int i = 0;
    int iLen = (int)pdu.length() / 2 - 1;	// minus 1 for the data length part
    for (; i < iLen; ++i)
    {
        ss.str(pdu.substr(2 + i * 2, 2));	// plus 2 to jump over the data length part
        ss >> std::hex >> iChar;
        ss.clear();

        iCount++;
        if (iCount > 7) iCount = 1;

        std::bitset<8> bit(iChar);
        std::bitset<8> bitTemp = bit >> (8 - iCount);
        std::bitset<8> bitLeft(iLeft);
        bit <<= iCount;
        bit >>= 1;
        bit |= bitLeft;
        iLeft = bitTemp.to_ulong();

        char charTemp = (char)bit.to_ulong();
        tss << charTemp;

        if (iCount == 7)	// output the extra byte
            tss << (char)bitTemp.to_ulong();
    }

    message = tss.str();
//	std::tcout << message << std::endl;

    return i;
}

int CPDU::Encode7bit(pdudata& pdu, const std::tstring msg)
{
    int iLeft = 0;
    std::vector<long> vect;
    int iLen = (int)msg.length();
    int iCount = iLen % 8;

    for (int i = iLen - 1; i >= 0; --i)
    {
        std::bitset<8> bit(msg[i]);

        std::bitset<8> bitTemp = bit << (8 - iCount + 1);

        bit >>= (iCount - 1);
        std::bitset<8> bitLeft(iLeft);
        bit |= bitLeft;

        iLeft = bitTemp.to_ulong();
        if (iCount != 8)
            vect.push_back(bit.to_ulong());

        iCount--;
        if (iCount < 1) iCount = 8;
    }

    std::reverse(vect.begin(), vect.end());
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << iLen;
    for (int i = 0; i < (int)vect.size(); ++i)
        ss << std::hex << vect[i];
    pdudata pdu = ss.str();
    std::cout << pdu << std::endl;

    return iLen;
}