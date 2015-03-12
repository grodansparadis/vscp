/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#include "server.h"

unsigned int ServerThread(void *srv) {((Cserver *)srv)->ThreadProc();return 0;}

Cserver::Cserver()
{
	for(int i=0;i<MAX_CLIENTS;i++) clients[i]=INVALID_SOCKET;
	server=INVALID_SOCKET;
}

Cserver::~Cserver()
{
	KillThread(&ServerThreadHandle,&ServerThreadEvent);
	for(int i=0;i<MAX_CLIENTS;i++)
	{
		if(clients[i]!=INVALID_SOCKET)
		{
			DEBUG("closing socket %d\n",i);
			closesocket(clients[i]);
		}
	}
	if(server!=INVALID_SOCKET)
	{
		closesocket(server);
		DEBUG("Server socket closed.\n");
	}
	WSACleanup();
}

bool Cserver::init()
{
	/* init winsock */
	WSADATA data;
	int res=WSAStartup(0x0002,&data);
	if(res!=0) 
	{ 
		DEBUG("WSAStartup failed\n");
		MessageBox(NULL,"Could not initialize Windows Sockets.\n"
			"Note that this program requires WinSock 2.0 or higher.","WinLIRC",MB_OK);
		return false;
	}

	/* begin password stuff */
	CRegKey key;
	bool haveKey=true;
	if(key.Open(HKEY_CURRENT_USER,"Software\\LIRC")!=ERROR_SUCCESS) //First try HKCU, then HKLM
		if(key.Open(HKEY_LOCAL_MACHINE,"Software\\LIRC")
		   !=ERROR_SUCCESS)
		{
			haveKey=false;
			DEBUG("warning: can't open Software\\LIRC key");
		}

	char s[512];
	DWORD len=512;
	if(!haveKey || key.QueryValue(s,"password",&len)!=ERROR_SUCCESS)
		password.Empty();
	else
		password=s;
	/* end password stuff */
	DWORD x;
	if(!haveKey || key.QueryValue(x,"tcp_port")!=ERROR_SUCCESS)
		tcp_port=IR_PORT;
	else
		tcp_port=x;
	if(startserver()==false)
		return false;

	/* start thread */
	/* THREAD_PRIORITY_IDLE combined with the REALTIME_PRIORITY_CLASS */
	/* of this program still results in a really high priority. (16 out of 31) */
	if((ServerThreadHandle=
		AfxBeginThread(ServerThread,(void *)this,THREAD_PRIORITY_IDLE))==NULL)
	{
		DEBUG("AfxBeginThread failed\n");
		return false;
	}	
    
	return true;
}

