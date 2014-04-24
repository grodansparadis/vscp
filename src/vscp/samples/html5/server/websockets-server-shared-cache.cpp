/*	Trevor N. Webster
	trevor.n.webster@gmail.com


	description:
	this program demonstrates a potential race condition, where multiple web socket clients
	write to a single (server-side) shared buffer. (Actually, the server-side threads running
	on_client_connected write to the shared buffer.
	If we remove the Mutex, we can reliably reproduce the race condition on every run.
	
	instructions to run:
		1.	run this program in Visual Studio 2010
		2.	when the server starts listening, open the included websockets-client.html file in multiple browser instances of Chrome or Firefox.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>//winsock

#include <windows.h>//threads
#include <process.h>

#pragma comment(lib, "wininet.lib")//winsock
#pragma comment(lib,"Ws2_32.lib")//winsock

#include <string>
#include <list>
#include <regex>
#include "md5\md5.h"
#include "md5\md5_loc.h"
using namespace std;

HANDLE ghMutex; 
HANDLE hClientMain;

/*	COMMON	*/
const char* _host = "127.0.0.1";
const char* _port = "38950";
#define NUMCLIENTS 3
#define NUM_MSGS_PER_CLIENT 12

/*	SERVER */
char shared_buffer[BUFSIZ + 1];	//+1 for null-termination
SOCKET listenfd;					//server's 1 listening SOCKET.
HANDLE connectHandles[NUMCLIENTS];	//server handles for handling client threads.
SOCKET connectDescriptors[NUMCLIENTS];
list<SOCKET> connectList;
unsigned nClients = 0;				//current number of clients connected
void init_resources();	//initialization related to multithreading
void server_startup();
void server_dispose();
SOCKET accept_client();
unsigned __stdcall on_client_connected( void* pArguments );
BOOL isMutexEnabled = true;	//FALSE to allow race conditions.
void acquire_mutex();
void release_mutex();
int server_send(char* src, SOCKET s, bool isWebSocket = true);
int server_recv(char* buffer, SOCKET s, bool isWebSocket = true);	//assume buffer and actual framed data size < BUFSIZ
/*	SERVER HANDSHAKE	*/
char* createHash(const char* key1, const char* key2, char* challenge);
bool server_negotiate_websockets_handshake(SOCKET connectfd);
long long longParse(char* str);
char* createMD5Buffer(int result1, int result2, char challenge[8]);

/*	CLIENT	*/
unsigned threadIDs[NUMCLIENTS];		
HANDLE clientHandles[NUMCLIENTS];	//client threads (clientfd)
void open_clientfd(SOCKET & clientfd);
unsigned __stdcall client_startup_main( void* pArguments );
unsigned __stdcall client_thread( void* pArguments );
void client_handshake(SOCKET clientfd);
void client_dispose();
int client_recv_frame(char* buffer, SOCKET s);
int client_send_frame(char* src, SOCKET s);

int main() {

	init_resources();
	server_startup();
	
	while (true) {
		SOCKET connectfd = accept_client();
		connectDescriptors[nClients] = connectfd;				
		connectList.push_back(connectfd);
		unsigned *threadid = (unsigned*)malloc(sizeof(int));
		connectHandles[nClients] = (HANDLE)_beginthreadex(NULL, 0, 
			&on_client_connected, 
			&connectDescriptors[nClients], //connectfd
			0, threadid);
		nClients += 1;
		//closesocket(connectfd);	//do not do this (even though Bryant + O'Hallaron say to)
	}

	server_dispose();
	return 0;
}

unsigned __stdcall on_client_connected( void* pArguments ) {
	SOCKET connectfd = *(SOCKET*)pArguments;
	bool isWebSocket = server_negotiate_websockets_handshake(connectfd);
	char* local_buffer = (char*)calloc(BUFSIZ, sizeof(char));
	int received;
	do {
		memset(local_buffer, '\0', BUFSIZ);
		received = server_recv(local_buffer, connectfd, isWebSocket);
		acquire_mutex();		
		//received = server_recv(shared_buffer, connectfd, isWebSocket);
		memcpy(shared_buffer, local_buffer, BUFSIZ);
		printf("server thread %x\tshared buffer: \"%s\"\n", GetCurrentThreadId(), shared_buffer);
		//server_send(shared_buffer, connectfd, isWebSocket);
		release_mutex();
		server_send(local_buffer, connectfd, isWebSocket);
	} while (received > -1);	//continue until client SOCKET closes.
	closesocket(connectfd);
	connectList.remove(connectfd);
	printf("Client socket %u connection closed.\nExiting server thread %x...\n", GetCurrentThread(), connectfd);
	return 0;
}

