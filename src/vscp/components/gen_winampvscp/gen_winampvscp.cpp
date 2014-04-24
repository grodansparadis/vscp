// gen_winampvscp.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "pluginmanager.h"
#include "configmanager.h"
#include "configdialog.h"
#include "gen_winampvscp.h"

//--------------------------------------------------
// Winamp General Purpose Function Callbacks 
//--------------------------------------------------
void PluginConfig()
{
    ConfigDialog::Get()->ShowDialog();
}

void PluginQuit()
{
    //Server::Get()->Stop();
}


int PluginInit()
{
    PluginManager::Get()->Initialize();
    ConfigManager::Get()->Deserialize();

    // start server
    //if ( ConfigManager::Get()->GetConfig()->IsAutoStartEnabled() )
    //    Server::Get()->Start();


#ifdef _DEBUG
	PluginConfig();
#endif

    return 0; // OK
}


//--------------------------------------------------
// DLL Entry point
//--------------------------------------------------
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}


extern "C"
__declspec( dllexport )WinampGeneralPurposePlugin* winampGetGeneralPurposePlugin()
{
    return PluginManager::Get()->GetPluginInterface();
}


/*
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
*/
