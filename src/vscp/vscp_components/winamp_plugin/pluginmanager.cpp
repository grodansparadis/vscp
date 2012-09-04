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
#include "httpq.h"

#include "pluginmanager.h"


//--------------------------------------------------
// Externs / Globals
//--------------------------------------------------
extern int PluginInit();
extern void PluginQuit();
extern void PluginConfig();

WinampGeneralPurposePlugin gPluginInterface =
{
	GPPHDR_VER,
	"",
	PluginInit,
	PluginConfig,
	PluginQuit,
	NULL,
	NULL
};


//--------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------
PluginManager* PluginManager::mpPluginManager = NULL;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    if(mpPluginManager != NULL)
    {
        delete mpPluginManager;
        mpPluginManager = NULL;
    }

}

//--------------------------------------------------
// Functions
//--------------------------------------------------
PluginManager* PluginManager::Get()
{
    if (mpPluginManager == NULL)
        mpPluginManager = new PluginManager();
    return mpPluginManager;
}

void PluginManager::Initialize()
{
    static char c[128];
    wsprintf((gPluginInterface.description = c),"%s %s", APPNAME, VERSION);
}

LRESULT PluginManager::WinampMessage(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    return ::SendMessage(gPluginInterface.hwndParent, nMsg, wParam, lParam);
}

WinampGeneralPurposePlugin *PluginManager::GetPluginInterface()
{
    return &gPluginInterface;
}

