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

#include "commands.h"
#include "query.h"
#include "socketwriter.h"
#include "configmanager.h"
#include "pluginmanager.h"
#include "util.h"

#include <id3.h>
#include <id3/tag.h>
#include <id3/misc_support.h>


//--------------------------------------------------
// Function documentation is created by a perl script 
// which parses the specific comments in this file.
//--------------------------------------------------

// root function is not documented
CommandFunctionDecl(Root)
{
    pWriter->WriteData(APPNAME);
    pWriter->WriteData(" ");
    pWriter->WriteData(VERSION);
}


/*!
<function>
    <name>validate_password</name>
    <action>
        Check a given string against the real password.
    </action>
    <arguments>
        <argument>
            <name>pass</name>
            <description>a string representing the password to validate.</description>
        </argument>
    </arguments>
    <return>
        1 if passwords match. <br/>
        0 if passwords do not match.
    </return>
    <example>
        http://localhost:4800/validate_password?pass=password
    </example>
</function>
!*/
CommandFunctionDecl(Validate_PasswordA)
{
    const char *arg = query.GetArgument("pass");

    if(ConfigManager::Get()->ValidatePassword(arg))
        pWriter->WriteSuccess();
    else
        pWriter->WriteError();
}


/*!
<function>
    <name>play</name>
    <action>
        Like clicking on it.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/play?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Play)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_BUTTON2, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>stop</name>
    <action>
        Like clicking on it.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/stop?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Stop)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_BUTTON4, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>pause</name>
    <action>
        Like clicking on it.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/pause?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Pause)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_BUTTON3, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>next</name>
    <action>
        Like clicking on it.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/next?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(NextTrack)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_BUTTON5, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>prev</name>
    <action>
        Like clicking on it.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/prev?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(PreviousTrack)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_BUTTON1, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>fadeoutandstop</name>
    <action>
        Fades out current song and stops playing.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/fadeoutandstop?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(FadeoutAndStop)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_BUTTON4_SHIFT, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>getversion</name>
    <action>
        Get the current version of the winamp player.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        Version of winamp, or 0 on error.
    </return>
    <example>
        http://localhost:4800/getversion?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(GetVersion)
{
    char buf[128];
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETVERSION);
    sprintf(buf, "0x%x", res);
    pWriter->WriteData(buf);
}


/*!
<function>
    <name>delete</name>
    <action>
        Clears the contents of the play list.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, 0 otherwise.
    </return>
    <example>
        http://localhost:4800/delete?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Delete)
{
    PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_DELETE);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>isplaying</name>
    <action>
        Get the playing status of winamp player.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 = playing <br/>
        0 = not playing <br/>
        3 = paused
    </return>
    <example>
        http://localhost:4800/isplaying?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(IsPlaying)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_ISPLAYING);
    pWriter->WriteReturnCode(res);
}


/*!
<function>
    <name>getlistlength</name>
    <action>
        Get the length of the current playlist, in tracks.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
       The length of the current playlist or 0 on error.
    </return>
    <example>
        http://localhost:4800/getlistlength?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(GetListLength)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTLENGTH);
    pWriter->WriteReturnCode(res);
}


/*!
<function>
    <name>getlistpos</name>
    <action>
        Gets the current index of the play list. <br/>
        note: The list is zero indexed so track 1 is index 0.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
       List position or 0 on error.
    </return>
    <example>
        http://localhost:4800/getlistpos?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(GetListPos)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTPOS);
    pWriter->WriteReturnCode(res);
}


// not documented. maintained for backwards comptability only.
// use GetListPos instead.
CommandFunctionDecl(GetPlayListPos)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTPOS);
    pWriter->WriteReturnCode(res);
}


/*!
<function>
    <name>shuffle_status</name>
    <action>
        Gets the status of shuffle button.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
       1 on success. <br/>
       0 otherwise.
    </return>
    <example>
        http://localhost:4800/shuffle_status?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Shuffle_Status)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GET_SHUFFLE);
    pWriter->WriteReturnCode(res);
}


/*!
<function>
    <name>repeat_status</name>
    <action>
        Gets the status of the repeat button.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
       1 if repeat is on. <br/>
       0 if repeat is off.
    </return>
    <example>
        http://localhost:4800/repeat_status?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Repeat_Status)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GET_REPEAT);
    pWriter->WriteReturnCode(res);
}


/*!
<function>
    <name>volumeup</name>
    <action>
        Turns up the volume.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/volumeup?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(VolumeUp)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_VOLUMEUP, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>volumedown</name>
    <action>
        Turns down the volume.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/volumedown?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(VolumeDown)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_VOLUMEDOWN, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>flushplaylist</name>
    <action>
        Flushes the playlist cache buffer.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/flushplaylist?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(FlushPlayList)
{
    PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_REFRESHPLCACHE);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>getcurrenttitle</name>
    <action>
        Returns the title of the current song from the winamp window name. 
        Used when the playlist title is not enough.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        Song title on success. <br />
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getcurrenttitle?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(GetCurrentTitle)
{
	// get current title from the window name if the play list pos is not enough
    char title[2048], *p;

    GetWindowText(PluginManager::Get()->GetPluginInterface()->hwndParent, title, sizeof(title));
	p = title + strlen(title) - 8;

	while(p >= title)
	{
		if(!strnicmp(p, "- Winamp", 8)) 
			break;
		--p;
	}
    if(p >= title) 
		--p;
	while(p >= title && *p == ' ') 
		--p;
	*++p = 0;
		
    pWriter->WriteData(title);

}


/*!
<function>
    <name>updatecurrenttitle</name>
    <action>
        Updates the information about the current title.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success. <br />
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/updatecurrenttitle?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(UpdateCurrentTitle)
{
    PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_UPDTITLE);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>internet</name>
    <action>
        Checks for an internet conection.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 if internect conecton exists. <br />
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/internet?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Internet)
{
    int res = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_INETAVAILABLE);
    pWriter->WriteReturnCode(res);
}


/*!
<function>
    <name>restart</name>
    <action>
        Restarts winamp and httpQ server. <br/>
        note: If the httpQ service is not set to start automatically, the httpQ server will not continue to accept connections.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success. <br />
        0 on error.
    </return>
    <example>
        http://localhost:4800/restart?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Restart)
{
    PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_RESTARTWINAMP);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>getautoservice</name>
    <action>
        Returns the status the httpQ autostart configuration.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 if the service is set to start automatically. <br />
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getautoservice?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(GetAutoService)
{
    pWriter->WriteData(ConfigManager::Get()->GetConfig()->IsAutoStartEnabled()?"1":"0");
}


/*!
<function>
    <name>setautoservice</name>
    <action>
        Returns the status the httpQ autostart configuration.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>enable</name>
            <description>
                1 Set the service to start automatically.  <br />
                0 Set the service to not start automatically. 
            </description>
        </argument>
    </arguments>
    <return>
        1 if the service is set to start automatically. <br />
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/setautoservice?p=pass&enable=1
    </example>
</function>
!*/
CommandFunctionDecl(SetAutoServiceA)
{
    const char *arg = query.GetArgument("enable");
    int res = atoi(arg);

    if(res != 0 && res != 1)
	{
        pWriter->WriteError();
	}
    else
    {
        ConfigManager::Get()->GetConfig()->SetAutoStart((res == 1)?true:false);
        ConfigManager::Get()->Serialize();
        pWriter->WriteData(ConfigManager::Get()->GetConfig()->IsAutoStartEnabled()?"1":"0");
    }
}


