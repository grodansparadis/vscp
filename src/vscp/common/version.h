// version.h
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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
#ifndef _____VSCP_VERSION_h_____
#define _____VSCP_VERSION_h_____
/*
    MAJOR version with incompatible API changes,
    MINOR version with add functionality in a backwards-compatible manner, and
    RELEASE version with backwards-compatible bug fixes.
    BUILD Just a new build.
*/
// I M P O T A N T ! ! ! Lines below must be located at line
// 35/36/37/38/40/42/43 I M P O T A N T ! ! !
#define VSCPD_MAJOR_VERSION     14
#define VSCPD_MINOR_VERSION     0
#define VSCPD_RELEASE_VERSION   0
#define VSCPD_BUILD_VERSION     0

#define VSCPD_DISPLAY_VERSION   "14.0.0.0 Aluminium"

#define VSCPD_COPYRIGHT                                                        \
    "Copyright (C) 2000-2019 Ake Hedman,, Grodans Paradis AB, "                \
    "https://www.grodansparadis.com"
#define VSCPD_COPYRIGHT_HTML                                                   \
    "Copyright (C) 2000-2019 Ake Hedman,, <a "                                 \
    "href=\"mailto:info@grodansparadis.com\">Grodans Paradis AB</a>, <a "      \
    "hrefs=\"https://www.grodansparadis.com\">https://"                        \
    "www.grodansparadis.com</a>"

#define VSCP_VERSION(major, minor, release)                                    \
    (((major) << 16) | ((minor) << 8) | (release))
#endif