bool Cserver::startserver(void)
{
	/* make the server socket */
	if(server!=NULL && server!=INVALID_SOCKET)
		closesocket(server);

	server=socket(AF_INET,SOCK_STREAM,0);
	if(server==INVALID_SOCKET)
	{ 
		DEBUG("socket failed, WSAGetLastError=%d\n",WSAGetLastError());
		return false;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(tcp_port);
	if(bind(server,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==SOCKET_ERROR)
		{ DEBUG("bind failed\n"); return false; }

	if(listen(server,LISTENQ)==SOCKET_ERROR)
		{ DEBUG("listen failed\n"); return false; }

	return true;
}

void Cserver::stopserver(void)
{
	/* make the server socket */
	if(server!=NULL)
		closesocket(server);
	server=NULL;
}

void Cserver::send(const char *s)
{
	int i;

	for(i=0;i<MAX_CLIENTS;i++)
		if(clients[i]!=INVALID_SOCKET)
			::send(clients[i],s,strlen(s),0);
}

void Cserver::reply(const char *command,int client,bool success,const char *data)
{
	int length=strlen(command);
	if (data) length+=strlen(data);
	if (success) length+=19;
	else length+=17;
	char *packet=new char[length+1];
	if (!packet) return;
	strcpy(packet,"BEGIN\n");
	strcat(packet,command);
	if (success) strcat(packet,"\nSUCCESS\n");
	else strcat(packet,"\nERROR\n");
	if (data) strcat(packet,data);
	strcat(packet,"END\n");
	
	if (clients[client]!=INVALID_SOCKET) ::send(clients[client],packet,length,0);
	if (packet) delete[] packet;
	return;
}

void Cserver::ThreadProc(void)
{
	if(server==INVALID_SOCKET) return;
	int i;
	
	CEvent ServerEvent;
	CEvent ClientEvent[MAX_CLIENTS];
#define MAX_DATA 1024	// longest message a client can send
	char ClientData[MAX_CLIENTS][MAX_DATA];
	char toparse[MAX_DATA];
	HANDLE events[MAX_CLIENTS+2];
	bool server_running=true;
	
	WSAEventSelect(server,ServerEvent,FD_ACCEPT);

	for(i=0;i<MAX_CLIENTS;i++) ClientData[i][0]=0;
	
	for(;;)
	{		
		int count=0;
		events[count++]=ServerThreadEvent;
		if(server_running) events[count++]=ServerEvent;
		for(i=0;i<MAX_CLIENTS;i++)
			if(clients[i]!=INVALID_SOCKET)
				events[count++]=ClientEvent[i];
		
		unsigned int res=WaitForMultipleObjects(count,events,FALSE,INFINITE);
		if(res==WAIT_OBJECT_0)
		{
			DEBUG("ServerThread terminating\n");
			AfxEndThread(0);
			return;
		}
		else if(server_running && res==(WAIT_OBJECT_0+1))
		{
			for(i=0;i<MAX_CLIENTS;i++)
				if(clients[i]==INVALID_SOCKET) break;
			if(i==MAX_CLIENTS)
			{
				DEBUG("Should have at least 1 empty client, but none found\n");
				continue;
			}
			
			clients[i]=accept(server,NULL,NULL);
			if(clients[i]==INVALID_SOCKET)
			{
				DEBUG("accept() failed\n");
				continue;
			}

			WSAEventSelect(clients[i],ClientEvent[i],FD_CLOSE|FD_READ);
			ClientEvent[i].ResetEvent();
			ClientData[i][0]='\0';
			DEBUG("Client connection %d accepted\n",i);

			for(i=0;i<MAX_CLIENTS;i++)
				if(clients[i]==INVALID_SOCKET) break;
			if(i==MAX_CLIENTS)
			{
				DEBUG("Server full.  Closing server socket.\n");
				stopserver();
				server_running=false;
			}
		}
		else /* client closed or data received */
		{
			count=server_running?2:1;
			for(i=0;i<MAX_CLIENTS;i++)
			{
				if(clients[i]!=INVALID_SOCKET)
				{
					if(res==(WAIT_OBJECT_0+(count++)))
					{
						/* either we got data or the connection closed */
						int curlen=strlen(ClientData[i]);
						int maxlen=MAX_DATA-curlen-1;
						int bytes=recv(	clients[i], ClientData[i]+curlen, maxlen, 0);
						if(bytes==0 || bytes==SOCKET_ERROR)
						{
							/* Connection was closed or something's screwy */
							closesocket(clients[i]);
							clients[i]=INVALID_SOCKET;
							DEBUG("Client connection %d closed\n",i);

							if(server_running==false)
							{
								DEBUG("Slot open.  Restarting server.\n");
								if(startserver()==true)
								{
									WSAEventSelect(server,ServerEvent,FD_ACCEPT);
									server_running=true;
								}
								else
								{
									DEBUG("Server failed to restart.\n");
									stopserver();
								}
							}
						}
						else /* bytes > 0, we read data */
						{
							ClientData[i][curlen+bytes]='\0';
							char *cur=ClientData[i];
							for(;;) {
								char *nl=strchr(cur,'\n');
								if(nl==NULL) {
									if(cur!=ClientData[i]) 
										memmove(ClientData[i],cur,strlen(cur)+1);
									break;
								} else {
									*nl='\0';
									// ----------------------------
									// Do something with the received line (cur)
									DEBUG("Got string: %s\n",cur);
									LRESULT copyDataResult=false;
									char *response=NULL;
									strcpy(toparse,cur);
									char *command=NULL;
									char *remotename=NULL;
									char *buttonname=NULL;
									char *repeats=NULL;
									if (toparse) command=strtok(toparse," \t\r");
									if (!command) //ignore lines with only whitespace
									{
										cur=nl+1;
										break;
									}
									else if (stricmp(command,"VERSION")==0)
									{

										if (strtok(NULL," \t\r")==NULL)
										{
											copyDataResult = true;
											response = new char[strlen(id)+9];
											if (response) sprintf(response,"DATA\n1\n%s\n",id);
										}
										else copyDataResult=-100;
									}
									else if (stricmp(command,"LIST")==0)
									{
										remotename=strtok(NULL," \t\r");
										struct ir_remote *all=global_remotes;
										int n=0;
										if (!remotename)
										{
											copyDataResult = true;
											while(all)
											{
												n++;
												all=all->next;
											}
											if (n!=0)
											{
												response = new char[n*(LINE_LEN+2)+7];
												sprintf(response,"DATA\n%d\n",n);
												all=global_remotes;
												while(all)
												{
													strcat(response,all->name);
													strcat(response,"\n");
													all=all->next;
												}
											}
										}
										else
										{
											while (all!=NULL && stricmp(remotename,all->name)) all=all->next;
											if (all)
											{
												copyDataResult = true;
												struct ir_ncode *allcodes=all->codes;
												while (allcodes->name)
												{
													n++;
													allcodes++;
												}
												if (n!=0)
												{
													response = new char[n*(LINE_LEN+2)+7];
													sprintf(response,"DATA\n%d\n",n);
													allcodes=all->codes;
													while(allcodes->name)
													{
														strcat(response,allcodes->name);
														strcat(response,"\n");
														allcodes++;
													}
												}
											}
											else copyDataResult=-1; //unknown remote
										}
									}

									else if (!password.IsEmpty() && !password.CompareNoCase(command)) //only accept commands if a password is set and matches
									{
										CString incoming = (LPCSTR) (cur);
										int j=incoming.FindOneOf(" \t")+1;
										remotename=strtok(NULL," \t\r");
										if (remotename==NULL)
										{
											response = new char[14];
											if (response) sprintf(response,"DATA\n1\nremote missing\n");
										}
										else 
										{
											buttonname=strtok(NULL," \t\r");
											if (buttonname==NULL)
											{
												response = new char[12];
												if (response) sprintf(response,"DATA\n1\ncode missing\n");
											}
											else
											{

												HWND pOtherWnd = FindWindow(NULL, "WinLirc");
												if (pOtherWnd)
												{
													COPYDATASTRUCT cpd;
													cpd.dwData = 0;
													cpd.cbData = strlen(&cur[j]);
													cpd.lpData = (void*)&cur[j];
													copyDataResult = SendMessage(pOtherWnd,WM_COPYDATA,NULL,(LPARAM)&cpd);
												}
											}
										}
									}
									else
									{
										response = new char[strlen(command)+26];
										if (response) sprintf(response,"DATA\n1\n%s%s\n","unknown command: ",command);
									}
									if (copyDataResult==-1)
									{
										response = new char[strlen(remotename)+25];
										if (response) sprintf(response,"DATA\n1\n%s%s\n","unknown remote: ",remotename);
									}
									else if (copyDataResult==-2)
									{
										response = new char[strlen(buttonname)+23];
										if (response) sprintf(response,"DATA\n1\n%s%s\n","unknown code: ",buttonname);
									}
									else if (copyDataResult==-100)
									{
											response = new char[15];
											if (response) sprintf(response,"DATA\n1\nbad send packet\n");
									}
									reply(cur,i,copyDataResult==1,response);
									if (response) delete[] response;
									cur=nl+1;
								}
							}
						
						}

						break;
					}
				}
			}
		}
	}
}