/*!
<function>
    <name>shoutcast_connect</name>
    <action>
        Attempts to start the shoutcast server.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success. <br />
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/shoutcast_connect?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(ShoutCast_Connect)
{
    HWND shoutcast = FindWindow(NULL, "SHOUTcast source");

    if(!shoutcast)
    {
        pWriter->WriteError();
    }
    else
    {
        char tmp[256];
        SendMessage(shoutcast, WM_COMMAND, SHOUTCAST_CONNECT, 0);
        SendDlgItemMessage(shoutcast, SHOUTCAST_CONNECT, WM_GETTEXT, sizeof(tmp), (LPARAM)(LPCTSTR)&tmp);
        pWriter->WriteData(tmp);
    }
}


/*!
<function>
    <name>shoutcast_status</name>
    <action>
        Attempts to retrieve status from shoutcast server.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>a</name>
            <description>
                status of shoutcast server if successful  <br/>
                0 otherwise. 
            </description>
        </argument>
    </arguments>
    <return>
    </return>
    <example>
        http://localhost:4800/shoutcast_status?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(ShoutCast_Status)
{
    HWND shoutcast = FindWindow(NULL, "SHOUTcast source");

    if(!shoutcast)
    {
        pWriter->WriteError();
    }
    else
    {
        char tmp[256];
        SendDlgItemMessage(shoutcast, SHOUTCAST_STATUS, WM_GETTEXT, sizeof(tmp), (LPARAM)(LPCTSTR)&tmp);
        pWriter->WriteData(tmp);
    }
}


/*!
<function>
    <name>deletepos</name>
    <action>
        Deletes the playlist item at index 'argument'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>index</name>
            <description>
                Index in playlist to remove.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success, <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/deletepos?p=pass&index=0
    </example>
</function>
!*/
CommandFunctionDecl(DeletePosA)
{
    const char *arg = query.GetArgument("index");

    HWND hwndPE = FindWindow("Winamp PE", NULL);
    int res = atoi(arg);
    if(!hwndPE || res < 0)
    {
        pWriter->WriteError();
    }
    else
    {
        SendMessage(hwndPE, WM_USER, 104, res);
        pWriter->WriteSuccess();
    }
}


