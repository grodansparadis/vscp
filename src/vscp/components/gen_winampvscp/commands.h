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
#ifndef __COMMANDS_H
#define __COMMANDS_H


//--------------------------------------------------
// Winamp IPC Calls not define by SDK
//--------------------------------------------------
#define WINAMP_EXEC_VISUAL  40192   // execute current visualization plug-in 
#define SHOUTCAST_STATUS    1000    // the status text box on shoutcast
#define SHOUTCAST_CONNECT   1008	// the connect button for shoutcast


//--------------------------------------------------
// Forward Declerations
//--------------------------------------------------
class Query;
class SocketWriter;


//--------------------------------------------------
// Command Function Decleration
//--------------------------------------------------
#define CommandFunctionDecl(x) void x(Query &query, SocketWriter *pWriter)


//--------------------------------------------------
// Function Declerations
//--------------------------------------------------
CommandFunctionDecl(Root);
CommandFunctionDecl(Validate_PasswordA);
CommandFunctionDecl(Play);
CommandFunctionDecl(Stop);
CommandFunctionDecl(Pause);
CommandFunctionDecl(NextTrack);
CommandFunctionDecl(PreviousTrack);
CommandFunctionDecl(FadeoutAndStop);
CommandFunctionDecl(GetVersion);
CommandFunctionDecl(Delete);
CommandFunctionDecl(IsPlaying);
CommandFunctionDecl(GetListLength);
CommandFunctionDecl(GetListPos);
CommandFunctionDecl(GetPlayListPos);
CommandFunctionDecl(Shuffle_Status);
CommandFunctionDecl(Repeat_Status);
CommandFunctionDecl(VolumeUp);
CommandFunctionDecl(VolumeDown);
CommandFunctionDecl(FlushPlayList);
CommandFunctionDecl(GetCurrentTitle);
CommandFunctionDecl(UpdateCurrentTitle);
CommandFunctionDecl(Internet);
CommandFunctionDecl(Restart);
CommandFunctionDecl(GetAutoService);
CommandFunctionDecl(ShoutCast_Connect);
CommandFunctionDecl(ShoutCast_Status);
CommandFunctionDecl(DeletePosA);
CommandFunctionDecl(Exec_Visual);
CommandFunctionDecl(GetOutputTimeA);
CommandFunctionDecl(JumpToTimeA);
CommandFunctionDecl(SetPlayListPosA);
CommandFunctionDecl(GetPlayListTitle);
CommandFunctionDecl(GetPlayListFile);
CommandFunctionDecl(GetPlayListTitleA);
CommandFunctionDecl(GetPlayListFileA);
CommandFunctionDecl(ChDirA);
CommandFunctionDecl(PlayFileA);
CommandFunctionDecl(GetInfoA);
CommandFunctionDecl(ShuffleA);
CommandFunctionDecl(RepeatA);
CommandFunctionDecl(SetVolumeA);
CommandFunctionDecl(GetVolume);
CommandFunctionDecl(GetEqDataA);
CommandFunctionDecl(SetEqDataA);
CommandFunctionDecl(SetAutoServiceA);
CommandFunctionDecl(GetId3Tag);
CommandFunctionDecl(GetMpegInfo);


#endif // __COMMANDS_H