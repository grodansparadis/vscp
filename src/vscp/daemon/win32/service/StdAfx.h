#pragma once

#ifndef STRICT
#define STRICT
#endif

#ifndef WINVER
#define WINVER 0x0502
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0502
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define VC_EXTRALEAN //Exclude rarely-used stuff from Windows headers

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS //some CString constructors will be explicit

#define _AFX_ALL_WARNINGS //turns off MFC's hiding of some common and often safely ignored warning messages

#ifndef _SECURE_ATL
#define _SECURE_ATL 1 //Use the Secure C Runtime in ATL
#endif

//#define CNTSERVICE_MFC_EXTENSIONS //Uncomment this define if you would like to use the MFC integration provided by the CNTService framework

#ifdef CNTSERVICE_MFC_EXTENSIONS
#include <afxwin.h>     
#include <afxext.h> 
#include <afxmt.h>
#else
#include <string>
#include <vector>
#include <algorithm>
#endif

#include <atlbase.h>
#include <winsvc.h>

