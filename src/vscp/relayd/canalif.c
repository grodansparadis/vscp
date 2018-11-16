// canalif.c
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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
//


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>

#include "canalif.h"

/* 
 * canalif_init 
 */
void canalif_init( canalif_t *pdrvobj )
{
    /* No methods found in the driver yet */
    pdrvobj->m_proc_CanalOpen = NULL;
    pdrvobj->m_proc_CanalClose = NULL;
    pdrvobj->m_proc_CanalGetLevel = NULL;
    pdrvobj->m_proc_CanalSend = NULL;
    pdrvobj->m_proc_CanalDataAvailable = NULL;
    pdrvobj->m_proc_CanalReceive = NULL;
    pdrvobj->m_proc_CanalGetStatus = NULL;
    pdrvobj->m_proc_CanalGetStatistics = NULL;
    pdrvobj->m_proc_CanalSetFilter = NULL;
    pdrvobj->m_proc_CanalSetMask = NULL;
    pdrvobj->m_proc_CanalSetBaudrate = NULL;
    pdrvobj->m_proc_CanalGetVersion = NULL;
    pdrvobj->m_proc_CanalGetDllVersion = NULL;
    pdrvobj->m_proc_CanalGetVendorString = NULL;

    /* Generation 2 */
    pdrvobj->m_proc_CanalBlockingSend = NULL;
    pdrvobj->m_proc_CanalBlockingReceive = NULL;
    pdrvobj->m_proc_CanalGetdriverInfo = NULL;

    pdrvobj->m_hdll = NULL;
    pdrvobj->m_drvPath = NULL;
    pdrvobj->m_drvConfig = NULL;
    pdrvobj->m_drvFlags = 0;
}

/* 
 * canalif_cleanup
 */
void canalif_cleanup( canalif_t *pdrvobj )
{

}

/* 
 * canalif_setPath 
 */
char * canalif_setPath( canalif_t *pdrvobj, const char *path ) 
{
    /* Check pointer */
    if ( NULL == pdrvobj ) return NULL;
    if ( NULL == path ) return NULL;

    /* Delete old setting if set */
    if ( NULL != pdrvobj->m_drvPath ) {
        free( pdrvobj->m_drvPath );
    }

    pdrvobj->m_drvPath = (char *)malloc( strlen( path ) + 1 );
    strcpy( pdrvobj->m_drvPath, path );
    return pdrvobj->m_drvPath;
 }

 /* 
 * canalif_setDrvParams 
 */
char * canalif_setConfig( canalif_t *pdrvobj, 
                            const char *pConfig ) 
{
    /* Check driver object pointer */
    if ( NULL == pdrvobj ) return NULL;

    // Check so there is a configuration string to handle
    if ( NULL == pConfig ) return NULL;

    /* Delete old setting if set */
    if ( NULL != pdrvobj->m_drvConfig ) {
        free( pdrvobj->m_drvConfig );
    }

    pdrvobj->m_drvConfig = (char *)malloc( strlen( pConfig ) + 1 );
    strcpy( pdrvobj->m_drvConfig, pConfig );
    return pdrvobj->m_drvConfig;
 }