/*!
<function>
    <name>exec_visual</name>
    <action>
        Launch the current visualization plugin.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        1 on success, <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/exec_visual?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(Exec_Visual)
{
    PluginManager::Get()->WinampMessage(WM_COMMAND, WINAMP_EXEC_VISUAL, 0);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>getoutputtime</name>
    <action>
        Returns the position in milliseconds of the current song, or the song length, in seconds.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>frmt</name>
            <description>
                0 Position (in ms) of current song <br/>
                1 Length (in sec) of current song
            </description>
        </argument>
    </arguments>
    <return>
        -1 if not playing. <br/>
        0 on error. 
    </return>
    <example>
        http://localhost:4800/getoutputtime?p=pass&frmt=1
    </example>
</function>
!*/
CommandFunctionDecl(GetOutputTimeA)
{
    const char *arg = query.GetArgument("frmt");

    int res = atoi(arg);
    if(res != 0 && res != 1)
    {
        pWriter->WriteError();
    }
    else
    {
        res = PluginManager::Get()->WinampMessage(WM_WA_IPC, res, IPC_GETOUTPUTTIME);
        pWriter->WriteReturnCode(res);
    }
}


/*!
<function>
    <name>jumptotime</name>
    <action>
        Sets the position in milliseconds of the current song (approximately) to 'argument'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>ms</name>
            <description>
                Position in milliseconds to jump to.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/jumptotime?p=pass&ms=1000
    </example>
</function>
!*/
CommandFunctionDecl(JumpToTimeA)
{
    const char *arg = query.GetArgument("ms");

    int res = atoi(arg);
	if(res < 0)
	{
        pWriter->WriteError();
	}
    else
    {
        res = PluginManager::Get()->WinampMessage(WM_WA_IPC, res, IPC_JUMPTOTIME);
        if (res != 0)
            pWriter->WriteError();
        else
            pWriter->WriteSuccess();
    }
}


/*!
<function>
    <name>setplaylistpos</name>
    <action>
        Sets the playlsit position to 'argument'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>index</name>
            <description>
                Position to set play list to.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise. 
    </return>
    <example>
        http://localhost:4800/setplaylistpos?p=pass&index=1000
    </example>
</function>
!*/
CommandFunctionDecl(SetPlayListPosA)
{
    const char *arg = query.GetArgument("index");

    int res = atoi(arg);
    if(res < 0)
    {
        pWriter->WriteError();
    }
    else
    {
        PluginManager::Get()->WinampMessage(WM_WA_IPC, res, IPC_SETPLAYLISTPOS);
        pWriter->WriteSuccess();
    }
}


/*!
<function>
    <name>getplaylisttitle</name>
    <action>
        Returns a list of all the titles in the playlist seperated by the specified delimiter.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>delim</name>
            <description>the specified delimieter.</description>
        </argument>
    </arguments>
    <return>
        List of track titles on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getplaylisttitle?p=pass&delim=;
    </example>
</function>
!*/
CommandFunctionDecl(GetPlayListTitle)
{
    const char *delim = query.GetArgument("delim");
    int len = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTLENGTH);

    for(int i = 0; i<len; i++)
    {
	    char *tmp = (char *)PluginManager::Get()->WinampMessage(WM_WA_IPC, i, IPC_GETPLAYLISTTITLE);
	    if(!tmp)
	    {
            pWriter->WriteError();
	    }
	    else
	    {
            pWriter->WriteData(tmp); // title
            if (i < len - 1)
                pWriter->WriteData(delim);
	    }
    }
}


