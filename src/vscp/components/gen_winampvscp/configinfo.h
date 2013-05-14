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
#ifndef __CONFIGINFO_H
#define __CONFIGINFO_H

class ConfigInfo
{
public:
    friend class ConfigManager;

    ConfigInfo();
	ConfigInfo& operator=(const ConfigInfo &info);
    ~ConfigInfo();

    inline DWORD Version() const { return dwVersion; };
    inline DWORD GetServerAddress() const { return dwServerAddress; };
    inline int GetPort() const { return nPort; }
    inline const char *GetPassword() const { return &strPassword[0]; }

    inline void SetServerAddress(DWORD ipaddr) { dwServerAddress = ipaddr; };
    inline void SetPort(int port) { nPort = port; }
    inline void SetPassword(const char *pass) { strcpy(strPassword, pass); }

private:
    DWORD   dwVersion;
    DWORD	dwServerAddress;
	int		nPort;
	short	nZone;
	short	nSubZone;
	char	strPassword[ MAX_PATH ];
};


#endif //__CONFIGINFO_H