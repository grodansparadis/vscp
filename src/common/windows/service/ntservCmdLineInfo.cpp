/*  
Module : NTSERVCMDLINEINFO.CPP
Purpose: Implementation for the class CNTServiceCommandLineInfo 
Created: PJN / 14-07-1997
History: PJN / 11-01-2006 1. Command line which the service uses to start with can now be specified via a new /SCL command line option.
                          This value is parsed into CNTServiceCommandLineInfo::m_sServiceCommandLine. This value is used when the service
                          is being installed.
                          2. Optimized code in CNTServiceCommandLineInfo::CNTServiceCommandLineInfo.
                          3. Removed unnecessary CNTServiceCommandLineInfo destructor
                          4. Addition of a new /SD command line option which allows the description of the service to be used for 
                          registration to be customized at runtime. This value is parsed into CNTServiceCommandLineInfo::m_sServiceDescription.
                          5. Addition of a new /SN command line option which allows the service name to be used for registration to be 
                          customized at runtime. This value is parsed into CNTServiceCommandLineInfo::m_sServiceDescription.
                          5. Addition of a new /SDN command line option which allows the service display name to be used for registration 
                          to be customized at runtime. This value is parsed into CNTServiceCommandLineInfo::m_sServiceDisplayName.
                          6. Addition of 2 new command line options namely "/App" or "/Application". These behave the same way as the 
                          existing "/Debug" command line option.
                          7. Addition of a new /U command line option which allows the user account which the service runs under to be 
                          configured.
                          8. Addition of a new /P command line option which allows the password which the service is to run with to be 
                          configured.
         PJN / 13-01-2006 1. Addition of a new /AUTO command line option which allows you to specify that the service should be started
                          automatically.
                          2. Addition of a /console command line option which means the same as /debug

Copyright (c) 1996 - 2014 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////////////// Includes ////////////////////////////////////

#include <pch.h>
#include "ntservCmdLineInfo.h"


///////////////////////////////// Macros //////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


///////////////////////////////// Implementation //////////////////////////////

CNTServiceCommandLineInfo::CNTServiceCommandLineInfo() : m_nShellCommand(RunAsService),
                                                         m_dwTimeout(5000),
                                                         m_bSilent(FALSE),
                                                         m_bAutoStart(FALSE)
{
}

void CNTServiceCommandLineInfo::ParseParam(LPCTSTR pszParam, BOOL bFlag, BOOL bLast)
{
  if (bFlag)
  {
    CNTServiceString sParam(pszParam);
    CNTServiceString sParamUpper(sParam);
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    sParamUpper.MakeUpper();
    int nParamLength = sParam.GetLength();
  #else
    std::transform(sParamUpper.begin(), sParamUpper.end(), sParamUpper.begin(), _totupper);
    CNTServiceString::size_type nParamLength = sParam.length();
  #endif

    if (_tcsicmp(pszParam, _T("install")) == 0)
      m_nShellCommand = InstallService;
    else if (_tcsicmp(pszParam, _T("silent")) == 0)
      m_bSilent = TRUE;
    else if (_tcsicmp(pszParam, _T("start")) == 0)
      m_nShellCommand = StartTheService;
    else if (_tcsicmp(pszParam, _T("pause")) == 0)
      m_nShellCommand = PauseService;
    else if (_tcsicmp(pszParam, _T("continue")) == 0)
      m_nShellCommand = ContinueService;
    else if (_tcsicmp(pszParam, _T("stop")) == 0)
      m_nShellCommand = StopService;
    else if (_tcsicmp(pszParam, _T("auto")) == 0)
      m_bAutoStart = TRUE;
    else if ((_tcsicmp(pszParam, _T("remove")) == 0) ||
             (_tcsicmp(pszParam, _T("uninstall")) == 0) )
      m_nShellCommand = UninstallService;
    else if ((_tcsicmp(pszParam, _T("debug")) == 0) || (_tcsicmp(pszParam, _T("app")) == 0) || 
             (_tcsicmp(pszParam, _T("application")) == 0) || (_tcsicmp(pszParam, _T("console")) == 0))
      m_nShellCommand = DebugService;
    else if ((_tcsicmp(pszParam, _T("help")) == 0) ||
             (_tcsicmp(pszParam, _T("?")) == 0) )
      m_nShellCommand = ShowServiceHelp;
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("T:")) == 0 && (nParamLength > 2))
      m_dwTimeout = _ttoi(sParam.Mid(2, nParamLength - 2));
  #else
    else if (sParamUpper.find(_T("T:")) == 0 && (nParamLength > 2))
      m_dwTimeout = _ttoi(sParam.substr(2, nParamLength - 2).c_str());
  #endif
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("SCL:")) == 0 && (nParamLength > 4))
      m_sServiceCommandLine = sParam.Mid(4, nParamLength - 4);
  #else
    else if (sParamUpper.find(_T("SCL:")) == 0 && (nParamLength > 4))
      m_sServiceCommandLine = sParam.substr(4, nParamLength - 4);
  #endif
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("SD:")) == 0 && (nParamLength > 3))
      m_sServiceDescription = sParam.Mid(3, nParamLength - 3);
  #else
    else if (sParamUpper.find(_T("SD:")) == 0 && (nParamLength > 3))
      m_sServiceDescription = sParam.substr(3, nParamLength - 3);
  #endif
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("SN:")) == 0 && (nParamLength > 3))
      m_sServiceName = sParam.Mid(3, nParamLength - 3);
  #else
    else if (sParamUpper.find(_T("SN:")) == 0 && (nParamLength > 3))
      m_sServiceName = sParam.substr(3, nParamLength - 3);
  #endif
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("SDN:")) == 0 && (nParamLength > 4))
      m_sServiceDisplayName = sParam.Mid(4, nParamLength - 4);
  #else
    else if (sParamUpper.find(_T("SDN:")) == 0 && (nParamLength > 4))
      m_sServiceDisplayName = sParam.substr(4, nParamLength - 4);
  #endif
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("P:")) == 0 && (nParamLength > 2))
      m_sPassword = sParam.Mid(2, nParamLength - 2);
  #else
    else if (sParamUpper.find(_T("P:")) == 0 && (nParamLength > 2))
      m_sPassword = sParam.substr(2, nParamLength - 2);
  #endif
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    else if (sParamUpper.Find(_T("U:")) == 0 && (nParamLength > 2))
      m_sUserName = sParam.Mid(2, nParamLength - 2);
  #else
    else if (sParamUpper.find(_T("U:")) == 0 && (nParamLength > 2))
      m_sUserName = sParam.substr(2, nParamLength - 2);
  #endif
  }
  else
  {
    //Currently don't support parsing anything from
    //the command line except flags
  }

  if (bLast)
  {
    //Again the we don't support anything for the
    //last parameter
  }
}
