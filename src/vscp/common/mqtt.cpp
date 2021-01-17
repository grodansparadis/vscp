// FILE: mqtt.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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

#include "mqtt.h"

#include <vscp_client_mqtt.h>
#include <vscphelper.h>

#include <unistd.h>
#include <pthread.h>

#include <string>
#include <deque>

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpMqttObj::vscpMqttObj()
{
    m_pClientItem = NULL;
    m_pCtrlObj = NULL;

    m_bQuit = false;
}

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpMqttObj::~vscpMqttObj()
{
    m_bQuit = true;     // Signal that it's time to quit
    pthread_join(m_mqttClientWorkerThread, NULL);
}



///////////////////////////////////////////////////////////////////////////////
// startWorkerThread
//

bool vscpMqttObj::startWorkerThread(CControlObject *pObj)
{
    if (NULL == pObj) {
        syslog(LOG_ERR,
                "MQTT: Need controlobject pointer to start MQTT client worker thread.");
        return false;
    }

    m_pCtrlObj = pObj;

    if (pthread_create(&m_mqttClientWorkerThread,
                        NULL,
                        mqttClientWorkerThread,
                        this)) {
        syslog(LOG_ERR,
                "MQTT: Unable to create the MQTT client worker thread.");
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// MQTT Workerthread
//

void*
mqttClientWorkerThread(void* pData)
{
    return NULL;
}

