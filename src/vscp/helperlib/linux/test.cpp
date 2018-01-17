// test.c :
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2014-2018 Ake Hedman, Grodans Paradis AB <akhe@grodandparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.


#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>

#include <canal.h>
#include <canaldlldef.h>

static void* handle = NULL;


#define CanalOpen_Name                "CanalOpen"
#define CanalClose_Name               "CanalClose"
#define CanalGetLevel_Name            "CanalGetLevel"
#define CanalSend_Name                "CanalSend"
#define CanalReceive_Name             "CanalReceive"
#define CanalDataAvailable_Name       "CanalDataAvailable"
#define CanalGetStatus_Name           "CanalGetStatus"
#define CanalGetStatistics_Name       "CanalGetStatistics"
#define CanalSetFilter_Name           "CanalSetFilter"
#define CanalSetMask_Name             "CanalSetMask"
#define CanalSetBaudrate_Name         "CanalSetBaudrate"
#define CanalGetVersion_Name          "CanalGetVersion"
#define CanalGetDllVersion_Name       "CanalGetDllVersion"
#define CanalGetVendorString_Name     "CanalGetVendorString"
//Generation 2
#define CanalBlockingSend_Name        "CanalBlockingSend"
#define CanalBlockingReceive_Name     "CanalBlockingReceive"

static  LPFNDLL_CANALOPEN              CanalOpen_P ;
static	LPFNDLL_CANALCLOSE             CanalClose_P;
static	LPFNDLL_CANALGETLEVEL          CanalGetLevel_P;
static	LPFNDLL_CANALSEND              CanalSend_P;
static	LPFNDLL_CANALRECEIVE           CanalReceive_P;
static	LPFNDLL_CANALDATAAVAILABLE     CanalDataAvailable_P;
static	LPFNDLL_CANALGETSTATUS         CanalGetStatus_P;
static	LPFNDLL_CANALGETSTATISTICS     CanalGetStatistics_P;
static	LPFNDLL_CANALSETFILTER         CanalSetFilter_P;
static	LPFNDLL_CANALSETMASK           CanalSetMask_P;
static	LPFNDLL_CANALSETBAUDRATE       CanalSetBaudrate_P;
static	LPFNDLL_CANALGETVERSION        CanalGetVersion_P;
static	LPFNDLL_CANALGETDLLVERSION     CanalGetDllVersion_P;
static	LPFNDLL_CANALGETVENDORSTRING   CanalGetVendorString_P;
// Generation 2
static	LPFNDLL_CANALBLOCKINGSEND      CanalBlockingSend_P;
static	LPFNDLL_CANALBLOCKINGRECEIVE   CanalBlockingReceive_P;

void displayErrorAndExit(const char* funcName)
{
    // Free the library
    dlclose( handle );
    fprintf( stderr, "test: Unable to get dl entry for %s error=%s\n", funcName, dlerror() );
    exit ( EXIT_FAILURE );
}

