/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 * Copyright (C) 2005-2011 Ake Hedman, eurosource
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
#ifndef __CONFIGMANAGER_H
#define __CONFIGMANAGER_H

// include this here since most need it
#include "configinfo.h" 

class ConfigManager
{
public:
    static ConfigManager *Get();
    inline ConfigInfo *GetConfig() { return mpConfigInfo; }
    void SetConfig(ConfigInfo &config);

    void Serialize();
    void Deserialize();
    bool ValidatePassword(const char *password);
    const char *GetIniFile() const { return mIniFile; };

private:
    ConfigManager();
    ~ConfigManager();

    UINT ReadInt(char *key, int def);
    BOOL WriteInt(char *key, int value);
    DWORD ReadString(char *key, char *buffer, int size);
    BOOL WriteString(char *key, char *buffer);

    static ConfigManager *mpConfigManager;
    ConfigInfo *mpConfigInfo;
    char mIniFile[MAX_PATH];
};


#endif //__CONFIGMANAGER_H