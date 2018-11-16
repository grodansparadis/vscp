// canalif.h
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


#ifndef ____DEFINED_CANALIF____
#define ____DEFINED_CANALIF____


#include <canaldlldef.h>    /* Level I driver functions             */


/******************************************************************************
 *                              CANAL if structure
 * ****************************************************************************/

typedef struct canalif {
    /* Level I (CANAL) driver methods */
    LPFNDLL_CANALOPEN                   m_proc_CanalOpen;
    LPFNDLL_CANALCLOSE                  m_proc_CanalClose;
    LPFNDLL_CANALGETLEVEL               m_proc_CanalGetLevel;
    LPFNDLL_CANALSEND                   m_proc_CanalSend;
    LPFNDLL_CANALRECEIVE                m_proc_CanalReceive;
    LPFNDLL_CANALDATAAVAILABLE          m_proc_CanalDataAvailable;
    LPFNDLL_CANALGETSTATUS              m_proc_CanalGetStatus;
    LPFNDLL_CANALGETSTATISTICS          m_proc_CanalGetStatistics;
    LPFNDLL_CANALSETFILTER              m_proc_CanalSetFilter;
    LPFNDLL_CANALSETMASK                m_proc_CanalSetMask;
    LPFNDLL_CANALSETBAUDRATE            m_proc_CanalSetBaudrate;
    LPFNDLL_CANALGETVERSION             m_proc_CanalGetVersion;
    LPFNDLL_CANALGETDLLVERSION          m_proc_CanalGetDllVersion;
    LPFNDLL_CANALGETVENDORSTRING        m_proc_CanalGetVendorString;
    /* Generation 2 */
    LPFNDLL_CANALBLOCKINGSEND           m_proc_CanalBlockingSend;
    LPFNDLL_CANALBLOCKINGRECEIVE        m_proc_CanalBlockingReceive;
    LPFNDLL_CANALGETDRIVERINFO          m_proc_CanalGetdriverInfo;

    void *m_hdll;               /* Handle to dll                */
    long m_hCANAL;              /* Handel to CANAL driver       */
    char *m_drvPath;            /* Path to CANAL driver         */
    char *m_drvConfig;          
    
    /* Driver configuration string  */
    unsigned long m_drvFlags;   /* Driver flags                 */

} canalif_t;

/******************************************************************************
 *                            canalif_init
 * 
 * @param pdrvobj Driver object
 * 
 * ****************************************************************************/
void canalif_init( canalif_t *pdrvobj );

/******************************************************************************
 *                            canalif_cleanup
 * 
 * @param pdrvobj Driver object
 * 
 * ****************************************************************************/
void canalif_cleanup( canalif_t *pdrvobj );

/*
    canalif_setPath

    @param pdrvobj Driver object
    @param path Path to driver
    @return NULL for failure or pointer to the driver path if successful.
*/
char * canalif_setPath( canalif_t *pdrvobj, const char *path );

/*
    canalif_setDrvParams

    @param pdrvobj Driver object
    @param pConfig Pointer to configurations string
    @param flags Flags for driver.
    @return NULL for failure or pointer to the driver path if successful.
*/
char * canalif_setConfig( canalif_t *pdrvobj, const char *pConfig );


#endif