void initLib(void)
{
    handle = dlopen("./libvscphelper.so", RTLD_LAZY);

    if ( 0 == handle )
    {
        fprintf( stderr, "test: Failed to load library %s\n", dlerror() );
        exit ( EXIT_FAILURE );
    }

    // * * * * CANAL OPEN * * * *
    if ( 0 == ( CanalOpen_P  = (LPFNDLL_CANALOPEN)dlsym( handle, CanalOpen_Name ) ) )
    {
        displayErrorAndExit(CanalOpen_Name);
    }
    // * * * * CANAL CLOSE * * * *
    if ( 0 == (CanalClose_P = (LPFNDLL_CANALCLOSE)dlsym( handle, CanalClose_Name ) ) )
    {
        displayErrorAndExit(CanalClose_Name);
    }

    // * * * * CANAL GETLEVEL * * * *
    if ( 0 == ( CanalGetLevel_P = (LPFNDLL_CANALGETLEVEL)dlsym( handle, CanalGetLevel_Name ) ) )
    {
        displayErrorAndExit(CanalGetLevel_Name);
    }

    // * * * * CANAL SEND * * * *
    if ( 0 == ( CanalSend_P = (LPFNDLL_CANALSEND)dlsym( handle, CanalSend_Name ) ) )
    {
        displayErrorAndExit(CanalSend_Name);
    }

    // * * * * CANAL RECEIVE * * * *
    if ( 0 == ( CanalReceive_P = (LPFNDLL_CANALRECEIVE)dlsym( handle, CanalReceive_Name ) ) )
    {
        displayErrorAndExit(CanalReceive_Name);
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    if ( 0 == ( CanalDataAvailable_P = (LPFNDLL_CANALDATAAVAILABLE)dlsym( handle,
										CanalDataAvailable_Name ) ) )
    {
        displayErrorAndExit(CanalDataAvailable_Name);
    }

    // * * * * CANAL GET STATUS * * * *
    if ( 0 == ( CanalGetStatus_P = (LPFNDLL_CANALGETSTATUS)dlsym( handle, CanalGetStatus_Name ) ) )
    {
        displayErrorAndExit(CanalGetStatus_Name);
    }

    // * * * * CANAL GET STATISTICS * * * *
    if ( 0 == ( CanalGetStatistics_P = (LPFNDLL_CANALGETSTATISTICS)dlsym( handle,
										CanalGetStatistics_Name ) ) )
    {
        displayErrorAndExit(CanalGetStatistics_Name);
    }

    // * * * * CANAL SET FILTER * * * *
    if ( 0 == ( CanalSetFilter_P = (LPFNDLL_CANALSETFILTER)dlsym( handle, CanalSetFilter_Name ) ) )
    {
        displayErrorAndExit(CanalSetFilter_Name);
    }

    // * * * * CANAL SET MASK * * * *
    if ( 0 == ( CanalSetMask_P = (LPFNDLL_CANALSETMASK)dlsym( handle, CanalSetMask_Name ) ) )
    {
        displayErrorAndExit(CanalSetMask_Name);
    }

    // * * * * CANAL SET BAUDRATE * * * *
    if ( 0 == ( CanalSetBaudrate_P = (LPFNDLL_CANALSETBAUDRATE)dlsym( handle,  CanalSetBaudrate_Name ) ) )
    {
        displayErrorAndExit( CanalSetBaudrate_Name);
    }

    // * * * * CANAL GET VERSION * * * *
    if ( 0 == ( CanalGetVersion_P = (LPFNDLL_CANALGETVERSION)dlsym( handle,  CanalGetVersion_Name) ) )
    {
        displayErrorAndExit( CanalGetVersion_Name);
    }

    // * * * * CANAL GET DLL VERSION * * * *
    if ( 0 == ( CanalGetDllVersion_P = (LPFNDLL_CANALGETDLLVERSION)dlsym( handle,
										CanalGetDllVersion_Name) ) )
    {
        displayErrorAndExit(  CanalGetDllVersion_Name);
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    if ( 0 == ( CanalGetVendorString_P = (LPFNDLL_CANALGETVENDORSTRING)dlsym( handle,
										CanalGetVendorString_Name) ) )
    {
        displayErrorAndExit( CanalGetVendorString_Name);
    }
}


long CanalOpen( const char *pDevice, unsigned long flags )
{
    return CanalOpen_P(pDevice, flags );
}

int CanalClose( long handle )
{
    return CanalClose_P(handle);
}

unsigned long CanalGetLevel( long handle )
{
    return CanalGetLevel_P(handle);
}

int CanalSend( long handle, PCANALMSG pCanalMsg )
{
    return CanalSend_P(handle, pCanalMsg);
}

int CanalReceive( long handle, PCANALMSG pCanalMsg )
{
    return CanalReceive_P(handle , pCanalMsg);
}

int CanalDataAvailable( long handle )
{
    return CanalDataAvailable_P (handle);
}

int CanalGetStatus( long handle, PCANALSTATUS pCanalStatus )
{
    return CanalGetStatus_P(handle, pCanalStatus);
}

int CanalGetStatistics( long handle, PCANALSTATISTICS pCanalStatistics  )
{
    return CanalGetStatistics_P(handle, pCanalStatistics);
}

int CanalSetFilter( long handle, unsigned long filter )
{
    return CanalSetFilter_P(handle, filter);
}

int CanalSetMask( long handle, unsigned long mask )
{
    return CanalSetMask_P(handle, mask);
}

int CanalSetBaudrate( long handle, unsigned long baudrate )
{
    return CanalSetBaudrate_P(handle, baudrate);
}

unsigned long CanalGetVersion( void )
{
    return CanalGetVersion_P();
}

unsigned long CanalGetDllVersion( void )
{
    return CanalGetDllVersion_P();
}

const char * CanalGetVendorString( void )
{
    return CanalGetVendorString_P();
}

int main()
{
    long myCanalHandle;
    canalMsg myCanalMsg;
    myCanalMsg.flags = 0;
    myCanalMsg.id = 0x10;
    myCanalMsg.sizeData = 2;
    myCanalMsg.data[0] = 4;
    myCanalMsg.data[1] = 7;
    myCanalMsg.timestamp = 0;
    initLib();
    myCanalHandle = CanalOpen_P ("logger",0);
    assert (myCanalHandle > 0);
    CanalSend_P (myCanalHandle, &myCanalMsg);
    return EXIT_SUCCESS;
}
