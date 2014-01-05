/*
CX10 - Class to send commands to an X10 firecracker
Contact : jg@jgsoftware.com
Use the code for anything, just submit suggestions and bug fixes to above address.

5/7/2004
  Initial release
*/

#include "StdAfx.h"
#include "x10.h"

static BYTE houseCodes[16] = 
{ 
  0x60, // A - 01100000
  0x70, // B - 01110000
  0x40, // C - 01000000
  0x50, // D - 01010000
  0x80, // E - 10000000
  0x90, // F - 10010000
  0xA0, // G - 10100000
  0xB0, // H - 10110000
  0xE0, // I - 11100000
  0xF0, // J - 11110000
  0xC0, // K - 11000000
  0xD0, // L - 11010000
  0x00, // M - 00000000
  0x10, // N - 00010000
  0x20, // O - 00100000
  0x30  // P - 00110000
};

static BYTE unitCodes[8] =
{
  0x00, // 1,9  00000000
  0x10, // 2,10 00010000
  0x08, // 3,11 00001000
  0x18, // 4,12 00011000
  0x40, // 5,13 01000000
  0x50, // 6,14 01010000
  0x48, // 7,15 01001000
  0x58  // 8,16 01011000
};

//****************************************************************************
// CX10::CX10 - Constructor
//****************************************************************************

CX10::CX10(void)
{
  m_hPort = INVALID_HANDLE_VALUE;
  m_fConnected = false;
  m_chLastHouseCode = 'A';
  m_nLastUnitCode = 1;
}

//****************************************************************************
// CX10::~CX10 - Destructor
//****************************************************************************

CX10::~CX10(void)
{
  Close();
}

//****************************************************************************
// CX10::Open - Opens the COM port specified by the pszComPort parameter and
//              sets the port parameters
//****************************************************************************

void CX10::Open(LPCSTR pszComPort)
{
  Close();
  m_hPort = CreateFile(pszComPort,GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
  if (INVALID_HANDLE_VALUE == m_hPort) throw GetLastError;
  
  m_fConnected = true;
  try
  {
    SetComms();
  }
  catch (int dwError)
  {
    Close();
    throw dwError;
  }
}

//****************************************************************************
// CX10::Close - Closes COM port if open
//****************************************************************************

void CX10::Close()
{
  if (m_hPort != INVALID_HANDLE_VALUE) CloseHandle(m_hPort);
  m_hPort = INVALID_HANDLE_VALUE;
  m_fConnected = false;
}

//****************************************************************************
// CX10::SendCommand - Sends a command frame to X10 unit
//****************************************************************************

BOOL CX10::SendCommand(char cHouseCode, int nUnitCode, CommandType nCommand)
{
  BYTE hc;

  // Force the house code to lowercase and then convert it to use as index into the houseCodes array
  hc = houseCodes[(BYTE(cHouseCode) | 0x20) - 'a'];

  nUnitCode--; // 
  if ((nUnitCode > 7) && ((OFF == nCommand) || (ON == nCommand)))
  {
    hc|= 0x4;
    nUnitCode-= 8;
  }

  SetDTR(false);  // Put FireCracker into ready mode
  SetRTS(false);
  Sleep(30);
  SetDTR(true);  // Put FireCracker into ready mode
  SetRTS(true);
  Sleep(30);

  SendByte(0xD5); // Send first byte of header 11010101
  SendByte(0xAA); // and then the second byte  10101010

  SendByte(hc);  // Send the house code
  switch (nCommand)
  {
    case OFF : 
      SendByte(unitCodes[nUnitCode] | 0x20); // Send off code for the unit
      break;
    case ON  : 
      SendByte(unitCodes[nUnitCode]);  // Send on code for the unit
      break;
    case DIM : 
      SendByte(0x98);                // Send dim code 10011000
      break;
    case BRIGHTEN :
      SendByte(0x88);                // Send brighten code 10001000
      break;
    case ALL_L_ON :
      SendByte(0x90); // All lights on 10010000
      break;
    case ALL_L_OFF :
      SendByte(0xA0); // All lights off 10100000
      break;
    case ALL_U_OFF :
      SendByte(0x80); // All units off 10000000
      break;
  }
  SendByte(0xAD); // Footer 10101101
  Sleep(30);

  return false;
}

//****************************************************************************
// CX10::SendByte - Sends a byte of data to the X10 unit by toggling the
//                  CTS and RTS lines
//****************************************************************************

void CX10::SendByte(BYTE value)
{
  for (BYTE bBit = 0x80;bBit > 0;bBit >>= 1)
  {
    if (value & bBit)
      SetDTR(false);
    else
      SetRTS(false);
      
    Sleep(1);
    SetDTR(true);
    SetRTS(true);
    Sleep(1);
  }
}

//****************************************************************************
// CX10::SetComms - Initializes the COM port
//****************************************************************************

void CX10::SetComms()
{
  DCB dcb;

  if (m_fConnected)
  {
    ZeroMemory(&dcb,sizeof(DCB));    
    dcb.DCBlength = sizeof(DCB);
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.BaudRate = CBR_9600; // Arbitrary values. Not used
    dcb.ByteSize = 8; // Arbitrary values. Not used

    if (!SetCommState(m_hPort,&dcb))
      throw GetLastError();
  }
}

//****************************************************************************
// CX10::SetDTR - Causes the DTR line to either go high or low
//****************************************************************************

void CX10::SetDTR(bool fState)
{
  if(!EscapeCommFunction(m_hPort,fState ? SETDTR : CLRDTR))
    throw GetLastError();
}

//****************************************************************************
// CX10::SetRTS - Causes the RTS line to either go high or low
//****************************************************************************

void CX10::SetRTS(bool fState)
{
  if (!EscapeCommFunction(m_hPort,fState ? SETRTS : CLRRTS))
    throw GetLastError();
}
