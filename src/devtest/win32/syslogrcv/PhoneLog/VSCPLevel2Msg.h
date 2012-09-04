// VSCPLevel2Msg.h: interface for the CVSCPLevel2Msg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VSCPLEVEL2MSG_H__81CD47B8_51C0_4E88_A02B_734CAFFBCA4E__INCLUDED_)
#define AFX_VSCPLEVEL2MSG_H__81CD47B8_51C0_4E88_A02B_734CAFFBCA4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VSCPII_PRIORITY_0		0x00
#define VSCPII_PRIORITY_1		0x20
#define VSCPII_PRIORITY_2		0x40
#define VSCPII_PRIORITY_3		0x60
#define VSCPII_PRIORITY_4		0x80
#define VSCPII_PRIORITY_5		0xA0
#define VSCPII_PRIORITY_6		0xC0
#define VSCPII_PRIORITY_7		0xE0

#define VSCPII_PRIORITY_LOW		0x00
#define VSCPII_PRIORITY_HIGH	0xE0

#define VSCPII_HARD_CODED		0x10


class CVSCPLevel2Msg  
{
public:
	CVSCPLevel2Msg();
	virtual ~CVSCPLevel2Msg();

protected:

	/*
		Message Priority
	*/
	unsigned char m_priority;

	/*
		Address is hardcoded
	*/
	bool m_bHardCoded;


	/*
		Message class
	*/
	unsigned short m_class;

	/*
		Message type
	*/
	unsigned short m_type;

	/*
		Message CRC
		Calculated on HEAD + CLASS + TYPE + ADDRESS + SIZE + DATA
	*/
	unsigned short m_crc;

	/*
		Data size
	*/
	unsigned short m_size;

	/*
		Data part
		max 492 bytes
	*/
	unsigned char *m_pData;

};

#endif // !defined(AFX_VSCPLEVEL2MSG_H__81CD47B8_51C0_4E88_A02B_734CAFFBCA4E__INCLUDED_)