/*!
<function>
    <name>getplaylisttitle</name>
    <action>
        Returns the title of the playlist entry at index 'index'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>index</name>
            <description>
                index in the list from which to retrieve title. <br/>
                note: the list is zero indexed so track 1 is 0. 
            </description>
        </argument>
    </arguments>
    <return>
        Title of track on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getplaylisttitle?p=pass&index=1
    </example>
</function>
!*/
CommandFunctionDecl(GetPlayListTitleA)
{
    const char *arg = query.GetArgument("index");
    
    int res = atoi(arg);
    if(res < 0)
    {
        pWriter->WriteError();
    }
    else
    {
	    char *tmp = (char *)PluginManager::Get()->WinampMessage(WM_WA_IPC, res, IPC_GETPLAYLISTTITLE);
        if(tmp)
	        pWriter->WriteData(tmp);
        else
	        pWriter->WriteError();
    }
}


/*!
<function>
    <name>getplaylistfile</name>
    <action>
        Returns a list of all the filenames in the playlist seperated by the specified delimiter.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>The password.</description>
        </argument>
        <argument>
            <name>delim</name>
            <description>The specified delimieter.</description>
        </argument>
    </arguments>
    <return>
        List of filenames on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getplaylistfile?p=pass&delim=;
    </example>
</function>
!*/
CommandFunctionDecl(GetPlayListFile)
{
    const char *delim = query.GetArgument("delim");
    int len = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTLENGTH);

    for(int i = 0; i<len; i++)
    {
	    char *tmp = (char *)PluginManager::Get()->WinampMessage(WM_WA_IPC, i, IPC_GETPLAYLISTFILE);
	    if(!tmp)
	    {
            pWriter->WriteError();
	    }
	    else
	    {
            pWriter->WriteData(tmp); // title
            if (i < len - 1)
                pWriter->WriteData(delim);
	    }
    }
}


/*!
<function>
    <name>getplaylistfile</name>
    <action>
        Returns the filename of the playlist entry at index 'argument'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>index</name>
            <description>
                index in the list from which to retrieve title. <br/>
                note: the list is zero indexed so track 1 is 0. 
            </description>
        </argument>
    </arguments>
    <return>
        filename of track on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getplaylistfile?p=pass&index=1
    </example>
</function>
!*/
CommandFunctionDecl(GetPlayListFileA)
{
    const char *arg = query.GetArgument("index");
    
    int res = atoi(arg);
    if(res < 0)
    {
        pWriter->WriteError();
    }
    else
    {
	    char *tmp = (char *)PluginManager::Get()->WinampMessage(WM_WA_IPC, res, IPC_GETPLAYLISTFILE);
        if(tmp)
	        pWriter->WriteData(tmp);
        else
	        pWriter->WriteError();
    }
}