/*	returns TRUE if this is a websockets client.
	only need to check this once (after accept).
*/
bool server_negotiate_websockets_handshake(SOCKET connectfd) {	
	const char *key1 = NULL, *key2 = NULL, *origin = NULL, *resource = NULL;
	char *location = (char*)calloc(0x40, sizeof(char));
	char requestbuf[BUFSIZ];
	char responsebuf[BUFSIZ];
	memset(requestbuf,0,BUFSIZ);
	memset(responsebuf,0,BUFSIZ);
	int received = recv(connectfd, requestbuf, BUFSIZ, 0);
	char* key1pattern = "(Sec-WebSocket-Key1:)[[:s:]](.+\\r\\n)";
	char* key2pattern = "(Sec-WebSocket-Key2:)[[:s:]](.+\\r\\n)";
	char* resourcePattern = "(GET)[[:s:]](/[[:alnum:]]+)";
	char* originPattern = "(Origin:)[[:s:]](.+)\\r\\n";
	char* challenge = (char*)calloc(8,sizeof(char));
	for (int i = 0; i < 8; i++) {
		challenge[i] = requestbuf[strlen(requestbuf) - 8 + i];
	}

	match_results<const char*> m;
	tr1::regex rx;	
	string s;

	//match Sec-WebSocket-Key1 
	m = match_results<const char*>();
	rx = tr1::regex(key1pattern);
	tr1::regex_search(requestbuf, m, rx);	
	s = m[2];
	key1 = strdup(s.c_str());

	//match Sec-WebSocket-Key2
	m = match_results<const char*>();
	rx = tr1::regex(key2pattern);
	tr1::regex_search(requestbuf, m, rx);
	s = m[2];
	key2 = strdup(s.c_str());
	//match origin
	m = match_results<const char*>();
	rx = tr1::regex(originPattern);
	tr1::regex_search(requestbuf, m, rx);
	s = m[2];
	origin = strdup(s.c_str());
	//match GET (resource)
	m = match_results<const char*>();
	rx = tr1::regex(resourcePattern);
	tr1::regex_search(requestbuf, m, rx);
	s = m[2];
	resource = strdup(s.c_str());
	sprintf(location, "ws://%s:%s%s", _host, _port, resource);
	//ws->location = "ws://" + ws->ipAddress + ":" + ws->port + *resource;
	if (key1 == NULL || key2 == NULL || resource == NULL)// if (*key1 == "" || *key2 == "" || *resource == "")
		return false;

	//create handshake response:
	char* hash = createHash(key1, key2, challenge);
	char* handshake = (char*)calloc(BUFSIZ,sizeof(char));
	char * handshakeFormat = "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
		"Upgrade: WebSocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Origin: %s\r\n" 
		"Sec-WebSocket-Location: %s\r\n\r\n";
	sprintf(handshake, handshakeFormat, origin, location);
	int i, j;
	for (i = 0; i < strlen(handshake); i++) {
		responsebuf[i] = handshake[i];
	}
	for (j = 0; j < MD5_SIZE; i++, j++) {
		responsebuf[i] = hash[j];
	}
	int sent = send(connectfd, responsebuf, strlen(responsebuf), 0);
	if (sent > 0) {
		memset(responsebuf, 0x00, BUFSIZ);
		sprintf(responsebuf, "client socket descriptor is %d.", connectfd);
		sent = server_send(responsebuf, connectfd, true);
	}
	return sent > 0;
}

SOCKET accept_client() {
	struct sockaddr clientinfo = { 0 };
	SOCKET connectfd = accept(listenfd, &clientinfo, NULL);		
	struct sockaddr_in* ipv4info = (struct sockaddr_in*)&clientinfo;
	printf("Client connected on socket %x. Address %s:%d\n", connectfd, 
	inet_ntoa(ipv4info->sin_addr), ntohs(ipv4info->sin_port));		
	return connectfd;
}


