/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Kosta Arvanitis (karvanitis@hotmail.com)
 */
#ifndef __UTIL_H
#define __UTIL_H

//-------------------------------------------------
// General Utils
//-------------------------------------------------

//extern char *strrstr(const char *, const char *);
extern  int lastindexof(const char *, const char *);
extern char *strmemdup(const char *s, size_t n);
extern void strmemfree(const char *s);
extern char *strtolower(char *s);
extern int axtoi(const char *x);
extern void urldecode(char *str);
extern bool openurl(const char *url);
extern bool filexists(char *filepath);

//-------------------------------------------------
// Id3Lib Utils
//-------------------------------------------------

#define MPEG_VERSION2DESCRIPTION(v) \
    ((v == MPEGVERSION_2_5) ? "MPEG 2.5" : \
    (v == MPEGVERSION_2) ? "MPEG 2.0" : \
    (v == MPEGVERSION_1) ? "MPEG 1.0" : "")


#define MPEG_LAYER2DESCRIPTION(l) \
    ((l == MPEGLAYER_III) ? "layer 3" : \
    (l == MPEGLAYER_II) ? "layer 2" : \
    (l == MPEGLAYER_I) ? "layer 1" : "")


#define MPEG_CHANNELMODE2DESCRIPTION(m) \
    ((m == MP3CHANNELMODE_STEREO) ? "Stereo" : \
    (m == MP3CHANNELMODE_JOINT_STEREO) ? "Joint Stereo" : \
    (m == MP3CHANNELMODE_DUAL_CHANNEL) ? "Dual Channel" : \
    (m == MP3CHANNELMODE_SINGLE_CHANNEL) ? "Single Channel" : "")


#define MPEG_CRC2DESCRIPTION(c) \
    ((c == MP3CRC_MISMATCH) ? "Mismatch" : \
    (c == MP3CRC_NONE) ? "No" : \
    (c == MP3CRC_OK) ? "OK" : "")

#define MPEG_COPYRIGHT2DESCRIPTION(c) \
    ((c) ? "Yes" : "No")

#define MPEG_ORIGINAL2DESCRIPTION(o) \
    ((o) ? "Yes" : "No")

#define MPEG_EMPHASIS2DESCRIPTION(e) \
    ((e == MP3EMPHASIS_NONE) ? "None" : \
    (e == MP3EMPHASIS_50_15MS) ? "50/15 ms" : \
    (e == MP3EMPHASIS_Reserved) ? "Reserved" : \
    (e == MP3EMPHASIS_CCIT_J17) ? "CITT J.17" : "")

#define MPEG_BITRATE2KBPS(b)    b/1000
#define MPEG_FREQUENCY2KHZ(f)   f/1000



#endif // __UTIL_H

