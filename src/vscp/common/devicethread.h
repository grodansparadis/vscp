// devicethread.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#if !defined(DEVICETHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define DEVICETHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_



void *deviceThread( void *pData );

void *deviceLevel1ReceiveThread( void *pData );
void *deviceLevel1WriteThread( void *pData );

void *deviceLevel2ReceiveThread( void *pData );
void *deviceLevel2WriteThread( void *pData );

class CControlObject;
class CDeviceItem;
class deviceThreadObj;


/*!
    The common object for device threads
*/

class deviceThreadObj 
{

public:

    /// Constructor
    deviceThreadObj();

    /// Destructor
    virtual ~deviceThreadObj();

    // path to dll
    std::string m_path2Driver;

    /// dl/dll handler
    //void* m_handle_dll;

    // DeviceItem for device to control	
    CDeviceItem *m_pDeviceItem;

    // Control object that invoked thread
    CControlObject *m_pCtrlObject;

    // Quit flag
    bool m_bQuit;

    // Holder for CANAL receive thread
    pthread_t m_level1ReceiveThread;

    // Holder for CANAL write thread
    pthread_t m_level1WriteThread;
    
    // Holder for VSCP Level II receive thread
    pthread_t m_level2ReceiveThread;

    // Holder for VSCP Level II write thread
    pthread_t m_level2WriteThread;

    /*!
        Check filter/mask to see if filter should be delivered

        The filter have the following matrix

        mask bit n	|	filter bit n	| msg id bit	|	result
        ===========================================================
            0               X                   X           Accept
            1               0                   0           Accept
            1               0                   1           Reject
            1               1                   0           Reject
            1               1                   1           Accept

            Formula is !( ( filter �d ) & mask )

        @param pclientItem Pointer to client item
        @param pcanalMsg Pointer to can message
        @return True if message is accepted false if rejected
        TODO
    */
        
};




#endif



