// Menu definition
// Copyright (c) 2015 Andreas Merkle
// <vscp@blue-andi.de>
//
// Licence:
// The MIT License (MIT)
// [OSI Approved License]
//
// The MIT License (MIT)
//
// Copyright (c) 2012-2016 Grodans Paradis AB (Paradise of the Frog)
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
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Alternative licenses for VSCP & Friends may be arranged by contacting
// Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

/*jshint bitwise: false */

/** Create the root namespace and making sure we're not overwriting it */
var navBarMenu = navBarMenu || {};

/* ---------------------------------------------------------------------- */

navBarMenu.content = [
    {
        title: "Home",
        url: "../index.html"
    },
    {
        title: "General VSCP functionality",
        url: "#",
        dropDown: [
            {
                title: "Set Server",
                url: "javascript:do_setServer();"
            },
            {
                title: "Open REST session",
                url: "javascript:do_open();"
            },
            {
                title: "Close REST session",
                url: "javascript:do_close();"
            },
            {
                title: "Get status",
                url: "javascript:do_status();"
            },
            {
                title: "Read one event",
                url: "javascript:do_readEventOne();"
            },
            {
                title: "Read all events",
                url: "javascript:do_readEventAll();"
            },
            {
                title: "Send event",
                url: "javascript:do_sendEvent();"
            },
            {
                title: "Set filter",
                url: "javascript:do_setFilter();"
            },
            {
                title: "Clear filter",
                url: "javascript:do_clrFilter();"
            },
            {
                title: "Clear event queue",
                url: "javascript:do_clrQueue();"
            },
            {
                title: "Send measurement",
                url: "javascript:do_sendMeasurement();"
            },
            {
                title: "Get table",
                url: "javascript:alert('Not available yet');"
            },
        ]
    },
    {
        title: "Variables",
        url: "#",
        dropDown: [
            {
                title: "Create variable",
                url: "javascript:do_createVariable();"
            },
            {
                title: "Read variable",
                url: "javascript:do_readVariable();"
            },
            {
                title: "Write variable",
                url: "javascript:do_writeVariable();"
            },
            {
                title: "Delete variable",
                url: "javascript:do_deleteVariable();"
            },
        ]
    },
];
