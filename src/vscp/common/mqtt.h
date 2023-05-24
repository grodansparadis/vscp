// FILE: mdf.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C)2000-2023 Ake Hedman, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef SRC_VSCP_COMMON_MQTT_H
#define SRC_VSCP_COMMON_MQTT_H

#include <guid.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscp_client_mqtt.h>

#include <stdlib.h>
#include <string>

#ifndef WIN32
#include <arpa/inet.h>
#include <syslog.h>
#include <unistd.h>
#endif

// Prototypes
void*
mqttClientWorkerThread(void* pData);

class CControlObject;
class CClientItem;
class vscpClientMqtt;

/*!
    The client object is created for each client
    
*/

class vscpMqttObj {

 public:
    vscpMqttObj();
    ~vscpMqttObj();

    /*!
        Get connection interface
    */
    std::string getInterface(void)
    {
        std::string str = m_client.getHost();
        str += ":";
        str += vscp_str_format("%d",m_client.getPort());
        return str;
    };

    /*!
        Start the client worker thread

        @param pObj Pointer to control object
        @return True on success, false on failure.
    */
    bool startWorkerThread(CControlObject *pObj);

 public:

    // Client thread is running as long as this
    // variable is false
    bool m_bQuit;

    // Client we are working as
    CClientItem *m_pClientItem;

    // The global control object
    CControlObject *m_pCtrlObj;

    // The MQTT client
    vscpClientMqtt m_client;

    // The thread that do the actual work
    pthread_t m_mqttClientWorkerThread;

};



#endif  // SRC_VSCP_COMMON_MQTT_H_"  [build/header_guard]
