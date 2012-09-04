/*
CX10 - Class to send commands to an X10 firecracker
Contact : jg@jgsoftware.com
Use the code for anything, just submit suggestions and bug fixes to above address.

5/7/2004
  Initial release
*/

#pragma once

class CX10
{
private :
  typedef enum { ON, OFF, BRIGHTEN, DIM, ALL_L_ON, ALL_L_OFF, ALL_U_OFF } CommandType;
  
  bool   m_fConnected;
  HANDLE m_hPort;

  char m_chLastHouseCode;
  int  m_nLastUnitCode;

public :
  CX10(void);
  ~CX10(void);

  void Open(LPCSTR pszComPort);
  void Close();
 
  void TurnOn(char cHouseCode, int nUnitCode)
  {
    SendCommand(cHouseCode,nUnitCode,ON);
    m_chLastHouseCode = cHouseCode;
    m_nLastUnitCode = nUnitCode;
  }
  void TurnOff(char cHouseCode, int nUnitCode)
  {
    SendCommand(cHouseCode,nUnitCode,OFF);
    m_chLastHouseCode = cHouseCode;
    m_nLastUnitCode = nUnitCode;
  }
  void Dim()
  {
    SendCommand(m_chLastHouseCode,0,DIM);
  }
  void Brighten()
  {
    SendCommand(m_chLastHouseCode,0,BRIGHTEN);
  }
  void AllLightsOn(char cHouseCode)
  {
    SendCommand(cHouseCode,0,ALL_L_ON);
  }
  void AllLightsOff(char cHouseCode)
  {
    SendCommand(cHouseCode,0,ALL_L_OFF);
  }
  void AllUnitsOff(char cHouseCode)
  {
    SendCommand(cHouseCode,0,ALL_U_OFF);
  }

private :  
  BOOL SendCommand(char cHouseCode, int nUnitCode, CommandType nCommand);
  void SendByte(BYTE value);
  void SetComms();
  void SetDTR(bool fState);
  void SetRTS(bool fState);
};