/*	SERVER		*/
void server_startup() {
	WSADATA wsaData;	
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)// Initialize Winsock
		printf("Error at WSAStartup()\n");
	//create the SOCKET
	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	

	struct sockaddr_in server = { 0 };
	server.sin_family = AF_INET;	
	server.sin_addr.s_addr = inet_addr(_host);
	server.sin_port = htons((u_short)atoi(_port));
	
	// Bind the socket.
	if (bind( listenfd, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR) 	{
		printf("Error on server bind.\n");
		return;
	}		
	if (listen(listenfd, SOMAXCONN ) == SOCKET_ERROR)
	{
		printf("Error listening on socket.\n");
		return;
	}		
	printf("Server is listening on socket... %s:%u\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));	
}


void init_resources() {
	ghMutex = CreateMutex( 
	NULL,              // default security attributes
	FALSE,             // initially not owned
	NULL);             // unnamed mutex

	if (ghMutex == NULL) 
		printf("CreateMutex error: %x\n", GetLastError());
	
}

void acquire_mutex() {
	if (!isMutexEnabled)	
		return;
	if ( WaitForSingleObject(ghMutex, INFINITE) != WAIT_OBJECT_0) { 
		printf("Error on WaitForSingleObject (thread %x)\n", GetCurrentThreadId()); 
	}
}

void release_mutex() {
	if (!isMutexEnabled)
		return;

	if (!ReleaseMutex(ghMutex)) { 
		printf("Error releasing Mutex on thread %x.\n", GetCurrentThreadId()); 
	}
}

void server_dispose() {
	CloseHandle(ghMutex);
	CloseHandle(hClientMain);
	for (int i = 0; i < NUMCLIENTS; i++) {
		closesocket(connectDescriptors[i]);
		CloseHandle(connectHandles[i]);
	}
	closesocket(listenfd);
	
}
int server_send(char* src, SOCKET s, bool isWebSocket)
{	
	if (!isWebSocket) {
		return send(s, src, strlen(src), 0);
	}
	int length = strlen(src) 
		+ 1	//0x00
		+ 1;	//0xFF
	char* frame = (char*)calloc(length,sizeof(char));
	frame[0] = 0x00;
	for (int i = 0; i < strlen(src); i++)
	{
		frame[1 + i] = src[i];//read src into frame
	}	
	frame[length - 1] = 0xFF;
	return send(s, frame, length, 0);
}

int server_recv(char* buffer, SOCKET s, bool isWebSocket){	

	memset(buffer, 0x00, BUFSIZ);
	if (!isWebSocket) {
		return recv(s, buffer, BUFSIZ, 0);
	}

	char framed_data[BUFSIZ];
	int received = recv(s, framed_data, BUFSIZ, 0);
	char end = 0xFF;
	for (int i = 1; framed_data[i] != end && i < received; i++)
	{
		buffer[i - 1] = framed_data[i];	
	}	
	return received;

	//if handshake fails: return -1.
}

/**	MD5		**/

char* createHash(const char *key1, const char *key2, char *challenge)
{
	int spaces1 = 0;
	int spaces2 = 0;
	char* digits1 = (char*)calloc(64,sizeof(char));
	char *digits2 = (char*)calloc(64,sizeof(char));
	int d1 = 0, d2 = 0;
	//string digits1, digits2;
	int result1, result2;
	for (int i = 0; i < strlen(key1); i++)
	{
		if (key1[i] == 0x20)
			spaces1++;
	}
	for (int i = 0; i < strlen(key2); i++)
	{
		if (key2[i] == 0x20)
			spaces2++;
	}

	for (int i = 0; i < strlen(key1); i++)
	{
		if (isdigit(key1[i]))
		{
			//digits1 += key1[i];
			digits1[d1++] = key1[i];
		}			
	}
	for (int i = 0; i < strlen(key2); i++)
	{
		if (isdigit(key2[i]))
		{
			//digits2 += key2[i];
			digits2[d2++] = key2[i];
		}			
	}
	result1 = longParse(digits1) / spaces1;
	result2 = longParse(digits2) / spaces2;

	char* raw_answer = createMD5Buffer(result1,result2,challenge);	

	/* calculate the sig */
	char * sig = (char*)calloc(MD5_SIZE,sizeof(char));

	md5_buffer(raw_answer, MD5_SIZE, sig);	//sig is the MD5 hash

	//debug
	/*for (int i = 0; i < MD5_SIZE; i++)
	{
	printf("%d %d\n", raw_answer[i], (unsigned char)sig[i]);
	}*/    
	/* convert from the sig to a string rep */
	//char* str = (char*)calloc(64, sizeof(char));    
	//md5_sig_to_string(sig, str, sizeof(str));

	return sig;
}

long long longParse(char *str)
{
	long long result = 0;
	for (int i = 0; i < strlen(str); i++)
	{
		unsigned digit = str[strlen(str) - 1 - i] - '0';
		result += digit * pow((double)10, i);
	}
	return result;
}



char* createMD5Buffer(int result1, int result2, char challenge[8])
{	
	char* raw_answer = (char*)calloc(MD5_SIZE,sizeof(char));
	raw_answer[0] = ((unsigned char*)&result1)[3];
	raw_answer[1] = ((unsigned char*)&result1)[2];
	raw_answer[2] = ((unsigned char*)&result1)[1];
	raw_answer[3] = ((unsigned char*)&result1)[0];
	raw_answer[4] = ((unsigned char*)&result2)[3];
	raw_answer[5] = ((unsigned char*)&result2)[2];
	raw_answer[6] = ((unsigned char*)&result2)[1];
	raw_answer[7] = ((unsigned char*)&result2)[0];
	for (int i = 0; i < 8; i++)
	{				
		raw_answer[8 + i] = challenge[i];		
	}
	//display: (debugging)
	/*printf("raw answser bytes: ");
	for (int i = 0; i < MD5_SIZE; i++)		
	printf(" %d", (unsigned char)raw_answer[i]);	
	printf("\n\n");*/
	return raw_answer;
}