/*!
<function>
    <name>chdir</name>
    <action>
        Change the working direcotry to 'argument'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>dir</name>
            <description>
                The path to the new current working directory.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/chdir?p=pass&dir=d:\mp3
    </example>
</function>
!*/
CommandFunctionDecl(ChDirA)
{
    const char *arg = query.GetArgument("dir");

    // decode special characters from text
    //UrlDecode(args);

    // setup cds data struct, include space for null char in data
    COPYDATASTRUCT cds;
    cds.dwData = IPC_CHDIR;
    cds.lpData = (void *)arg;
    cds.cbData = strlen((char *)cds.lpData) + 1;

	PluginManager::Get()->WinampMessage(WM_COPYDATA, NULL, (LPARAM)&cds);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>playfile</name>
    <action>
        Appends a file to the playlist. <br/>
        The file must be in the current working directory or pass in the directory 
        along with the filename as the argument.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>file</name>
            <description>
                The file name to append to the playlist.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/playfile?p=pass&file=c:\mp3\music.mp3
    </example>
</function>
!*/
CommandFunctionDecl(PlayFileA)
{
    const char *arg = query.GetArgument("file");

    // setup cds data struct, include space for null char in data
    COPYDATASTRUCT cds;
    cds.dwData = IPC_PLAYFILE;
    cds.lpData = (void *)arg;
    cds.cbData = strlen((char *)cds.lpData) + 1;

	int rc = PluginManager::Get()->WinampMessage(WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>getinfo</name>
    <action>
        Gets info about the current playing song. The value it returns 
        depends on the value of 'info'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>info</name>
            <description>
                0 Samplerate (i.e. 44100) <br >
                1 Bitrate (i.e. 128) <br >
                2 Channels (i.e. 2)
            </description>
        </argument>
    </arguments>
    <return>
        requested info on success. <br/>
        0 on error.
    </return>
    <example>
        http://localhost:4800/getinfo?p=pass&info=1
    </example>
</function>
!*/
CommandFunctionDecl(GetInfoA)
{
    const char *arg = query.GetArgument("info");

    int res = atoi(arg);
    if(res < 0 || res > 2)
    {
        pWriter->WriteError();
    }
    else
    {
        res = PluginManager::Get()->WinampMessage(WM_WA_IPC, res, IPC_GETINFO);
        pWriter->WriteReturnCode(res);
    }
}


/*!
<function>
    <name>shuffle</name>
    <action>
        Toggle shuffle on and off.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>enable</name>
            <description>
                0 Turn shuffle off <br/>
                1 Turn shuffle on 
            </description>
        </argument>
    </arguments>
    <return>
        1 if shuffle is on. <br/>
        0 otherwise 
    </return>
    <example>
        http://localhost:4800/shuffle?p=pass&enable=0
    </example>
</function>
!*/
CommandFunctionDecl(ShuffleA)
{
    const char *arg = query.GetArgument("enable");

    int cur = atoi(arg);
    if(cur != 0 && cur != 1)
    {
        pWriter->WriteError();
    }
    else
    {
        PluginManager::Get()->WinampMessage(WM_WA_IPC, cur, IPC_SET_SHUFFLE);
        pWriter->WriteSuccess();
    }
}


/*!
<function>
    <name>repeat</name>
    <action>
        Toggle repeat on and off.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>enable</name>
            <description>
                0 Turn repeat off <br/>
                1 Turn repeat on 
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise 
    </return>
    <example>
        http://localhost:4800/repeat?p=pass&enable=1
    </example>
</function>
!*/
CommandFunctionDecl(RepeatA)
{
    const char *arg = query.GetArgument("enable");

    int cur = atoi(arg);
    if(cur != 0 && cur != 1)
    {
        pWriter->WriteError();
    }
    else
    {
        PluginManager::Get()->WinampMessage(WM_WA_IPC, cur, IPC_SET_REPEAT);
        pWriter->WriteSuccess();
    }
}


/*!
<function>
    <name>setvolume</name>
    <action>
        Set the volume level.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>level</name>
            <description>
               0-255 volume level.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise 
    </return>
    <example>
        http://localhost:4800/setvolume?p=pass&level=100
    </example>
</function>
!*/
CommandFunctionDecl(SetVolumeA)
{
    const char *arg = query.GetArgument("level");

    int n = atoi(arg);
	if(n < 0 || n > 255)
    {
        pWriter->WriteError();
    }
    else
    {
        PluginManager::Get()->WinampMessage(WM_WA_IPC, n, IPC_SETVOLUME);
        pWriter->WriteSuccess();
    }
}


/*!
<function>
    <name>getvolume</name>
    <action>
        Get the volume level.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
    </arguments>
    <return>
        volume level on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/getvolume?p=pass
    </example>
</function>
!*/
CommandFunctionDecl(GetVolume)
{
    int n = -666;
    n = PluginManager::Get()->WinampMessage(WM_WA_IPC, n, IPC_SETVOLUME);
    pWriter->WriteReturnCode(n);
}


/*!
<function>
    <name>geteqdata</name>
    <action>
        Gets a band of eq data.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>band</name>
            <description>
                The band of eq data. <br/> <br/>
                0-9: 10 bands of EQ data. <br/>
                10 : Preamp value. <br/>
                11 : Equalizer enabled. <br/>
                12 : Equalizer autoload. <br/> <br/>
            </description>
        </argument>
    </arguments>
    <return>
        0-9: 0-63 (+20db - -20db). <br/>
        10 : 0-63 (+20db - -20db). <br/>
        11 : Zero if disabled, nonzero if enabled. <br/>
        12 : Zero if disabled, nonzero if enabled.
    </return>
    <example>
        http://localhost:4800/geteqdata?p=pass&band=8
    </example>
</function>
!*/
CommandFunctionDecl(GetEqDataA)
{
    const char *arg = query.GetArgument("band");

	int n = atoi(arg);
	if(n < 0 || n > 12)
	{
        pWriter->WriteError();
	}
    else
    {
        n = PluginManager::Get()->WinampMessage(WM_WA_IPC, n, IPC_GETEQDATA);
        pWriter->WriteReturnCode(n);
    }
}


/*!
<function>
    <name>seteqdata</name>
    <action>
        Sets the eq data at 'band' to 'level'.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>band</name>
            <description>
                The eq band to set where possible values include: <br/> <br/>
                0-9 : 10 bands of EQ data (0-63). <br/>
                10  : Preamp value (0-63). <br/>
                11  : Equalizer enabled (0-1). <br/>
                12  : Equalizer autoload (0-1). <br/>
                <br>
            </description>
        </argument>
        <argument>
            <name>level</name>
            <description>
                The level of the band setting.
            </description>
        </argument>
    </arguments>
    <return>
        1 on success. <br/>
        0 otherwise.
    </return>
    <example>
        http://localhost:4800/seteqdata?p=pass&band=8&level=10
    </example>
</function>
!*/
CommandFunctionDecl(SetEqDataA)
{
    const char *band = query.GetArgument("band");
    const char *level = query.GetArgument("level");

    PluginManager::Get()->WinampMessage(WM_WA_IPC, atoi(band), IPC_GETEQDATA);
    PluginManager::Get()->WinampMessage(WM_WA_IPC, atoi(level), IPC_SETEQDATA);
    pWriter->WriteSuccess();
}


/*!
<function>
    <name>getid3tag</name>
    <action>
        Get Id3 Tag information about the current playing song.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>tags</name>
            <description>
                The comma delimited list of tags to return. May be any of the following
                values: <br>
                <blockquote>
                    t  - Title <br/>
                    a  - Artist <br/>
                    y  - Year <br/>
                    g  - Genre <br/>
                    l  - Album <br/>
                    c  - Comment <br/>
                    r  - Track
                </blockquote>
            </description>
        </argument>
        <argument>
            <name>delim</name>
            <description>The delimeter to use when seperating the return data.</description>
        </argument>
    </arguments>
    <return>
    </return>
    <example>
        http://localhost:4800/getid3tag?p=pass&tags=t,a,y,g,l,c,r&delim=;
    </example>
</function>
!*/
CommandFunctionDecl(GetId3Tag)
{
    const char *tags = query.GetArgument("tags");
    const char *delim = query.GetArgument("delim");
    
    int cur = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTPOS);
    char *fpath = (char *)PluginManager::Get()->WinampMessage(WM_WA_IPC, cur, IPC_GETPLAYLISTFILE);

    ID3_Tag id3Tag(fpath);
    if(!id3Tag.HasV1Tag())
    {
        pWriter->WriteError();
    }
    else
    {
        char *data = NULL;
        char *format = strmemdup(tags, strlen(tags));
        char *p = strtok(format, ",");

	    while(p)
	    {
            switch( tolower(p[0]) )
            {
                case 't':
                {
                    data = ID3_GetTitle(&id3Tag);
                    break;
                }
                case 'a':
                {
                    data = ID3_GetArtist(&id3Tag);
                    break;
                }
                case 'y':
                {
                    data = ID3_GetYear(&id3Tag);
                    break;
                }
                case 'g':
                {
                    const char *tmp = ID3_V1GENRE2DESCRIPTION(ID3_GetGenreNum(&id3Tag));
                    if (tmp)
                        data = strmemdup(tmp, strlen(tmp));
                    break;
                }
                case 'l':
                {
                    data = ID3_GetAlbum(&id3Tag);
                    break;
                }
                case 'c':
                {
                    data = ID3_GetComment(&id3Tag);
                    break;
                }
                case 'r':
                {
                    data = ID3_GetTrack(&id3Tag);
                    break;
                }
                default:
                {
                    data = NULL;
                    break;
                }
            }

            if (data)
            {
                pWriter->WriteData(data);
                delete data;
                data = NULL;
            }
            pWriter->WriteData(delim);

            p = strtok(NULL, ",");
	    }

        strmemfree(format);
    }
}


/*!
<function>
    <name>getmpeginfo</name>
    <action>
        Get Mpeg information about the current playing song.
    </action>
    <arguments>
        <argument>
            <name>p</name>
            <description>the password.</description>
        </argument>
        <argument>
            <name>info</name>
            <description>
                The comma delimited list of info to return. May be any of the following
                values: <br>
                <blockquote>
                    s  - file size <br/>
                    t  - time <br/>
                    v  - mpeg version <br/>
                    l  - mpeg layer <br/>
                    f  - frequency <br/>
                    b  - bitrate <br/>
                    m  - channel mode br/>
                    c  - crc <br/>
                    y  - copy right <br/>
                    o  - original <br/>
                    e  - emphasis 
                </blockquote>
            </description>
        </argument>
        <argument>
            <name>delim</name>
            <description>The delimeter to use when seperating the return data.</description>
        </argument>
    </arguments>
    <return>
    </return>
    <example>
        http://localhost:4800/getmpeginfo?p=pass&info=s,t,v,l,f,b,m,c,y,o,e&delim=;
    </example>
</function>
!*/
CommandFunctionDecl(GetMpegInfo)
{
    const char *info = query.GetArgument("info");
    const char *delim = query.GetArgument("delim");

    int cur = PluginManager::Get()->WinampMessage(WM_WA_IPC, 0, IPC_GETLISTPOS);
    char *fpath = (char *)PluginManager::Get()->WinampMessage(WM_WA_IPC, cur, IPC_GETPLAYLISTFILE);

    ID3_Tag id3Tag(fpath);
    const Mp3_Headerinfo *mp3info = id3Tag.GetMp3HeaderInfo();
    if(!mp3info) 
    {
        pWriter->WriteError();
    }
    else
    {
        char data[256];
        char *format = strmemdup(info, strlen(info));
        char *p = strtok(format, ",");
        bool write = true;

        while(p)
        {
            switch( tolower(p[0]) )
            {
                case 's': // file size
                {
                    wsprintf(data, "%ld", id3Tag.GetFileSize());
                    break;
                }
                case 't': // time
                {
                    wsprintf(data, "%ld", mp3info->time);
                    break;
                }
                case 'v': // mpeg version
                {
                    wsprintf(data, "%s", MPEG_VERSION2DESCRIPTION(mp3info->version));
                    break;
                }
                case 'l': // mpeg layer
                {
                    wsprintf(data, "%s", MPEG_LAYER2DESCRIPTION(mp3info->layer));
                    break;
                }
                case 'f': // mpeg freq
                {
                    wsprintf(data, "%d", mp3info->frequency);
                    break;
                }
                case 'b': // bitrate
                {
                    wsprintf(data, "%d", MPEG_BITRATE2KBPS(mp3info->bitrate));
                    break;
                }
                case 'm': // mpeg channel mode
                {
                    wsprintf(data, "%s", MPEG_CHANNELMODE2DESCRIPTION(mp3info->channelmode));
                    break;
                }
                case 'c': // crc
                {
                    wsprintf(data, "%s", MPEG_CRC2DESCRIPTION(mp3info->crc));
                    break;
                }
                case 'y': // copyright
                {
                    wsprintf(data, "%s", MPEG_COPYRIGHT2DESCRIPTION(mp3info->copyrighted));
                    break;
                }
                case 'o': // original
                {
                    wsprintf(data, "%s", MPEG_ORIGINAL2DESCRIPTION(mp3info->original));
                    break;
                }
                case 'e': // emphasis
                {
                    wsprintf(data, "%s", MPEG_EMPHASIS2DESCRIPTION(mp3info->emphasis));
                    break;
                }
                default:
                {
                    write = false;
                    break;
                }
            }

            if (write)
            {
                pWriter->WriteData(data);
                pWriter->WriteData(delim);
            }

            p = strtok(NULL, ",");
	    }

        strmemfree(format);
    }
}