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
#ifndef __PLUGINMANAGER_H
#define __PLUGINMANAGER_H

typedef struct 
{
	int version;
	char *description;
	int (*PluginInit)();
	void (*PluginConfig)();
	void (*PluginQuit)();
	HWND hwndParent;
	HINSTANCE hDllInstance;
} WinampGeneralPurposePlugin;



class PluginManager
{
public:
    static PluginManager* Get();
    void Initialize();
    WinampGeneralPurposePlugin *GetPluginInterface();
    LRESULT WinampMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    PluginManager();
    ~PluginManager();
    
    static PluginManager* mpPluginManager;
    WinampGeneralPurposePlugin mPluginInterface;
};


#endif //__PLUGINMANAGER_H