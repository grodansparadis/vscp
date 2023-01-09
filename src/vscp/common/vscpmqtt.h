// rest.h:
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2023 Ake Hedman, the VSCP project
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

#if !defined(VSCPMQTT_H__INCLUDED_)
#define VSCPMQTT_H__INCLUDED_

#include <string>

class vscpMqttSubscribeItem 
{

public:

    /// Constructor
    vscpMqttSubscribeItem() {
        qos = 0;
        options = 0;
        mid = 0;
    }

    /// Destructor
    ~vscpMqttSubscribeItem();

    std::string topic;
    int qos;
    int options;
    int mid;        // Message id
};


class vscpMqttPublishItem 
{

public:

    /// Constructor
    vscpMqttPublishItem() {
        mid = 0;
    }

    /// Destructor
    ~vscpMqttPublishItem();

    std::string topic;
    int mid;        // Message id
};

#endif // VSCPMQTT_H__INCLUDED_